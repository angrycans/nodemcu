#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <Arduino.h>

#include "OLEDDisplayUi.h"

#include "config.h"
#include "HAL_Def.h"
#include "SDLogger.h"

extern String ErrInfo;
extern SDLogger logger;

namespace HAL
{

    void Init();
    void Update();

    /* LED */
    void LED_Init();
    void LED_Update();

    /* I2C */
    void I2C_Init(bool startScan);

    /* IMU */
    void IMU_Init();
    void IMU_Update();

    /* SDCARD*/
    void SDCARD_Init();

    /* DISPLAY*/
    void DISPLAY_Init();
    void DISPLAY_Update();

    /* BLE*/
    void BLE_Init();
    void BLE_Update();

}

#endif
