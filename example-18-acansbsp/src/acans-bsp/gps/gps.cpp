#include "gps.h"
// #include "../../app/app.h"

// extern APP app;

namespace HAL
{

    BspGPS::BspGPS()
    {

        running = false;
    }

    void BspGPS::init()
    {

        spdlog::info("BspGPS::init()");

#if BSP_MODULE_GPS_NOFLASH
        delay(50);
        spdlog::info("BspGPS set");
        gpsSerial.begin(gpsCFGBUAD, SERIAL_8N1, RXD1, TXD1);
        ublox_init();
        spdlog::info("BspGPS::set ok");
#endif

        delay(300);
        gpsSerial.begin(gpsBUAD, SERIAL_8N1, RXD1, TXD1);
#if BSP_MODULE_SD
        // dataFile = SD.open(DataFileName, FILE_READ);
        // if (dataFile)
        // {

        //     while (dataFile.available())
        //     {
        //         Serial.write(dataFile.read());
        //     }
        //     dataFile.close();
        // }
        // else
        // {
        //     Serial.println("Error opening file for reading");
        // }

        // dataFile = SD.open(DataFileName, FILE_WRITE);
        // if (dataFile)
        // {
        //     spdlog::info("GPS connect SD card ok");
        // }
        // else
        // {
        //     spdlog::info("GPS connect SD card error");
        // }

        init_gps_queue();

#endif
    }

    void BspGPS::init_gps_queue()
    {
        spdlog::info("xQueue_gps_handle  init ...");
        xQueue_gps_handle = xQueueCreate(100, sizeof(GPS_t));
        if (xQueue_gps_handle == NULL)
        {
            spdlog::info("xQueue_gps_handle init failed...");
        }

        spdlog::info("xQueue_gps_handle  init ok.");

        // xTaskCreate(
        //     taskWritefile,   /*任务函数*/
        //     "taskWritefile", /*带任务名称的字符串*/
        //     10000,           /*堆栈大小，单位为字节*/
        //     NULL,            /*作为任务输入传递的参数*/
        //     1,               /*任务的优先级*/
        //     0);              /*任务句柄*/
        // spdlog::info("Task taskWritefile start");
    }

    QueueHandle_t BspGPS::get_gps_queue_handle()
    {
        return xQueue_gps_handle;
    }

    BspGPS::~BspGPS()
    {

        if (xQueue_gps_handle != nullptr)
        {
            vQueueDelete(xQueue_gps_handle);
        }
    }
    //     void BspGPS::logfile(const char *data)
    //     {
    // #if BSP_MODULE_SD
    //         if (dataFile)
    //         {
    //             dataFile.println(data);
    //             dataFile.flush();
    //         }
    // #endif
    //     }

    void BspGPS::loop()
    {

        // if ((millis() / 1000) % 2)
        // {
        //     spdlog::info("BspGPS::loop()");
        // }
        while (gpsSerial.available() > 0)
        {
            uint8_t inByte = gpsSerial.read();
            // Serial.print(inByte);
            ublox_decode(inByte);
        }
    }

    void BspGPS::recordGps(GPS_t *gps_data)
    {
        // spdlog::info(
        //     "{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}/{}/{} {}:{}:{}.{}",
        //     gps_data->time, gps_data->latitude, gps_data->longitude, gps_data->altitude, gps_data->velN, gps_data->velE,
        //     gps_data->velD, gps_data->speed, gps_data->heading, gps_data->hAcc, gps_data->vAcc, gps_data->sAcc,
        //     gps_data->cAcc, static_cast<int>(gps_data->fixed), static_cast<int>(gps_data->numSV),
        //     gps_data->date.year, gps_data->date.month, gps_data->date.day, gps_data->date.hour,
        //     gps_data->date.min, gps_data->date.sec, gps_data->date.msec);

        double lat = gps_data->latitude;
        double lng = gps_data->longitude;
        double altitude = gps_data->altitude;
        double kmph = gps_data->speed;
        int year = gps_data->date.year;
        int month = gps_data->date.month;
        int day = gps_data->date.day;
        int hour = gps_data->date.hour;
        int minute = gps_data->date.min;
        int second = gps_data->date.sec;
        int csecond = gps_data->date.msec;
        // double deg = gps.course.deg();
        int satls = gps_data->numSV;
        // int latlng = 0;

        char buf[256];
        snprintf(buf, sizeof(buf),
                 "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%lu", year,
                 month, day, hour, minute, second, csecond,
                 lat, lng, 0.0, 0.0, 0.0, 0.0, kmph, 0.0f, satls, 1, 1, millis());

        spdlog::info("{}", buf);

        // #if BSP_MODULE_SD
        //         logfile(buf);

        // #endif
    }

