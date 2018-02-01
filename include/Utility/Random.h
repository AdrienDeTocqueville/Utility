#pragma once

#include <initializer_list>
#include <type_traits>
#include <cstdlib>
#include <vector>

#include <iostream>


class Random
{
    public:
        static bool nextBool();

        template <typename T>
        static typename std::enable_if<!std::is_integral<T>::value, T>::type next(T _min = 0, T _max = 1) // range is [_min, _max[
        {
            return _min +  (_max-_min) * (T)rand() / RAND_MAX;
        }

        template <typename T>
        static typename std::enable_if<std::is_integral<T>::value, T>::type next(T _min = 0, T _max = 2) // range is [_min, _max[
        {
            return _min + (rand() % (_max-_min));
        }


        template <typename T>
        static const T& element(std::initializer_list<T> _elements)
        {
            return *(_elements.begin() + Random::next((size_t)0, _elements.size()));
        }

        template <typename T>
        static T& element(std::vector<T>& _elements)
        {
            return _elements[ Random::next((size_t)0, _elements.size()) ];
        }

        template <typename T>
        static const T& element(const std::vector<T>& _elements)
        {
            size_t i = Random::next((size_t)0, _elements.size());
            if (i >= _elements.size())
            std::cout << "erreur" << std::endl;
            return _elements[ i ];
        }


        static void init();
        static void setSeed(long int _seed);

        static long int getSeed();

    private:
        static long int seed;
};