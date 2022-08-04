#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <Arduino.h>
#include "HAL_Def.h"

namespace HAL
{
    void Init();
    void Update();

/* IMU */
    void IMU_Init();
    void IMU_Update();

}

#endif
