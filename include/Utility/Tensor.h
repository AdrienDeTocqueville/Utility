#pragma once

#include <initializer_list>
#include <ostream>
#include <vector>

#include "clWrapper.h"

using std::size_t;
using coords_t = std::vector<size_t>;

std::ostream& operator<<(std::ostream& os, const coords_t& coords);
coords_t operator/(const coords_t& coords, const int& a);

class Tensor
{
    public:
        #ifdef TENSOR_DOUBLE
            using value_type = double;
        #else
            using value_type = float;
        #endif

        static Tensor identityMatrix(size_t _dimension);

        Tensor();
        Tensor(Tensor&& _tensor);
        Tensor(const Tensor& _tensor);

        Tensor(const coords_t&  _dimensions, const value_type& _value = 0.0);
        Tensor(std::initializer_list<size_t> _dimensions, const value_type& _value = 0.0);

        ~Tensor();

        Tensor& operator=(Tensor _tensor);


        const coords_t&  size() const;
        size_t size(size_t _dimension) const;
        size_t nDimensions() const;
        size_t nElements() const;

        void flatten();

        void resize(const coords_t&  _dimensions, const value_type& _value = 0.0);
        void resizeAs(const Tensor& _tensor, const value_type& _value = 0.0);

        void fill(value_type _value);
        void round(unsigned _decimals = 0);
        void randomize(value_type _min = 0.0, value_type _max = 1.0);

        bool isnan() const;

        value_type length() const;
        value_type length2() const;

        value_type& max();
        const value_type& max() const;

        Tensor max(size_t _dim) const;

        coords_t  argmax() const;

        value_type* data();
        const value_type* data() const;


        Tensor getTranspose() const;

        size_t getStride(size_t i) const;
        size_t getIndex(const coords_t& _indices) const;

        template <typename T> void cast();

        #ifdef USE_OPENCL
        void openCL(const cl::Context& _context, cl_mem_flags _flags = CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR) const; // CL_MEM_USE_HOST_PTR || CL_MEM_COPY_HOST_PTR
        void releaseCL();

        const cl::Buffer& getBuffer() const;
        #endif // USE_OPENCL


        bool operator==(const Tensor& _tensor);
        bool operator!=(const Tensor& _tensor);

        value_type& operator[](size_t  _index);
        const value_type& operator[](size_t  _index) const;

        value_type& operator()(coords_t  _indices);
        value_type& operator()(size_t i0);
        value_type& operator()(size_t i0, size_t i1);
        value_type& operator()(size_t i0, size_t i1, size_t i2);

        const value_type& operator()(coords_t  _indices) const;
        const value_type& operator()(size_t i0) const;
        const value_type& operator()(size_t i0, size_t i1) const;
        const value_type& operator()(size_t i0, size_t i1, size_t i2) const;

        void operator+=(const Tensor& _tensor);
        void operator-=(const Tensor& _tensor);
        void operator*=(const value_type& s);
        void operator/=(const value_type& s);

        void addOuterProduct(const Tensor& a, const Tensor& b);

        friend Tensor operator+(const Tensor& a, const Tensor& b);
        friend Tensor operator-(const Tensor& a, const Tensor& b);

        friend Tensor operator*(const value_type& s, const Tensor& t);

    private:
        coords_t dimensions;
        coords_t strides;
        std::vector<value_type> values;

        #ifdef USE_OPENCL
        mutable cl::Buffer buffer;
        #endif // USE_OPENCL

        friend void swap(Tensor& first, Tensor& second)
        {
            using std::swap;

            swap(first.dimensions, second.dimensions);
            swap(first.strides, second.strides);
            swap(first.values, second.values);

            #ifdef USE_OPENCL
            swap(first.buffer, second.buffer);
            #endif // USE_OPENCL
        }
};

template <typename T>
void Tensor::cast()
{
    for (Tensor::value_type& v: values)
        v = (T)v;
}

Tensor::value_type dot(const Tensor& a, const Tensor& b);

void outerProduct(Tensor& result, const Tensor& a, const Tensor& b);

void convolve(Tensor& result, const Tensor& kernel, const Tensor& src);

void mulmm(Tensor& result, const Tensor& a, const Tensor& b);
void mulmtm(Tensor& result, const Tensor& a, const Tensor& b);
void mulmmt(Tensor& result, const Tensor& a, const Tensor& b);

void mulmv(Tensor& result, const Tensor& a, const Tensor& b);

Tensor operator+(const Tensor& a, const Tensor& b);
Tensor operator-(const Tensor& a, const Tensor& b);

Tensor operator*(const Tensor::value_type& a, const Tensor& b);

bool operator==(const Tensor& a, const Tensor& b);
//bool operator!=(const Tensor& a, const Tensor& b);

std::ostream& operator<<(std::ostream& os, const Tensor& t);

Tensor Vector(std::initializer_list<Tensor::value_type> _data);

Tensor Matrix(std::vector<Tensor>& _data);
Tensor Matrix(std::initializer_list<Tensor> _data);
Tensor Matrix(std::initializer_list<std::initializer_list<Tensor::value_type>> _data);
