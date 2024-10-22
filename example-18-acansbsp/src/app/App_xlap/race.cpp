
#include "race.h"

Race::Race()
{

    startGpsConsumerTask();
}

Race::~Race()
{
    BSP &bsp = BSP::getInstance();
    bsp.gps.setRunning(false);

    if (gpsConsumerTaskHandle != nullptr)
    {
        vTaskDelete(gpsConsumerTaskHandle);
        gpsConsumerTaskHandle = nullptr;
    }
}

void Race::recordGps(GPS_t *gps)
{
    double lat = gps->latitude;
    double lng = gps->longitude;
    double altitude = gps->altitude;
    double kmph = gps->speed;
    int year = gps->date.year;
    int month = gps->date.month;
    int day = gps->date.day;
    int hour = gps->date.hour;
    int minute = gps->date.min;
    int second = gps->date.sec;
    int csecond = gps->date.msec;
    // double deg = gps.course.deg();
    int satls = gps->numSV;
    int fixed = gps->fixed;
    double hAcc = gps->hAcc;
    double sAcc = gps->sAcc;

    // int latlng = 0;

    char buf[256];
    snprintf(buf, sizeof(buf),
             "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%lu,%d,%.2f,%.2f", year,
             month, day, hour, minute, second, csecond,
             lat, lng, 0.0, 0.0, 0.0, 0.0, kmph, 0.0f, satls, 1, 1, millis(), fixed, hAcc, sAcc);

    spdlog::info("{}", buf);
}

void Race::gpsConsumerTask(void *param)
{
    Race *race = static_cast<Race *>(param);
    BSP &bsp = BSP::getInstance();
    QueueHandle_t queueHandle = bsp.gps.get_gps_queue_handle();
    GPS_t gps_data;

    while (true)
    {
        if (xQueueReceive(queueHandle, &gps_data, portMAX_DELAY) == pdPASS)
        {
            // double lat = gps_data.latitude;
            // double lng = gps_data.longitude;
            // double altitude = gps_data.altitude;
            // double kmph = gps_data.speed;
            // int year = gps_data.date.year;
            // int month = gps_data.date.month;
            // int day = gps_data.date.day;
            // int hour = gps_data.date.hour;
            // int minute = gps_data.date.min;
            // int second = gps_data.date.sec;
            // int csecond = gps_data.date.msec;
            // // double deg = gps.course.deg();
            // int satls = gps_data.numSV;
            // int fixed = gps_data.fixed;
            // double hAcc = gps_data.hAcc;
            // double sAcc = gps_data.sAcc;

            // // int latlng = 0;

            // char buf[256];
            // snprintf(buf, sizeof(buf),
            //          "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%lu,%d,%.2f,%.2f", year,
            //          month, day, hour, minute, second, csecond,
            //          lat, lng, 0.0, 0.0, 0.0, 0.0, kmph, 0.0f, satls, 1, 1, millis(), fixed, hAcc, sAcc);

            // spdlog::info("{}", buf);
            race->recordGps(&gps_data);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Race::startGpsConsumerTask()
{
    BSP &bsp = BSP::getInstance();
    bsp.gps.setRunning(true);
    xTaskCreate(gpsConsumerTask, "GpsConsumerTask", 10240, this, 0, &gpsConsumerTaskHandle);
}