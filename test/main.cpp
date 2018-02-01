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

    const std::vector<int> v{1, 2, 3};

    for (unsigned i(0); i < 10; ++i)
        Random::element(v);

    return 0;
}