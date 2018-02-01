#include <iostream>
#include <cstdlib>
#include "Utility/Utility.h"

int main()
{
    Random::init();

    Tensor x({3, 4});
    x.randomize();

    std::cout << x << std::endl;
    x.flatten();
    std::cout << x << std::endl;

    return 0;
}