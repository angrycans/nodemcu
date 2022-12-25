
#include "HAL.h"

// TinyGPSPlus gps;

File dataFile;
bool isSetTime = false;
int preRecordCd = 3;
int recordtoLoopCd = 15;
char buffer[150];
double KMPH = 0; // current speed
double RecordKmph = 0;
char DataFileDir[12] = "/XLAPDATA/";
float gforce = 0.0f;
float gforce_last = 0.0f;

void recordGps()
{

#if defined(DEBUG)
    Serial.print("fixed :");
    Serial.print(gps_data.fixed);
    Serial.print("numSV :");
    Serial.print(gps_data.numSV);
    Serial.print("time :");
    Serial.println(gps_data.time);
#endif

    //     if ((race.getStatus().status == d_gps_searching) && ((int)(millis() - race.getStatus().timer) > 5000 && gps.charsProcessed() < 10))
    //     {
    //         ErrInfo = "No GPS data received, please reboot";
    // #if defined(DEBUG)
    //         snprintf(logbuff, sizeof(logbuff), "No GPS data received");
    //         logger.LogInfo(logbuff);
    // #endif
    //     }

    // if (gps.location.isUpdated())
    if (gps_data.numSV > 2)
    {
        if (race.getStatus().status == d_gps_searching && gps_data.numSV > 2)
        {
            setTime(gps_time.hour, gps_time.min, gps_time.sec, gps_time.day, gps_time.month, gps_time.year);
            adjustTime(8 * SECS_PER_HOUR);
            isSetTime = true;
            race.setStatus(d_Looping);
        }
        race.lastGpsUpdateTimer = millis();
        double lat = gps_data.latitude;
        double lng = gps_data.longitude;
        double altitude = gps_data.altitude;
        KMPH = gps_data.speed;
        int year = gps_time.year;
        int month = gps_time.month;
        int day = gps_time.day;
        int hour = gps_time.hour;
        int minute = gps_time.min;
        int second = gps_time.sec;
        int csecond = gps_data.time;
        // double deg = gps.course.deg();
        int satls = gps_data.numSV;
        int latlng = 0;

        // if (race.last_gps.location.lat() == lat && race.last_gps.location.lng() == lng)
        // if (race.last_gps.date.year() == gps.date.year() && race.last_gps.date.month() == gps.date.month() && race.last_gps.date.day() == gps.date.day() && race.last_gps.time.hour() == gps.time.hour() && race.last_gps.time.minute() == gps.time.minute() && race.last_gps.time.second() == gps.time.second() && race.last_gps.time.centisecond() == gps.time.centisecond())
        // {
        //     // #if defined(DEBUG)
        //     //             logger.LogInfo(buffer);
        //     // #endif
        //     return;
        // }

        if (gps_data_last.latitude == lat && gps_data_last.longitude == lng)
        {
            latlng = 1;
        }

        race.computerSession(&gps_data);

        gforce = sqrt(gravity.x * gravity.x + gravity.y * gravity.y + gravity.z * gravity.z);

        snprintf(buffer, sizeof(buffer),
                 "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%lu,%d", year,
                 month, day, hour, minute, second, csecond,
                 lat, lng, (ypr[1] * 180 / M_PI), 90.0 - (ypr[1] * 180 / M_PI), gforce, gforce - gforce_last, KMPH, 0.0f, satls, 1, 1, millis(), latlng);

        gforce_last = gforce;

        // snprintf(buffer, sizeof(buffer),
        //          "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%lu,%02d",
        //          year,
        //          month, day, hour, minute, second, csecond, lng, lat, altitude, KMPH, deg, gravity.y, (ypr[1] * 180 / M_PI), millis(), satls);

#if defined(DEBUG)
        Serial.println(buffer);
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
    ublox_init();
    logger.LogInfo("init GPS ok");
}

void HAL::GPS_Update()
{
    while (gpsSerial.available() > 0)
    {
        uint8_t inByte = gpsSerial.read();
        ublox_decode(inByte);
        recordGps();
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
        // if (gps.encode(inByte))
        // {
        //     recordGps();
        //     // printData();
        // }
    }
}
