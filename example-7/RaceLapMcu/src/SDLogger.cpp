/*
  SDLogger.cpp - Library for logging Warning, Error and Info to SD card and Serial console.
  Created by Laukik Ragji, December 06, 2016.
  Released with MIT.
*/

#include "SDLogger.h"

#include "./HAL/HAL.h"

SDLogger::SDLogger()
{
}

void SDLogger::Begin(bool sd)
{

  if (sd)
  {
    if (SD.mkdir("/XLAPDATA"))
    {
      Serial.println("XLAPDATA Dir is created.");
    }
    else
    {
      Serial.println("XLAPDATA Dir create error");
    }
    Serial.println("XLAPDATA log init ...");
    logFile = SD.open("/XLAPDATA/log.txt", FILE_WRITE);

    if (logFile)
    {
      Serial.print("log.txt size=");
      Serial.print(logFile.size());
      Serial.print(" 1M=");
      Serial.println(1 * 1024 * 1024);
      if (logFile.size() > 1 * 1024 * 1024.0)
      {
        SD.remove("/XLAPDATA/log.txt");
        delay(250);

        logFile = SD.open("/XLAPDATA/log.txt", FILE_WRITE);
        delay(250);
      }
    }

    if (logFile)
    {
      Serial.println("XLAPLOG init ok have SDCARD");
      logFile.println("\r\n\r\n-----------------------------------------------------------\r\n\r\n");
      logFile.flush();
    }
    else
    {
      Serial.println("XLAPLOG init error");
      ErrInfo += "XLAPLOG init error\n";
      // HAL::DISPLAY_Update();
    }
  }
  else
  {
    Serial.print("RLLOG init ok no SDCARD");
  }

  // char logbuff[100];
  // snprintf(logbuff, sizeof(logbuff), "logbuff %d", sizeof(logbuff));

  // Serial.println(logbuff);

  // snprintf(logbuff, sizeof(logbuff), "he %d", sizeof(logbuff));

  // Serial.println(logbuff);

  // snprintf(logbuff, sizeof(logbuff), "nihao a e fil and text transformations: colorize,e filters and text transformations: colorize, %d", sizeof(logbuff));

  // Serial.println(logbuff);
  // WriteToSD("hello world");
}

void SDLogger::LogInfo(String info)
{

  Serial.println(info);

  WriteToSD(info);
}

void SDLogger::LogInfo(const char *info)
{

  Serial.println(info);

  WriteToSD(info);
}

void SDLogger::WriteToSD(String data)
{

  if (logFile)
  {
    logFile.println(data);
    logFile.flush();
  }
}

void SDLogger::WriteToSD(const char *data)
{

  if (logFile)
  {
    logFile.println(data);
    logFile.flush();
  }
}