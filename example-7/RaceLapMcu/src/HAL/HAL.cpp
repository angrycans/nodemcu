#include "HAL/HAL.h"

String ErrInfo = "";
SDLogger logger;
char DataFileName[64] = "";
Race race;

void HAL::Init()
{
    // Serial.begin(115200);
    Serial.println(VERSION_FIRMWARE_NAME);
    Serial.println("Version: " VERSION_SOFTWARE);
    Serial.println("Author: " VERSION_AUTHOR_NAME);

    HAL::LED_Init();
    HAL::BTN_Init();
    HAL::DISPLAY_Init();
    HAL::SDCARD_Init();
    HAL::WIFI_Init();
    HAL::BLE_Init();
    HAL::GPS_Init();
    HAL::POWER_Init();
}

void HAL::Update()
{
    HAL::LED_Update();
    HAL::DISPLAY_Update();
    HAL::BTN_Update();
    HAL::GPS_Update();
    HAL::POWER_Update();
}
