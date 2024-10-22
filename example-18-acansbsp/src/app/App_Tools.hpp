#pragma once
#ifndef App_Tools_hpp
#define App_Tools_hpp

#define D_TO_RADIANS (PI / 180.0f)
static float EARTH_RADIUS = 3956.0; // In miles.

struct point_t
{
    double x, y;
};

// #define UI_LOG(format, args...) printf(format, ##args)
// #define UI_LOG(format, ...) Serial.printf(format, ##__VA_ARGS__)

static char *formatTimeMs(unsigned long milli)
{
    static char tmp[10]; // 使用静态缓冲区避免频繁分配和释放内存
    int min = (milli / (1000 * 60)) % 60;
    int sec = (milli / 1000) % 60;
    int ms = milli % 1000;
    sprintf(tmp, "%02d:%02d.%03d", min, sec, ms);
    return tmp;
}

#endif