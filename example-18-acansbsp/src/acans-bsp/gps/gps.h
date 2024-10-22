#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>
#include "../bsp-config.h"
#include "ublox_Def.h"
#include <mooncake.h>
#include <FS.h>
#include <SD.h>

namespace HAL
{
    class BspGPS
    {
    private:
        bool running = false;
        // File dataFile;
        // char DataFileName[64] = "/log.txt";

        GPS_t gps_data;
        void ublox_send_message(uint8_t *data, uint8_t length);
        void ublox_decode(uint8_t data);
        void delay_ms(uint32_t ms);
        void ublox_init(void);
        // void logfile(const char *data);
        QueueHandle_t xQueue_gps_handle;

    public:
        BspGPS();

        ~BspGPS();
        void init();

        void loop();

        void init_gps_queue();
        QueueHandle_t get_gps_queue_handle();
        void recordGps(GPS_t *gps_data);

        void ublox_payload_decode(UBLOX_RAW_t raw_data);
        void ublox_parse_char(uint8_t data);
        void ublox_enable_message(uint8_t class1, uint8_t id, uint8_t rate);
        void ublox_cfg_rate(uint16_t rate);
        void ublox_cfg_port(uint32_t baudrate);
        void ublox_cfg_nav5(void);

        void setRunning(bool _r)
        {
            running = _r;
        }

        bool getRunning()
        {
            return running;
        }
    };
}
