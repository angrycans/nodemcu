
#pragma once
#include "../bsp-config.h"
#if BSP_MODULE_LED

#include "esp32-hal-log.h"

#define TAG_LED "LED"

class LED
{
private:
public:
    LED()
    {
        ESP_LOGI(TAG_LED, "init...");
        pinMode(BSP_LED_PIN, OUTPUT);
    }

    void loop()
    {
        digitalWrite(BSP_LED_PIN, (millis() / 1000) % 2);
    }
};

#endif
