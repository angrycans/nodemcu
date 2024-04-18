#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <Arduino.h>
// #include <TinyGPS++.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h> // This file system is used.
#include "HAL_Def.h"
#include "HAL_Ublox_Def.h"
#include <Battery.h>

#include "OLEDDisplayUi.h"

#include "config.h"

#include "helper.hpp"

#include "../race.h"

extern String ErrInfo;
#include "SDLogger.h"

// extern void boot_print_reset_reason();

class Race;
extern Race race;
extern char logbuff[100];

extern SDLogger logger;
// extern TinyGPSPlus gps;
extern GPS_t gps_data_2;
extern GPS_t gps_data_last;

extern Battery battery;

extern char DataFileName[64];
extern char DataFileDir[12];

extern bool B_SDCARDOK;

extern double RecordKmph;
extern File dataFile;
extern bool isSetTime;
extern double KMPH;

// battery
extern int voltage, usage;

// #include "GY521.h"
// extern GY521 mpu;

#include "helper_3dmath.h"

extern VectorInt16 aaReal;  // [x, y, z]            gravity-free accel sensor measurements
extern VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
extern VectorFloat gravity; // [x, y, z]            gravity vector
extern float ypr[3];        // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
extern float gforce;
extern float gforce_last;

namespace HAL
{

    void Init();
    void Update();

    /* LED */
    void LED_Init();
    void LED_Update();

    /* BTN*/
    void BTN_Init();
    void BTN_Update();

    /* I2C */
    void I2C_Init(bool startScan);

    /* IMU */

    void IMU_Init();
    void IMU_Update();
    void IMU_Calibration();

    /* SDCARD*/
    void SDCARD_Init();

    /* DISPLAY*/
    void DISPLAY_Init();
    void DISPLAY_Update();

    /* BLE*/
    void BLE_Init();
    void BLE_Update();

    /*GPS*/
    void GPS_Init();
    void GPS_Update();

    /*wifi*/
    void WIFI_Init();

    /*POWER*/
    void POWER_Init();
    void POWER_Update();
    void Power_GetInfo();

}

#endif
