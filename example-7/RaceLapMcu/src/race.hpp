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
  d_Setup,
  d_gps_searching,
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

class LapInfo
{
public:
  int maxspeed;
  int avespeed;
  unsigned long time;
  unsigned long time2;
  int off;
  unsigned long difftime;
  // time_t p0_at;
  // time_t p1_at;
  // // int p0_at_ms;
  // // int p1_at_ms;
  //  point_t p0;
  //  point_t p1;

  // bool isbest;
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
  float **trackplan;
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
    totalLap = 0;
    maxspeed = 0;
  }

  void setTrace(float arr[][4], int _size)
  {
    // lat1 = atof(doc["lat1"]);
    // lng1 = atof(doc["lng1"]);
    // lat2 = atof(doc["lat2"]);
    // lng2 = atof(doc["lng2"]);
    // Serial.print[doc["trackplan"]]
    trackplan_size = _size;
    trackplan = new float *[_size];

    for (int i = 0; i < trackplan_size; i++)
    {
      trackplan[i] = new float[4];
      trackplan[i][0] = arr[i][0];
      trackplan[i][1] = arr[i][1];
      trackplan[i][2] = arr[i][2];
      trackplan[i][3] = arr[i][3];
      char tmp1[100];
      sprintf(tmp1, "%d %f %f %f %f", i, trackplan[i][0], trackplan[i][1], trackplan[i][2], trackplan[i][3]);
      Serial.println(tmp1);
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
    double lat = _gps->location.lat();
    double lng = _gps->location.lng();
    // double altitude = gps.altitude.meters();
    double speed = _gps->speed.kmph();
    // int year = gps.date.year();
    // int month = gps.date.month();
    // int day = gps.date.day();
    // int hour = gps.time.hour();
    // int minute = gps.time.minute();
    // int second = gps.time.second();
    // int csecond = gps.time.centisecond();
    // double deg = gps.course.deg();
    // int satls = gps.satellites.value();

    double lapmaxspeed = 0;
    double lapavespeed = 0;

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

    //有赛道终点信息
    if (trackplan_size > 0)
    {
      if (segmentsIntersect(lat, lng, last_gps.location.lat(), last_gps.location.lng(), trackplan[trackplan_size - 1][0], trackplan[trackplan_size - 1][1], trackplan[trackplan_size - 1][2], trackplan[trackplan_size - 1][3]))
      {
#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "segmentsIntersect checked");
        logger.LogInfo(logbuff);
#endif

        tmElements_t te1;
        te1.Second = _gps->time.second();
        te1.Hour = _gps->time.hour();
        te1.Minute = _gps->time.minute();
        te1.Day = _gps->date.day();
        te1.Month = _gps->date.month();
        te1.Year = _gps->date.year() - 1970; // Y2K, in seconds = 946684800UL

        int te1_ms = _gps->time.centisecond() * 10;

        // tmElements_t te0;
        // te0.Second = last_gps.time.second();
        // te0.Hour = last_gps.time.hour();
        // te0.Minute = last_gps.time.minute();
        // te0.Day = last_gps.date.day();
        // te0.Month = last_gps.date.month();
        // te0.Year = last_gps.date.year() - 1970; // Y2K, in seconds = 946684800UL
        // int te0_ms = last_gps.time.centisecond() * 10;

        point_t cp;
        IntersectPoint1({lat, lng}, {last_gps.location.lat(), last_gps.location.lng()}, {trackplan[trackplan_size - 1][0], trackplan[trackplan_size - 1][1]}, {trackplan[trackplan_size - 1][2], trackplan[trackplan_size - 1][3]}, &cp);

        // int ms = (int)(makeTime(te1) * 1000 + _gps->time.centisecond() - makeTime(te0) * 1000 + last_gps.time.centisecond());

        // var distance_point0 = turf.distance([prevprevItem[1], prevprevItem[2]], [prev_intersectpoint[1], prev_intersectpoint[0]], { units: 'kilometers' });
        //           var distance_point1 = turf.distance([prevPoint[1], prevPoint[2]], [intersectP[1], intersectP[0]], { units: 'kilometers' });
        //           let off0 = (distance_point0 / +prevprevItem[4]) * 60 * 60 * 1000;
        //           let off1 = (distance_point1 / +prevPoint[4]) * 60 * 60 * 1000;

        float d = 0.0f;
        float off = 0.0f;

        if (lapInfoList->size() > 0)
        {
          d = Distance({lat, lng}, cp);
          off = (d * 1000 / speed) * 60 * 60;
        }

        LapInfo lapinfo;
        lapinfo.time = millis();
        lapinfo.time2 = te1_ms - (int)off;
        lapinfo.off = (int)off;
        lapinfo.difftime = lapInfoList->size() == 0 ? 0 : lapinfo.time2 - lapInfoList->get(currentLap).time2;
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

        // race.planInfoList.

#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "[%s]currlap %d,totol %d,maxspeed:%f,bestlap:%d,bestsession:%lu", formatTime(millis()), race.currentLap, race.totalLap, race.maxspeed, race.bestLap, race.bestSessionTime);
        logger.LogInfo(logbuff);
#endif

        // reset the session
        sessionTime = millis();
        lapmaxspeed = 0;
        lapavespeed = 0;
      }

      last_gps = *_gps;
    }
  }
};

#endif