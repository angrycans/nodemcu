#pragma once

#include <Arduino.h>
#include <queue.h>
#include "../../acans-bsp/bsp-config.h"
#include "../../acans-bsp/gps/gps.h"
#include <mooncake.h>
#include "../app.h"

struct TrackLapPoint
{
    double p1_x, p1_y;
    double p2_x, p2_y;
};

// struct TrackInfo{
//     string name;
//     TrackLapPoint lapPonit[];

// };

enum DeviesStatus
{
    gps_Setup,        // 系统初始化
    gps_searching,    // gps模块搜星中
    gps_loop,         // 系统loop中 等待记录中
    gps_preRecord,    // 速度>RecordKmph cd=preRecordCd 准备记录中
    gps_recording,    // 数据记录中
    gps_recordToStop, // 速度<RecordKmph cd=recordtoLoopCd 记录准备停止中
    gps_stop,         // 记录停止
                      // gps_stopToLoop,   // 记录停止
    gps_Unknown       // 未知状态

};

struct RaceStatuS
{
    DeviesStatus status;
    unsigned long timer = 0;
};

class Race
{
private:
    File dataFile;
    // char DataFileName[64] = "";
    char DataFileName[32] = "";
    double CurrKmph = 0;   // current speed
    double RecordKmph = 3; //
    int PreRecordCd = 3;
    int PecordtoLoopCd = 15;
    char DataFileDir[12] = "/xlapdata/";

    RaceStatuS raceStatues;

public:
    Race();
    ~Race();
    TaskHandle_t gpsConsumerTaskHandle;

    static void gpsConsumerTask(void *param);
    void startGpsConsumerTask();

    void recordGps(GPS_t *gps);

    // void setBSP
};
