
#include "HAL/HAL.h"

Battery battery(3400, 4200, 4);

void HAL::POWER_Init()
{
    battery.begin(5000, 1.0);
}

void HAL::POWER_Update()
{
}
