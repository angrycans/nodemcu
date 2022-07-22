/*
  SDLogger.h - Library for logging Warning, Error and Info to SD card and Serial console.
  Created by Laukik Ragji, December 06, 2016.
  Released with MIT.
*/

#ifndef RACE_hpp
#define RACE_hpp

#include "Arduino.h"
#include <TinyGPS++.h>
#include <LinkedList.h>
#include "SDLogger.h"
#include <TimeLib.h>
#include <sys/time.h> // struct timeval

enum DeviesStatus
{
  d_Setup,         //系统初始化
  d_gps_searching, // gps模块搜星中
  d_Looping,       //系统loop中 等待记录中
  d_preRecord,     //速度>RecordKmph cd=preRecordCd 准备记录中
  d_Recording,     //数据记录中
  d_RecordToStop,  //速度<RecordKmph cd=recordtoLoopCd 记录准备停止中
  d_Stop,          //记录停止
  d_Unknown        //未知状态
};

struct RaceStatuS
{
  DeviesStatus status;
  unsigned long timer = 0;
};

class LapInfo
{
public:
  int maxspeed;
  int avespeed;
  unsigned long time;
  // unsigned long time2;
  int off;
  int difftime;
  // time_t p0_at;
  // time_t p1_at;
  // // int p0_at_ms;
  // // int p1_at_ms;
  //  point_t p0;
  //  point_t p1;

  // bool isbest;

  void dump(SDLogger *_logger)
  {
    char tmp[100];
    snprintf(tmp, sizeof(tmp), "%ld,%d,%d,%d,%d", time, off, difftime, maxspeed, avespeed);
    _logger->LogInfo(tmp);
  }
};

class Race
{
private:
  RaceStatuS r_status;
  int last_satellites = 0;

  // bool __dev__ = false;
  // SDLogger *logger;

public:
  // float lat1;
  // float lng1;
  // float lat2;
  // float lng2;
  ESPLinkedList<LapInfo> *lapInfoList;
  int trackplan_size = 0;
  double **trackplan;
  unsigned long lastGpsUpdateTimer = 0;
  TinyGPSPlus last_gps;

  bool sessionActive = false;

  unsigned long sessionTime = 0;
  unsigned long bestSessionTime = 0;

  int currentLap = 0;
  int bestLap = 0;
  int totalLap = 0;
  double maxspeed = 0;
  double avespeed = 0;
  double lapmaxspeed = 0;
  double lapavespeed = 0;

  Race()
  {
    r_status.status = d_Setup;
    r_status.timer = millis();
    lapInfoList = new ESPLinkedList<LapInfo>();
  }

  void setStatus(DeviesStatus _status)
  {
    r_status.status = _status;
    r_status.timer = millis();

    switch (_status)
    {
    case d_Recording:
      if (!sessionActive)
      {
        sessionActive = true;
        sessionTime = millis();
        totalLap = 0;
        maxspeed = 0;
        avespeed = 0;
        bestLap = 0;
        bestSessionTime = 0;
      }

      break;
    case d_Looping:

      break;

    default:
      break;
    }

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
    case d_RecordToStop:
      return "RecordToStop";
      break;
    case d_Stop:
      return "Stop";
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
    totalLap = 0;
    maxspeed = 0;
  }

  void setTrace(double arr[][4], int _size)
  {
    trackplan_size = _size;
    trackplan = new double *[_size];

    for (int i = 0; i < trackplan_size; i++)
    {
      trackplan[i] = new double[4];
      trackplan[i][0] = arr[i][0];
      trackplan[i][1] = arr[i][1];
      trackplan[i][2] = arr[i][2];
      trackplan[i][3] = arr[i][3];
      // char tmp1[100];
      // sprintf(tmp1, "%d %.8lf %.8lf %.8lf %.8lf", i, trackplan[i][0], trackplan[i][1], trackplan[i][2], trackplan[i][3]);
      // Serial.println(tmp1);
    }
  }

