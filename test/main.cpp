#include <iostream>
#include <cstdlib>
#include "Utility/Utility.h"

int main()
{
    std::cout << Random::getSeed() << std::endl;

    std::cout << CL_DEVICE_TYPE_ALL << std::endl;

    cl::getDeviceIds(cl::DeviceType::ALL, cl::getPlatformsIds().front());

    return 0;
}