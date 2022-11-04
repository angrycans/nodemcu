#include "HAL/HAL.h"

void HAL::Init()
{
    // Serial.begin(115200);
    Serial.println(VERSION_FIRMWARE_NAME);
    Serial.println("Version: " VERSION_SOFTWARE);
    Serial.println("Author: " VERSION_AUTHOR_NAME);

    //  HAL::IMU_Init();
}

void HAL::Update()
{
    //  HAL::IMU_Update();
}