    void BspGPS::ublox_send_message(uint8_t *data, uint8_t length)
    {
        // 根据平台实现串口发送函数
        for (uint8_t i = 0; i < length; i++)
        {
            gpsSerial.write(data[i]);
        }
    }

    // ublox数据解析函数，在串口中断或串口数据处理中调用该函数，传入串口数据
    void BspGPS::ublox_decode(uint8_t data)
    {
        ublox_parse_char(data);
    }

    // 毫秒级延时函数，用户根据平台来实现
    void BspGPS::delay_ms(uint32_t ms)
    {
        delay(ms);
    }

    /**********************************************************************************************************
     *函 数 名: ublox_init
     *功能说明: ublox配置初始化
     *形    参: 无
     *返 回 值: 无
     **********************************************************************************************************/
    void BspGPS::ublox_init(void)
    {
        // 根据平台实现串口初始化 MG90X默认波特率为115200
        // usart_init(GPS_DEFAULT_BAUDRATE);

        // GPS模块上电300ms后才能进行配置
        spdlog::info("ublox_init");
        delay_ms(300);

        // ublox UART端口配置
        // 波特率115200
        // 输入协议NMEA+UBX+RTCM
        // 输出协议UBX

        // 配置输出速率
        // 输入参数为时间间隔，单位ms
        // 旧版固件的M10，如果设置输出频率大于等于10Hz，会有16颗搜星数量的限制
        // 微空科技的M10模组均使用最新版固件，无此问题，可放心设置输出频率为10Hz
        spdlog::info("ublox_cfg_rate");
        ublox_cfg_rate(100); // 100ms输出间隔：10Hz
        delay_ms(5);

        // 导航参数配置
        // AIRBORNE_2G模式(无人机推荐)
        spdlog::info("ublox_cfg_nav5");
        ublox_cfg_nav5();
        delay_ms(5);

        // 使能PVT语句输出
        // PVT语句里包含了经纬度、时间、速度等等所有导航需要用的数据，所以只输出并解析这一帧就足够了
        ublox_enable_message(UBLOX_NAV_CLASS, UBLOX_NAV_PVT, 1);
        delay_ms(5);
        spdlog::info("ublox_cfg_port");
        ublox_cfg_port(gpsBUAD);
        delay_ms(5);
        spdlog::info("ublox_cfg ok");
    }

    void BspGPS::ublox_payload_decode(UBLOX_RAW_t raw_data)
    {
        if (raw_data.class1 == UBLOX_NAV_CLASS)
        {
            switch (raw_data.id)
            {
            case UBLOX_NAV_PVT:
                gps_data.time = raw_data.payload.pvt.iTOW;
                gps_data.latitude = (double)raw_data.payload.pvt.lat * (double)1e-7;  // 单位:deg
                gps_data.longitude = (double)raw_data.payload.pvt.lon * (double)1e-7; // 单位:deg
                gps_data.altitude = (float)raw_data.payload.pvt.hMSL * 0.001f;        // 单位:m
                gps_data.hAcc = (float)raw_data.payload.pvt.hAcc * 0.001f;            // 单位:m
                gps_data.vAcc = (float)raw_data.payload.pvt.vAcc * 0.001f;            // 单位:m
                gps_data.velN = (float)raw_data.payload.pvt.velN * 0.001f;            // 单位:m/s
                gps_data.velE = (float)raw_data.payload.pvt.velE * 0.001f;            // 单位:m/s
                gps_data.velD = (float)raw_data.payload.pvt.velD * 0.001f;            // 单位:m/s
                gps_data.speed = (float)raw_data.payload.pvt.gSpeed * 0.001f;         // 单位:m/s
                gps_data.heading = (float)raw_data.payload.pvt.heading * 1e-5f;       // 单位:deg
                gps_data.sAcc = (float)raw_data.payload.pvt.sAcc * 0.001f;            // 单位:m/s
                gps_data.cAcc = (float)raw_data.payload.pvt.cAcc * 1e-5f;             // 单位:deg
                gps_data.numSV = raw_data.payload.pvt.numSV;
                gps_data.fixed = raw_data.payload.pvt.gpsFix;
                gps_data.date.year = raw_data.payload.pvt.year;
                gps_data.date.month = raw_data.payload.pvt.month;
                gps_data.date.day = raw_data.payload.pvt.day;
                gps_data.date.hour = raw_data.payload.pvt.hour;
                gps_data.date.min = raw_data.payload.pvt.min;
                gps_data.date.sec = raw_data.payload.pvt.sec;
                gps_data.date.msec = (gps_data.time % 1000);
                // recordGps(&gps_data);
                if (running)
                {
                    xQueueSend(xQueue_gps_handle, &gps_data, portMAX_DELAY);
                }

                break;

            default:
                break;
            }
        }
        else if (raw_data.class1 == UBLOX_ACK_CLASS)
        {
            if (raw_data.id == UBLOX_ACK_ACK)
            {
            }
        }
    }

