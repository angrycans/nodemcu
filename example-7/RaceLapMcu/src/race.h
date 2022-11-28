/*
  SDLogger.h - Library for logging Warning, Error and Info to SD card and Serial console.
  Created by Laukik Ragji, December 06, 2016.
  Released with MIT.
*/

#ifndef RACE_hpp
#define RACE_hpp

#include "HAL/HAL.h"
#include "Arduino.h"
#include <TinyGPS++.h>
#include <LinkedList.h>
#include "SDLogger.h"
#include <TimeLib.h>
#include <sys/time.h> // struct timeval
#include "helper.hpp"

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
  String TrackName = "uname";
  String UID = "UID99879";
  String UNAME = "Acans";

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

  Race();

  void setStatus(DeviesStatus _status);

  RaceStatuS getStatus();

  String getStatusName();

  void resetSession();

  void setTrace(double arr[][4], int _size);

  bool nearTarck(float lat, float lon);

  //计算圈速
  void computerSession(TinyGPSPlus *_gps);

  void computerLapinfo(TinyGPSPlus *_gps);

  void getTrackInfo();

  String getHeader(int year, int month, int day, int hour, int minute, int second);
};

#endif