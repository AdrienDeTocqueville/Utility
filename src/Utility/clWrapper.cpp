#ifdef USE_OPENCL

#include "Utility/clWrapper.h"

#include "Utility/Error.h"
#include "Utility/Tensor.h"

#include <cstdlib>
#include <fstream>
#include <iostream>


namespace cl
{

std::vector<cl_platform_id> getPlatformsIds()
{
    std::vector<cl_platform_id> ids;

    cl_int error;

    cl_uint platCount;
    error = clGetPlatformIDs(0, nullptr, &platCount);
    if (error != CL_SUCCESS)
        Error::add(ErrorType::WARNING, "OpenCL: Failed to get platform count with error code " + toString(error));

    ids.resize(platCount);
    error = clGetPlatformIDs(platCount, ids.data(), nullptr);
    if (error != CL_SUCCESS)
        Error::add(ErrorType::WARNING, "OpenCL: Failed to get platforms with error code " + toString(error));

    return ids;
}

std::vector<cl_device_id> getDeviceIds(DeviceType _deviceType, cl_platform_id _platformId)
{
    static cl_device_type deviceTypes[] = {CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_ALL};
    std::cout << deviceTypes[_deviceType] << std::endl;

    std::vector<cl_device_id> ids;

    cl_int error;

    cl_uint deviceCount = 0;
    error = clGetDeviceIDs(_platformId, deviceTypes[_deviceType], 0, nullptr, &deviceCount);
    if (error != CL_SUCCESS)
        Error::add(ErrorType::WARNING, "OpenCL: Failed to get devices count with error code " + toString(error));

    ids.resize(deviceCount);
    error = clGetDeviceIDs(_platformId, deviceTypes[_deviceType], deviceCount, ids.data(), nullptr);
    if (error != CL_SUCCESS)
        Error::add(ErrorType::WARNING, "OpenCL: Failed to get devices with error code " + toString(error));

    return ids;
}

std::string getVersion(cl_platform_id _platformId)
{
    cl_uint charCount = 0;
    clGetPlatformInfo(_platformId, CL_PLATFORM_VERSION, 0, nullptr, &charCount);

    char* cversion = new char[charCount];
    clGetPlatformInfo(_platformId, CL_PLATFORM_VERSION, charCount, cversion, nullptr);

    std::string version(cversion);
    delete[] cversion;

    return version;
}

Context::Context():
    deviceId(0)
{ }

Context::~Context()
{
    release();
}

void Context::create(DeviceType _deviceType)
{
    cl_platform_id platformId = getPlatformsIds().front();

    create( getDeviceIds(_deviceType, platformId).front() );
}

void Context::create(cl_device_id _deviceId)
{
    if (_deviceId == deviceId)
        return;

    if (id)
        release();


    cl_int error;

    id = clCreateContext(nullptr, 1, &_deviceId, nullptr, nullptr, &error);
    deviceId = _deviceId;

    if (error != CL_SUCCESS)
        Error::add(ErrorType::WARNING, "Error " + toString(error) + " while creating context on device: " + toString(_deviceId));
}

void Context::release()
{
    clReleaseContext(id);
    id = 0;

    deviceId = 0;
    programs.clear();
}

const Program& Context::getProgram(const std::string& _path)
{
    Program& program = programs[_path];
    program.create(*this, _path);

    return program;
}

cl_device_id Context::getDeviceId() const
{
    return deviceId;
}


/// Program
std::string Program::baseDirectory = "";

Program::~Program()
{
    release();
}

void Program::create(const Context& _context, const std::string& _path)
{
    if (id)
        return;

    cl_int error;
    std::string path = baseDirectory + _path;

	std::ifstream in(_path);
	if (!in)
        return Error::add(ErrorType::FILE_NOT_FOUND, _path);

    // Read file
	std::string src( (std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>() );
    const char* strings = src.c_str();
    const size_t length = src.size();

    // Create Program
    id = clCreateProgramWithSource(_context(), 1, &strings, &length, &error);
    if (error != CL_SUCCESS)
        Error::add(ErrorType::UNKNOWN_ERROR, "Error " + toString(error) + " while creating program: " + _path);

    // Compile program
    auto deviceId = _context.getDeviceId();
    error = clBuildProgram(id, 1, &deviceId, nullptr, nullptr, nullptr);//"-cl-std=CL2.0"

    if (error == CL_BUILD_PROGRAM_FAILURE)
    {
        // TODO: remove iostream dependency
        std::cout << getBuildLog(deviceId) << std::endl;

        Error::add(ErrorType::USER_ERROR, "Build failed for: " + _path);
    }
    else if (error != CL_SUCCESS)
        Error::add(ErrorType::UNKNOWN_ERROR, "Error " + toString(error) + " while building program: " + _path);
}

void Program::release()
{
	clReleaseProgram(id);
    id = 0;
}

std::string Program::getBuildLog(cl_device_id _deviceId) const
{
    size_t logLength;
    clGetProgramBuildInfo(id, _deviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logLength);

    std::string buildLog(logLength, ' ');
    char* log = new char[logLength];
    clGetProgramBuildInfo(id, _deviceId, CL_PROGRAM_BUILD_LOG, logLength, log, nullptr);

    buildLog = log;
    delete[] log;

    return buildLog;
}

void Program::setBaseDirectory(const std::string& _baseDirectory)
{
    baseDirectory = _baseDirectory;

    if (baseDirectory.size() && baseDirectory.back() != '/')
        baseDirectory.append("/");
}


/// Kernel
Kernel::~Kernel()
{
    release();
}

void Kernel::create(const Program& _program, const std::string& _name)
{
    if (&_program == program && _name == name)
        return;

    if (id)
        release();


    cl_int error;

    id = clCreateKernel(_program(), _name.c_str(), &error);
    program = &_program;
    name = _name;

    if (error != CL_SUCCESS)
    {
        if (error == CL_INVALID_KERNEL_NAME)
            Error::add(ErrorType::USER_ERROR, "Invalid kernel name: " + _name);
        else
            Error::add(ErrorType::UNKNOWN_ERROR, "Error " + toString(error) + " while creating kernel: " + _name);
    }
}

void Kernel::release()
{
	clReleaseKernel(id);
    id = 0;

    program = nullptr;
    name.clear();
}

void Kernel::setArg(cl_uint _index, size_t _size, const void* _value)
{
    cl_int error = clSetKernelArg(id, _index, _size, _value);

    if (error != CL_SUCCESS)
        Error::add(ErrorType::UNKNOWN_ERROR, "Kernel arg number " + toString(_index) + " error: " + toString(error));
}

void Kernel::setArg(cl_uint _index, const Tensor& _value)
{
    setArg(_index, sizeof(cl_mem), &_value.getBuffer()());
}


/// Buffer
Buffer::~Buffer()
{
    release();
}

void Buffer::create(const cl::Context& _context, cl_mem_flags _flags, size_t _byteSize, void* _hostPtr)
{
    if (&_context == context && _flags == flags)
        return;

    if (id)
        release();


    cl_int error;

    id = clCreateBuffer(_context(), _flags, _byteSize, _hostPtr, &error);
    context = &_context;
    flags = _flags;

    if (error != CL_SUCCESS)
        Error::add(ErrorType::UNKNOWN_ERROR, "Unable to create buffer: " + toString(error));
}

void Buffer::release()
{
    clReleaseMemObject(id);
    id = 0;

    context = nullptr;
    flags = 0;
}


/// CommandQueue
CommandQueue::~CommandQueue()
{
    release();
}

void CommandQueue::create(const Context& _context, bool _inOrder)
{
    if (&_context == context && _inOrder == inOrder)
        return;

    if (id)
        release();


    cl_int error;

    // About out of order mode: https://www.khronos.org/registry/OpenCL/sdk/1.2/docs/man/xhtml/clCreateCommandQueue.html
    cl_command_queue_properties cqProperties = _inOrder? 0: CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    cl_queue_properties properties[] = {CL_QUEUE_PROPERTIES, cqProperties, 0};

    id = clCreateCommandQueueWithProperties(_context(), _context.getDeviceId(), properties, &error);
//    id = clCreateCommandQueue(_context(), _context.getDeviceId(), cqProperties, &error);

    context = &_context;
    inOrder = _inOrder;

    if (error != CL_SUCCESS)
        Error::add(ErrorType::UNKNOWN_ERROR, "clCreateCommandQueue() error: " + toString(error));
}

void CommandQueue::release()
{
    clReleaseCommandQueue(id);
    id = 0;

    context = nullptr;
}

void CommandQueue::join() const
{
    clFinish(id);
}

const Context& CommandQueue::getContext() const
{
    return *context;
}

void CommandQueue::enqueueKernel(Kernel& _kernel, const coords_t& _globalWorkSize, cl_event* _event) const
{
	cl_int error = clEnqueueNDRangeKernel(id, _kernel(), _globalWorkSize.size(), nullptr, _globalWorkSize.data(), nullptr, 0, nullptr, _event);

	if (error != CL_SUCCESS)
        Error::add(ErrorType::USER_ERROR, "Enqueue kernel: " + toString(error));
}

void CommandQueue::enqueueRead(const Buffer& _buffer, cl_bool _blockingRead, size_t _offset, size_t _byteSize, void* _hostPtr, cl_event* _event) const
{
	cl_int error = clEnqueueReadBuffer(id, _buffer(), _blockingRead, _offset, _byteSize, _hostPtr, 0, nullptr, _event);

    if (error != CL_SUCCESS)
        Error::add(ErrorType::USER_ERROR, "Unable to enqueue a buffer reading: " + toString(error));
}

void CommandQueue::enqueueWrite(const Buffer& _buffer, cl_bool _blockingWrite, size_t _offset, size_t _byteSize, const void* _hostPtr, cl_event* _event) const
{
	cl_int error = clEnqueueWriteBuffer(id, _buffer(), _blockingWrite, _offset, _byteSize, _hostPtr, 0, nullptr, _event);

    if (error != CL_SUCCESS)
        Error::add(ErrorType::USER_ERROR, "Unable to enqueue a buffer writing: " + toString(error));
}

void CommandQueue::enqueueRead(const Tensor& _tensor, const cl_bool& _blockingRead, cl_event* _event) const
{
	enqueueRead(_tensor.getBuffer(), _blockingRead, 0, _tensor.nElements() * sizeof(Tensor::value_type), const_cast<void*>((void*)_tensor.data()), _event);
}

void CommandQueue::enqueueWrite(const Tensor& _tensor, const cl_bool& _blockingWrite, cl_event* _event) const
{
	enqueueWrite(_tensor.getBuffer(), _blockingWrite, 0, _tensor.nElements() * sizeof(Tensor::value_type), const_cast<void*>((void*)_tensor.data()), _event);
}

void CommandQueue::enqueueCopy(const Buffer& _first, const cl::Buffer& _second, size_t _byteSize, cl_event* _event) const
{
    cl_int error = clEnqueueCopyBuffer(id, _first(), _second(), 0, 0, _byteSize, 0, nullptr, _event);

    if (error != CL_SUCCESS)
        Error::add(ErrorType::USER_ERROR, "Unable to enqueue a buffer copy: " + toString(error));
}

void CommandQueue::enqueueBarrier(const std::vector<cl_event>& _events) const
{
    cl_int error = clEnqueueBarrierWithWaitList(id, _events.size(), _events.data(), nullptr);

	if (error != CL_SUCCESS)
        Error::add(ErrorType::USER_ERROR, "Enqueue barrier: " + toString(error));
}

void CommandQueue::enqueueBarrier() const
{
    cl_int error = clEnqueueBarrierWithWaitList(id, 0, nullptr, nullptr);

	if (error != CL_SUCCESS)
        Error::add(ErrorType::USER_ERROR, "Enqueue barrier: " + toString(error));
}

}

#endif // USE_OPENCL