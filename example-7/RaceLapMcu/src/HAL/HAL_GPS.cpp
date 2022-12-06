
#include "HAL.h"
#include "../gps_helper.hpp"

TinyGPSPlus gps;
File dataFile;
bool isSetTime = false;
int preRecordCd = 3;
int recordtoLoopCd = 15;
char buffer[150];
double KMPH = 0; // current speed
double RecordKmph = 3;
char DataFileDir[12] = "/XLAPDATA/";
float gforce = 0.0f;
float gforce_last = 0.0f;

void recordGps()
{

    if ((race.getStatus().status == d_gps_searching) && ((int)(millis() - race.getStatus().timer) > 5000 && gps.charsProcessed() < 10))
    {
        ErrInfo = "No GPS data received, please reboot";
#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "No GPS data received");
        logger.LogInfo(logbuff);
#endif
    }

    // if (gps.location.isUpdated())
    if (gps.location.isValid() && gps.satellites.value() > 3)
    {
        if (race.getStatus().status == d_gps_searching && gps.date.isValid())
        {
            setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
            adjustTime(8 * SECS_PER_HOUR);
            isSetTime = true;
            race.setStatus(d_Looping);
        }
        race.lastGpsUpdateTimer = millis();
        double lat = gps.location.lat();
        double lng = gps.location.lng();
        double altitude = gps.altitude.meters();
        KMPH = gps.speed.kmph();
        int year = gps.date.year();
        int month = gps.date.month();
        int day = gps.date.day();
        int hour = gps.time.hour();
        int minute = gps.time.minute();
        int second = gps.time.second();
        int csecond = gps.time.centisecond() * 10;
        double deg = gps.course.deg();
        int satls = gps.satellites.value();

        if (race.last_gps.location.lat() == lat && race.last_gps.location.lng() == lng)
        {
            return;
        }

        race.computerSession(&gps);

        gforce = sqrt(gravity.x * gravity.x + gravity.y * gravity.y + gravity.z * gravity.z);

        snprintf(buffer, sizeof(buffer),
                 "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%lu", year,
                 month, day, hour, minute, second, csecond,
                 lat, lng, (ypr[1] * 180 / M_PI), 90.0 - (ypr[1] * 180 / M_PI), gforce, gforce - gforce_last, KMPH, 0, satls, 1, 1, millis());

        gforce_last = gforce;

        // snprintf(buffer, sizeof(buffer),
        //          "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%lu,%02d",
        //          year,
        //          month, day, hour, minute, second, csecond, lng, lat, altitude, KMPH, deg, gravity.y, (ypr[1] * 180 / M_PI), millis(), satls);

#if defined(DEBUG)
        // Serial.println(buffer);
#endif

        if (B_SDCARDOK)
        {
            if ((race.getStatus().status == d_Recording || race.getStatus().status == d_RecordToStop))
            {

                if (!dataFile)
                {
                    if (strcmp(DataFileName, "") == 0)
                    {
                        sprintf(DataFileName, "%s%04d%02d%02d%02d%02d%02d.xld", DataFileDir, year, month, day, hour, minute, second);
                    }
#if defined(DEBUG)
                    snprintf(logbuff, sizeof(logbuff), "[%s]new DataFileName recording %s", formatTime(millis()), DataFileName);
                    logger.LogInfo(logbuff);
#endif
                    dataFile = SD.open(DataFileName, FILE_WRITE);

                    dataFile.println(race.getHeader(year, month, day, hour, minute, second));
                }
                if (dataFile)
                {
                    dataFile.println(buffer);
                    dataFile.flush();
                }
            }
        }
    }

    switch (race.getStatus().status)
    {
    case d_Looping:

        if (KMPH > RecordKmph)
        {
            race.setStatus(d_preRecord);
#if defined(DEBUG)
            snprintf(logbuff, sizeof(logbuff), "[%s]d_Looping to d_preRecord kmph=%.2f", formatTime(millis()), KMPH);
            logger.LogInfo(logbuff);
#endif
        }

        break;
    case d_preRecord:
        if (KMPH < RecordKmph)
        {
            race.setStatus(d_Looping);
#if defined(DEBUG)
            snprintf(logbuff, sizeof(logbuff), "[%s]d_preRecord to d_Looping kmph=%.2f RecordKmph=%.2f", formatTime(millis()), KMPH, RecordKmph);
            logger.LogInfo(logbuff);
#endif
            return;
        }
        if (int((millis() - race.getStatus().timer) / 1000) > preRecordCd)
        {
            race.resetSession();
            race.setStatus(d_Recording);
#if defined(DEBUG)
            snprintf(logbuff, sizeof(logbuff), "[%s]d_preRecord to d_Recording >preRecordCd %d kmph %.2f", formatTime(millis()), preRecordCd, KMPH);
            logger.LogInfo(logbuff);
#endif
        }

        break;
    case d_Recording:
        if (int((millis() - race.lastGpsUpdateTimer) / 1000) > 5)
        {
#if defined(DEBUG)
            snprintf(logbuff, sizeof(logbuff), "[%s]GPS no signal %d", formatTime(millis()), int((millis() - race.lastGpsUpdateTimer) / 1000));
            logger.LogInfo(logbuff);
#endif
            KMPH = 0;
        }
        if (KMPH < RecordKmph)
        {
            race.setStatus(d_RecordToStop);
#if defined(DEBUG)
            snprintf(logbuff, sizeof(logbuff), "[%s]d_Recording to d_RecordToStop kmph=%.2f <RecordKmph=%.2f", formatTime(millis()), KMPH, RecordKmph);
            logger.LogInfo(logbuff);
#endif
        }

        break;

    case d_RecordToStop:
        if (KMPH > RecordKmph)
        {
            race.setStatus(d_Recording);
#if defined(DEBUG)
            snprintf(logbuff, sizeof(logbuff), "[%s]d_RecordToStop to d_Recording kmph=%.2f > RecordKmph=%.2f", formatTime(millis()), KMPH, RecordKmph);
            logger.LogInfo(logbuff);
#endif
            return;
        }
        if (int((millis() - race.getStatus().timer) / 1000) > recordtoLoopCd)
        {
            if (dataFile)
            {
#if defined(DEBUG)
                snprintf(logbuff, sizeof(logbuff), "[%s]close DataFileName %s", formatTime(millis()), DataFileName);
                logger.LogInfo(logbuff);
#endif
                dataFile.close();
                strcpy(DataFileName, "");
            }
            race.setStatus(d_Stop);
#if defined(DEBUG)
            snprintf(logbuff, sizeof(logbuff), "[%s]d_RecordToStop to d_Stop", formatTime(millis()));
            logger.LogInfo(logbuff);
#endif
        }
        break;

    case d_Stop:
        // race.computerLapinfo(&gps);

        race.setStatus(d_Looping);
#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "[%s]d_Stop to d_Looping", formatTime(millis()));
        logger.LogInfo(logbuff);
#endif
        break;

    default:
        break;
    }
}

void HAL::GPS_Init()
{
    logger.LogInfo("init GPS");

    gpsSerial.begin(BUAD, SERIAL_8N1, RXD1, TXD1);
    smartDelay(1000);

    if (gps.charsProcessed() < 10)
    {
        ErrInfo += "GPS init failed.\n";

        logger.LogInfo("GPS init failed. No GPS data received: check wiring");
    }
    logger.LogInfo("init GPS ok");
}

void HAL::GPS_Update()
{
    while (gpsSerial.available() > 0)
    {
        char inByte = gpsSerial.read();
        // #if defined(DEBUG)
        //     Serial.print(inByte);
        //     // snprintf(logbuff, sizeof(logbuff), "satellites %d", (int)gps.satellites.value());
        //     gpsFile.print(inByte);
        //     if ((int)(millis() - gpsFileTimer) > 30000)
        //     {
        //       gpsFile.flush();
        //       gpsFileTimer = millis();
        //     }
        // #endif
        // Serial.print(inByte);
        if (gps.encode(inByte))
        {
            recordGps();
            // printData();
        }
    }
}
