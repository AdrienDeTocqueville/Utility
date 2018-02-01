#include <iostream>
#include <cstdlib>
#include "Utility/Utility.h"

int main()
{
    Random::init();

    std::cout << Random::next(2.0f, 10.0f) << std::endl;

    return 0;
}