/*
  SDLogger.h - Library for logging Warning, Error and Info to SD card and Serial console.
  Created by Laukik Ragji, December 06, 2016.
  Released with MIT.
*/

#ifndef RACE_h
#define RACE_h

#include "Arduino.h"
#include <TinyGPS++.h>
#include "SDLogger.h"

enum DeviesStatus
{
  d_Setup,
  d_Looping,
  d_preRecord,
  d_Recording,
  d_RecordToLoop,
  d_Unknown
};

struct RaceStatuS
{
  DeviesStatus status;
  unsigned long timer = 0;
};

class Race
{
private:
  RaceStatuS r_status;
  int last_satellites = 0;
  // bool __dev__ = false;
  // SDLogger *logger;

public:
  unsigned long lastGpsUpdateTimer = 0;
  TinyGPSPlus last_gps;
  float lat1;
  float lng1;
  float lat2;
  float lng2;

  bool sessionActive = false;

  unsigned long sessionTime = 0;
  unsigned long bestSessionTime = 0;

  int currentLap = 0;
  int bestLap = 0;
  int totolLap = 0;
  double maxspeed = 0;

  Race()
  {
    r_status.status = d_Setup;
    r_status.timer = millis();
  }

  void setStatus(DeviesStatus _status)
  {
    r_status.status = _status;
    r_status.timer = millis();

    // if ()
  }

  RaceStatuS getStatus()
  {
    return r_status;
  }

  String getStatusName()
  {
    switch (r_status.status)
    {
    case d_Setup:
      return "setup";
      break;
    case d_Looping:
      return "Looping";
      break;
    case d_preRecord:
      return "preRecord";
      break;
    case d_Recording:
      return "Recording";
      break;
    case d_RecordToLoop:
      return "RecordToLoop";
      break;

    default:
      return "Unknown";
      break;
    }
  }

  void resetSession()
  {
    sessionActive = false;

    sessionTime = 0;
    bestSessionTime = 0;

    currentLap = 0;
    bestLap = 0;
    totolLap = 0;
    maxspeed = 0;
  }

  boolean segmentsIntersect(float lat1, float lon1, float lat2, float lon2, float finishLineLat1, float finishLineLon1, float finishLineLat2, float finishLineLon2)
  {
    // does line(p1, p2) intersect line(p3, p4)
    float fDeltaX = lat2 - lat1;
    float fDeltaY = lon2 - lon1;
    float da = finishLineLat2 - finishLineLat1;
    float db = finishLineLon2 - finishLineLon1;
    if ((da * fDeltaY - db * fDeltaX) == 0)
    {
      // The segments are parallel
      return false;
    }
    float s = (fDeltaX * (finishLineLon1 - lon1) + fDeltaY * (lat1 - finishLineLat1)) / (da * fDeltaY - db * fDeltaX);
    float t = (da * (lon1 - finishLineLon1) + db * (finishLineLat1 - lat1)) / (db * fDeltaX - da * fDeltaY);

    return (s >= 0) && (s <= 1) && (t >= 0) && (t <= 1);
  }

  String formatSessionTime(unsigned long sessionTime)
  {
    unsigned long minutes = sessionTime / 60000;
    unsigned long seconds = (sessionTime / 1000) - ((sessionTime / 60000) * 60);
    unsigned long tenths = (sessionTime / 100) % 10;
    if (seconds < 10)
      return String(minutes) + ":0" + String(seconds) + ":" + String(tenths);
    else
      return String(minutes) + ":" + String(seconds) + ":" + String(tenths);
  }

  // void setDev(SDLogger *_logger)
  // {
  //   __dev__ = true;
  //   logger = _logger;
  // }
};

#endif