/*
  SDLogger.cpp - Library for logging Warning, Error and Info to SD card and Serial console.
  Created by Laukik Ragji, December 06, 2016.
  Released with MIT.
*/

#include "Arduino.h"
#include <SD.h>
#include <SPI.h>
#include "SDLogger.h"

SDLogger::SDLogger()
{
}

void SDLogger::Begin()
{

  if (SD.mkdir("RLDATA"))
  {
    Serial.println("RLLOG is created.");
  }
  Serial.println("RLLOG init.");
  logFile = SD.open("/RLDATA/log.txt", FILE_WRITE);

  if (logFile)
  {
    if (logFile.size() > 1024 * 1024 * 1024)
    {
      SD.remove("/RLDATA/log.txt");
      delay(250);
      logFile = SD.open(logFilePath, FILE_WRITE);
    }
  }

  if (logFile)
  {
    Serial.println("RLLOG init ok");
    logFile.println("\r\n\r\n-----------------------------------------------------------\r\n\r\n");
    logFile.flush();
  }
  else
  {
    Serial.println("RLLOG init error");
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