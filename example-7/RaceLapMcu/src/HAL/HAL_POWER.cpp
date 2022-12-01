
#include "HAL/HAL.h"

Battery battery(3000, 4200, CONFIG_BATTERY_PIN);

void HAL::POWER_Init()
{
    pinMode(CONFIG_BATDET_PIN, INPUT_PULLUP);
    battery.begin(1200, 1.0);
}

void HAL::POWER_Update()
{
}
