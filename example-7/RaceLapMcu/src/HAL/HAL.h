#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <Arduino.h>
#include <TinyGPS++.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h> // This file system is used.
#include <Battery.h>

#include "OLEDDisplayUi.h"

#include "config.h"
#include "helper.hpp"

#include "HAL_Def.h"
#include "SDLogger.h"

#include "race.hpp"

extern String ErrInfo;
extern SDLogger logger;
extern TinyGPSPlus gps;
extern Battery battery;

extern char DataFileName[64];
extern Race race;
extern bool B_SDCARDOK;

extern double RecordKmph;
extern File dataFile;
extern bool isSetTime;
extern double KMPH;

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

}

#endif