    /**********************************************************************************************************
     *函 数 名: ublox_parse_char
     *功能说明: ublox协议字符串解析
     *形    参: 输入数据
     *返 回 值: 无
     **********************************************************************************************************/
    void BspGPS::ublox_parse_char(uint8_t data)
    {
        static UBLOX_RAW_t ublox_raw;

        switch (ublox_raw.state)
        {
        /*帧头1*/
        case UBLOX_WAIT_SYNC1:
            if (data == UBLOX_SYNC1)
                ublox_raw.state = UBLOX_WAIT_SYNC2;
            break;

        /*帧头2*/
        case UBLOX_WAIT_SYNC2:
            if (data == UBLOX_SYNC2)
                ublox_raw.state = UBLOX_WAIT_CLASS;
            else
                ublox_raw.state = UBLOX_WAIT_SYNC1;
            break;

        /*消息类型*/
        case UBLOX_WAIT_CLASS:
            ublox_raw.class1 = data;
            // 校验值初始化
            ublox_raw.ubloxRxCK_A = 0;
            ublox_raw.ubloxRxCK_B = 0;
            // 校验值计算
            ublox_raw.ubloxRxCK_A += data;
            ublox_raw.ubloxRxCK_B += ublox_raw.ubloxRxCK_A;
            ublox_raw.state = UBLOX_WAIT_ID;
            break;

        /*消息ID*/
        case UBLOX_WAIT_ID:
            ublox_raw.id = data;
            // 校验值计算
            ublox_raw.ubloxRxCK_A += data;
            ublox_raw.ubloxRxCK_B += ublox_raw.ubloxRxCK_A;
            ublox_raw.state = UBLOX_WAIT_LEN1;
            break;

        /*消息长度低8位*/
        case UBLOX_WAIT_LEN1:
            ublox_raw.length = data;
            // 校验值计算
            ublox_raw.ubloxRxCK_A += data;
            ublox_raw.ubloxRxCK_B += ublox_raw.ubloxRxCK_A;
            ublox_raw.state = UBLOX_WAIT_LEN2;
            break;

        /*消息长度高8位*/
        case UBLOX_WAIT_LEN2:
            ublox_raw.length += (data << 8);
            // 校验值计算
            ublox_raw.ubloxRxCK_A += data;
            ublox_raw.ubloxRxCK_B += ublox_raw.ubloxRxCK_A;
            if (ublox_raw.length >= (UBLOX_MAX_PAYLOAD - 1))
            {
                ublox_raw.length = 0;
                ublox_raw.state = UBLOX_WAIT_SYNC1;
            }
            else if (ublox_raw.length > 0)
            {
                ublox_raw.count = 0;
                ublox_raw.state = UBLOX_PAYLOAD;
            }
            else
            {
                ublox_raw.state = UBLOX_CHECK1;
            }
            break;

        /*消息负载*/
        case UBLOX_PAYLOAD:
            *((char *)(&ublox_raw.payload) + ublox_raw.count) = data;
            if (++ublox_raw.count == ublox_raw.length)
            {
                ublox_raw.state = UBLOX_CHECK1;
            }
            // 校验值计算
            ublox_raw.ubloxRxCK_A += data;
            ublox_raw.ubloxRxCK_B += ublox_raw.ubloxRxCK_A;
            break;

        /*CKA校验位对比*/
        case UBLOX_CHECK1:
            if (data == ublox_raw.ubloxRxCK_A)
            {
                ublox_raw.state = UBLOX_CHECK2;
            }
            else
            {
                ublox_raw.state = UBLOX_WAIT_SYNC1;
                ublox_raw.checksumErrors++;
            }
            break;

        /*CKB校验位对比*/
        case UBLOX_CHECK2:
            ublox_raw.state = UBLOX_WAIT_SYNC1;
            if (data == ublox_raw.ubloxRxCK_B)
            {
                // 接收完毕，解析数据负载
                ublox_payload_decode(ublox_raw);
            }
            else
            {
                ublox_raw.checksumErrors++;
            }
            break;

        default:
            break;
        }
    }

