#include <iostream>
#include <cstdlib>
#include "Utility/Utility.h"

int main()
{
    Tensor x{3, 4}; x.randomize(0, 10); x.cast<int>();

    std::cout << x << std::endl;
    x *= 2; std::cout << x<< std::endl;
    x /= 2; std::cout << x << std::endl;

    return 0;
}