
#include "HAL/HAL.h"

void HAL::LED_Init()
{
    pinMode(LED_RUN, OUTPUT);
    // pinMode(LED_DEV_BUILTIN, OUTPUT);
}

void HAL::LED_Update()
{
    digitalWrite(LED_RUN, (millis() / 1000) % 2);
    //  digitalWrite(LED_DEV_BUILTIN, (millis() / 1000) % 2);
}