    /**********************************************************************************************************
     *函 数 名: ublox_enable_message
     *功能说明: 使能ublox消息输出
     *形    参: 消息类型 消息id 输出速率（0表示不输出，1表示一个周期输出一次（最快），数值越大输出速率越低）
     *返 回 值: 无
     **********************************************************************************************************/
    void BspGPS::ublox_enable_message(uint8_t class1, uint8_t id, uint8_t rate)
    {
        uint8_t data[11];
        uint8_t msg_len = 11;
        uint8_t ck_a = 0, ck_b = 0;

        data[0] = UBLOX_SYNC1;     // 帧头1
        data[1] = UBLOX_SYNC2;     // 帧头2
        data[2] = UBLOX_CFG_CLASS; // 消息类型
        data[3] = UBLOX_CFG_MSG;   // 消息id
        data[4] = 0x03;            // 消息负载长度低8位
        data[5] = 0x00;            // 消息负载长度高8位

        data[6] = class1; // 设置消息类型
        data[7] = id;     // 设置消息id
        data[8] = rate;   // 设置消息输出速率

        for (uint8_t i = 2; i < msg_len - 2; i++)
        {
            ck_a += data[i];
            ck_b += ck_a;
        }

        data[9] = ck_a;
        data[10] = ck_b;

        ublox_send_message(data, msg_len);
    }

    /**********************************************************************************************************
     *函 数 名: ublox_cfg_rate
     *功能说明: 设置ublox输出速率
     *形    参: 输出速率（单位：ms）
     *返 回 值: 无
     **********************************************************************************************************/
    void BspGPS::ublox_cfg_rate(uint16_t rate)
    {
        uint8_t data[14];
        uint8_t msg_len = 14;
        uint8_t ck_a = 0, ck_b = 0;

        data[0] = UBLOX_SYNC1;     // 帧头1
        data[1] = UBLOX_SYNC2;     // 帧头2
        data[2] = UBLOX_CFG_CLASS; // 消息类型
        data[3] = UBLOX_CFG_RTATE; // 消息id
        data[4] = 0x06;            // 消息负载长度低8位
        data[5] = 0x00;            // 消息负载长度高8位

        data[6] = (uint8_t)rate;        // 输出速率低八位
        data[7] = (uint8_t)(rate >> 8); // 输出速率高八位
        data[8] = 0x01;                 // 导航周期低八位
        data[9] = 0x00;                 // 导航周期高八位
        data[10] = 0x01;                // timeRef 0:UTC, 1:GPS time
        data[11] = 0x00;                // 高八位

        for (uint8_t i = 2; i < msg_len - 2; i++)
        {
            ck_a += data[i];
            ck_b += ck_a;
        }

        data[12] = ck_a;
        data[13] = ck_b;

        ublox_send_message(data, msg_len);
    }