  bool nearTarck(float lat, float lon)
  {

    return false;
    if (trackplan_size > 0)
    {

      unsigned long distanceKmToTarck =
          (unsigned long)TinyGPSPlus::distanceBetween(
              lat,
              lon,
              trackplan[trackplan_size - 1][0],
              trackplan[trackplan_size - 1][1]) /
          1000;

      if ((distanceKmToTarck - 5) < 0)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {

      return false;
    }
  }

  //计算圈速
  void computerSession(TinyGPSPlus *_gps)
  {

    double speed = _gps->speed.kmph();

    //保存速度
    if (speed > maxspeed)
    {
      maxspeed = speed;
    }
    if (speed > lapmaxspeed)
    {
      lapmaxspeed = speed;
    }

    avespeed = (avespeed + speed) / 2;
    lapavespeed = (lapavespeed + speed) / 2;

    computerLapinfo(_gps);

    last_gps = *_gps;
  }

  void computerLapinfo(TinyGPSPlus *_gps)
  {
    // tmElements_t te1;
    // te1.Second = _gps->time.second();
    // te1.Hour = _gps->time.hour();
    // te1.Minute = _gps->time.minute();
    // te1.Day = _gps->date.day();
    // te1.Month = _gps->date.month();
    // te1.Year = _gps->date.year() - 1970; // Y2K, in seconds = 946684800UL

    // int timestamp = (int)(makeTime(te1) * 1000 + _gps->time.centisecond() * 10);

    float off = 0.0f;

    if (trackplan_size > 0)
    {
      if (segmentsIntersect(_gps->location.lat(), _gps->location.lng(), last_gps.location.lat(), last_gps.location.lng(), trackplan[trackplan_size - 1][0], trackplan[trackplan_size - 1][1], trackplan[trackplan_size - 1][2], trackplan[trackplan_size - 1][3]))
      {
#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "segmentsIntersect checked");
        logger.LogInfo(logbuff);
#endif

        point_t cp;
        IntersectPoint1({_gps->location.lat(), _gps->location.lng()}, {last_gps.location.lat(), last_gps.location.lng()}, {trackplan[trackplan_size - 1][0], trackplan[trackplan_size - 1][1]}, {trackplan[trackplan_size - 1][2], trackplan[trackplan_size - 1][3]}, &cp);

        if (lapInfoList->size() > 0)
        {
          float d = Distance({_gps->location.lat(), _gps->location.lng()}, cp);
          off = (d * 1000 / _gps->speed.kmph()) * 60 * 60;
        }
        LapInfo lapinfo;
        lapinfo.time = millis();
        // lapinfo.time2 = timestamp;
        lapinfo.off = (int)off;
        lapinfo.difftime = lapInfoList->size() == 0 ? 0 : (int)(lapinfo.time - sessionTime);
        lapinfo.maxspeed = lapmaxspeed;
        lapinfo.avespeed = lapavespeed;
        // lapinfo.p0_at = makeTime(te0);
        // lapinfo.p1_at = makeTime(te1);
        // lapinfo.p0_at_ms = gps.time.centisecond();
        lapInfoList->add(lapinfo);

        currentLap += 1;
        totalLap += 1;

        // this is the best or first lap
        if ((bestSessionTime > lapinfo.time - sessionTime) || (bestSessionTime == 0))
        { // test for session time and also for the very first lap, when bestSesstionTime is 0
          bestSessionTime = lapinfo.time - sessionTime;
          bestLap = currentLap;
        }
#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "[%s]currlap %d,totol %d,maxspeed:%f,bestlap:%d,bestsession:%lu", formatTime(millis()), race.currentLap, race.totalLap, race.maxspeed, race.bestLap, race.bestSessionTime);
        logger.LogInfo(logbuff);
        lapinfo.dump();
#endif

        // reset the session
        sessionTime = millis();
        lapmaxspeed = 0;
        lapavespeed = 0;
      }
    }
  }
};

#endif