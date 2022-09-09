#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "HAL_Def.h"

namespace HAL
{
    void Init();
    void Update();

    /* I2C */
    void I2C_Init(bool startScan);

    /* IMU */
    void IMU_Init();
    void IMU_Update();

}

#endif