    /**********************************************************************************************************
     *函 数 名: ublox_cfg_port
     *功能说明: 设置ublox输出配置
     *形    参: 波特率
     *返 回 值: 无
     **********************************************************************************************************/
    void BspGPS::ublox_cfg_port(uint32_t baudrate)
    {
        uint8_t data[28];
        uint8_t msg_len = 28;
        uint8_t ck_a = 0, ck_b = 0;

        data[0] = UBLOX_SYNC1;     // 帧头1
        data[1] = UBLOX_SYNC2;     // 帧头2
        data[2] = UBLOX_CFG_CLASS; // 消息类型
        data[3] = UBLOX_CFG_PRT;   // 消息id
        data[4] = 0x14;            // 消息负载长度低8位
        data[5] = 0x00;            // 消息负载长度高8位

        data[6] = 0x01;                       // 端口号 1:uart1
        data[7] = 0x00;                       // 预留
        data[8] = 0x00;                       // TX Ready高八位
        data[9] = 0x00;                       // TX Ready低八位
        data[10] = (uint8_t)(0x08D0);         // 串口配置
        data[11] = (uint8_t)(0x08D0 >> 8);    // 8位
        data[12] = (uint8_t)(0x08D0 >> 16);   // 1个停止位
        data[13] = (uint8_t)(0x08D0 >> 24);   // 无校验位
        data[14] = (uint8_t)(baudrate);       // 串口波特率
        data[15] = (uint8_t)(baudrate >> 8);  //
        data[16] = (uint8_t)(baudrate >> 16); //
        data[17] = (uint8_t)(baudrate >> 24); //
        data[18] = 0x07;                      // 输入协议配置
        data[19] = 0x00;                      // ubx+nmea+rtcm
        data[20] = 0x01;                      // 输出协议配置
        data[21] = 0x00;                      // ubx
        data[22] = 0x00;                      // flags低八位
        data[23] = 0x00;                      // flags高八位
        data[24] = 0x00;                      // reserved
        data[25] = 0x00;                      // reserved

        for (uint8_t i = 2; i < msg_len - 2; i++)
        {
            ck_a += data[i];
            ck_b += ck_a;
        }

        data[26] = ck_a;
        data[27] = ck_b;

        ublox_send_message(data, msg_len);
    }

    /**********************************************************************************************************
     *函 数 名: ublox_cfg_nav5
     *功能说明: 导航相关参数配置
     *形    参: 无
     *返 回 值: 无
     **********************************************************************************************************/
    void BspGPS::ublox_cfg_nav5(void)
    {
        uint8_t data[44];
        uint8_t msg_len = 44;
        uint8_t ck_a = 0, ck_b = 0;

        data[0] = UBLOX_SYNC1;     // 帧头1
        data[1] = UBLOX_SYNC2;     // 帧头2
        data[2] = UBLOX_CFG_CLASS; // 消息类型
        data[3] = UBLOX_CFG_NAV5;  // 消息id
        data[4] = 0x24;            // 消息负载长度低8位
        data[5] = 0x00;            // 消息负载长度高8位

        data[6] = 0xFF;             // Mask
        data[7] = 0xFF;             // Mask
        data[8] = AIRBORNE_2G;      // 动态平台模型，无人机推荐使用AIRBORNE_2G模式
        data[9] = 0x03;             // position fixing mode 3=auto 2D/3D
        data[10] = (uint8_t)(0x00); // Fixed altitude(mean sea level) for 2D fix mode
        data[11] = (uint8_t)(0x00 >> 8);
        data[12] = (uint8_t)(0x00 >> 16);
        data[13] = (uint8_t)(0x00 >> 24);
        data[14] = (uint8_t)(10000); // Fixed altitude variance for 2D mode
        data[15] = (uint8_t)(10000 >> 8);
        data[16] = (uint8_t)(10000 >> 16);
        data[17] = (uint8_t)(10000 >> 24);
        data[18] = 0x0A;           // 使用卫星的最低角度：10°
        data[19] = 0x00;           // drLimit
        data[20] = (uint8_t)(250); // pDop
        data[21] = (uint8_t)(250 >> 8);
        data[22] = (uint8_t)(250); // tDop
        data[23] = (uint8_t)(250 >> 8);
        data[24] = (uint8_t)(100); // pAcc Mask
        data[25] = (uint8_t)(100 >> 8);
        data[26] = (uint8_t)(350); // tAcc Mask
        data[27] = (uint8_t)(350 >> 8);
        data[28] = 0;  // Static hold threshold
        data[29] = 60; // DGNSS timeout
        data[30] = 0;  // cn0ThreshNumSVs
        data[31] = 25; // 参与导航的卫星所需最低C/N0
        data[32] = 0;  // pAccAdr
        data[33] = 0;
        data[34] = 0; // staticHoldMaxDist
        data[35] = 0;
        data[36] = 0; // utcStandard
        data[37] = 0; // reserved
        data[38] = 0;
        data[39] = 0;
        data[40] = 0;
        data[41] = 0;

        for (uint8_t i = 2; i < msg_len - 2; i++)
        {
            ck_a += data[i];
            ck_b += ck_a;
        }

        data[42] = ck_a;
        data[43] = ck_b;

        ublox_send_message(data, msg_len);
    }
};
