#pragma once

#include <CL/opencl.h>
#include <string>
#include <vector>
#include <map>

class Tensor;
using coords_t = std::vector<size_t>;

#ifdef USE_OPENCL

namespace cl
{

enum DeviceType {CPU, GPU, ALL};


std::vector<cl_platform_id> getPlatformsIds();
std::vector<cl_device_id> getDeviceIds(DeviceType _deviceType, cl_platform_id _platformId);

std::string getVersion(cl_platform_id _platformId);

template <typename T>
class Wrapper
{
    public:
        Wrapper(): id(0) {}
        virtual ~Wrapper() {}

        Wrapper(const Wrapper&& c) = delete;
        operator=(const Wrapper& c) = delete;

        virtual void release() = 0;

              T& operator()()       { return id; }
        const T& operator()() const { return id; }

        operator bool() const { return id != 0; }

    protected:
        T id;
};

class Program;
class Context: public Wrapper<cl_context>
{
    public:
        Context();
        Context(const Context&) = delete;

        ~Context();

        void create(DeviceType _deviceType);
        void create(cl_device_id _deviceId);
        void release();

        const Program& getProgram(const std::string& _path);

        cl_device_id getDeviceId() const;

    private:
        cl_device_id deviceId;

        std::map<std::string, Program> programs;
};

class Program: public Wrapper<cl_program>
{
    public:
        Program() {}
        Program(const Program&) = delete;

        ~Program();

        void create(const Context& _context, const std::string& _path);
        void release();

        std::string getBuildLog(cl_device_id _deviceId) const;

        static void setBaseDirectory(const std::string& _baseDirectory = "");

    private:
        static std::string baseDirectory;
};

class Kernel: public Wrapper<cl_kernel>
{
    public:
        Kernel(): program(nullptr) {}
        Kernel(const Kernel&) = delete;

        ~Kernel();

        void create(const Program& _program, const std::string& _name);
        void release();

        void setArg(cl_uint _index, size_t _size, const void* _value);
        void setArg(cl_uint _index, const Tensor& _value);

        template <typename T>
        void setArg(cl_uint _index, T _value)
        { setArg(_index, sizeof(T), &_value); }

    private:
        const Program* program;
        std::string name;
};

// TODO: Use clCreateImage2D for matrix ?
class Buffer: public Wrapper<cl_mem>
{
    public:
        Buffer(): context(nullptr) {}
        Buffer(const Buffer&) = delete;

        ~Buffer();

        void create(const Context& _context, cl_mem_flags _flags, size_t _byteSize, void* _hostPtr = nullptr);
        void release();

    private:
        const Context* context;
        cl_mem_flags flags;

        friend void swap(Buffer& first, Buffer& second)
        {
            using std::swap;

            swap(first.id, second.id);
        }
};

class CommandQueue: public Wrapper<cl_command_queue>
{
    public:
        CommandQueue(): context(nullptr) {}
        CommandQueue(const CommandQueue&) = delete;
        CommandQueue(const Context& _context, bool _inOrder = true) { create(_context, _inOrder); }

        ~CommandQueue();

        void create(const Context& _context, bool _inOrder = true);
        void release();

        void join() const;
        const Context& getContext() const;

        void enqueueKernel(Kernel& _kernel, const coords_t& _globalWorkSize, cl_event* _event = nullptr) const;

        void enqueueRead(const Buffer& _buffer, cl_bool _blockingRead, size_t _offset, size_t _byteSize, void* _hostPtr, cl_event* _event = nullptr) const;
        void enqueueWrite(const Buffer& _buffer, cl_bool _blockingWrite, size_t _offset, size_t _byteSize, const void* _hostPtr, cl_event* _event = nullptr) const;

        void enqueueRead(const Tensor& _tensor, const cl_bool& _blockingRead = CL_FALSE, cl_event* _event = nullptr) const;
        void enqueueWrite(const Tensor& _tensor, const cl_bool& _blockingWrite = CL_FALSE, cl_event* _event = nullptr) const;

        void enqueueCopy(const Buffer& _first, const Buffer& _second, size_t _byteSize, cl_event* _event = nullptr) const;

        void enqueueBarrier(const std::vector<cl_event>& _events) const;
        void enqueueBarrier() const;

    private:
        const Context* context;
        bool inOrder;
};

}

#endif // USE_OPENCL