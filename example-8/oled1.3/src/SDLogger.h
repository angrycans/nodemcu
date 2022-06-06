/*
  SDLogger.h - Library for logging Warning, Error and Info to SD card and Serial console.
  Created by Laukik Ragji, December 06, 2016.
  Released with MIT.
*/

#ifndef SDLogger_h
#define SDLogger_h

#include "Arduino.h"
#include <SD.h>
#include <SPI.h>

class SDLogger
{
public:
  SDLogger();
  void Begin(bool sd);
  void LogInfo(String info);
  void LogInfo(const char *info);

private:
  String logFilePath;
  File logFile;

  void WriteToSD(String data);
  void WriteToSD(const char *data);
};

#endif