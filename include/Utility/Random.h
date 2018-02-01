#pragma once

#include <initializer_list>
#include <cstdlib>
#include <vector>

template <typename T>
struct default_max
{
    static T get() { return 1; }
};

class Random
{
    public:
        static bool nextBool();

        template <typename T>
        static T next(T _min = 0, T _max = default_max<T>::get()) // range is [_min, _max[
        {
            return _min +  (_max-_min) * (T)rand() / RAND_MAX;
        }


        template <typename T>
        static T& element(std::initializer_list<T> _elements)
        {
            return *(_elements.begin() + Random::next(0, _elements.size()));
        }

        template <typename T>
        static T& element(std::vector<T>& _elements)
        {
            return _elements[ Random::next(0, _elements.size()) ];
        }

        template <typename T>
        static const T& element(const std::vector<T>& _elements)
        {
            return _elements[ Random::next(0, _elements.size()) ];
        }


        static void init();
        static void setSeed(long int _seed);

        static long int getSeed();

    private:
        static long int seed;
};

template <>
struct default_max<int>
{
    static int get() { return 2; }
};

template <>
int Random::next(int _min, int _max);