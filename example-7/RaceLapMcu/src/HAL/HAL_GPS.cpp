
#include "HAL.h"

GPS_t gps_data_2;
GPS_t gps_data_last;

static void ublox_parse_char(uint8_t data);
static void ublox_enable_message(uint8_t class1, uint8_t id, uint8_t rate);
static void ublox_cfg_rate(uint16_t rate);
static void ublox_cfg_port(uint32_t baudrate);
static void ublox_cfg_nav5(void);

File dataFile;
bool isSetTime = false;
int preRecordCd = 3;
int recordtoLoopCd = 15;
char buffer[100];
double KMPH = 0; // current speed
double RecordKmph = 3;
char DataFileDir[12] = "/XLAPDATA/";
float gforce = 0.0f;
float gforce_last = 0.0f;

void recordGps(GPS_t gps_data);

QueueHandle_t xQueue_gps_handle;
void taskWritefile(void *parameter);

void init_gps_queue()
{
    xQueue_gps_handle = xQueueCreate(100, sizeof(GPS_t));
    if (xQueue_gps_handle == NULL)
    {
        logger.LogInfo("xQueue_gps_handle init failed...");
    }

    logger.LogInfo("xQueue_gps_handle  init ok.");

    xTaskCreate(
        taskWritefile,   /*任务函数*/
        "taskWritefile", /*带任务名称的字符串*/
        10000,           /*堆栈大小，单位为字节*/
        NULL,            /*作为任务输入传递的参数*/
        1,               /*任务的优先级*/
        0);              /*任务句柄*/
    logger.LogInfo("Task taskWritefile start");
}

void taskWritefile(void *parameter)
{

    logger.LogInfo("taskWritefile running ...");

    GPS_t resv;

    while (1)
    {
        if (xQueueReceive(xQueue_gps_handle, &resv, 80 / portTICK_PERIOD_MS) == pdPASS)
        {
            recordGps(resv);
        }

        taskYIELD();
    }
    logger.LogInfo("taskWritefile quit");
    vTaskDelete(NULL);
}

void recordGps(GPS_t gps_data)
{

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
            setTime(gps_data.date.hour, gps_data.date.min, gps_data.date.sec, gps_data.date.day, gps_data.date.month, gps_data.date.year);
            adjustTime(8 * SECS_PER_HOUR);
            isSetTime = true;
            race.setStatus(d_Looping);
        }
        race.lastGpsUpdateTimer = millis();
        double lat = gps_data.latitude;
        double lng = gps_data.longitude;
        double altitude = gps_data.altitude;
        KMPH = gps_data.speed;
        int year = gps_data.date.year;
        int month = gps_data.date.month;
        int day = gps_data.date.day;
        int hour = gps_data.date.hour;
        int minute = gps_data.date.min;
        int second = gps_data.date.sec;
        int csecond = gps_data.date.msec;
        // double deg = gps.course.deg();
        int satls = gps_data.numSV;
        int latlng = 0;

        if (gps_data.time == gps_data_last.time)
        {
            // #if defined(DEBUG)
            //             logger.LogInfo(buffer);
            // #endif
            // return;
            logger.LogInfo("-----repeat data-----");
        }

        if (gps_data_last.latitude == lat && gps_data_last.longitude == lng)
        {
            latlng = 1;
        }

        // race.computerSession(&gps_data);

        // gforce = sqrt(gravity.x * gravity.x + gravity.y * gravity.y + gravity.z * gravity.z);

        // snprintf(buffer, sizeof(buffer),
        //          "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%lu,%d", year,
        //          month, day, hour, minute, second, csecond,
        //          lat, lng, (ypr[1] * 180 / M_PI), 90.0 - (ypr[1] * 180 / M_PI), gforce, gforce - gforce_last, KMPH, 0.0f, satls, 1, 1, millis(), latlng);

        // gforce_last = gforce;

        snprintf(buffer, sizeof(buffer),
                 "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%lu,%d", year,
                 month, day, hour, minute, second, csecond,
                 lat, lng, 0.0, 0.0, 0.0, 0.0, KMPH, 0.0f, satls, 1, 1, millis(), latlng);

        // snprintf(buffer, sizeof(buffer),
        //          "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%.2f,%.2f,%lu,%02d",
        //          year,
        //          month, day, hour, minute, second, csecond, lng, lat, altitude, KMPH, deg, gravity.y, (ypr[1] * 180 / M_PI), millis(), satls);

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
#if defined(DEBUG)

                    if (gps_data_last.date.msec == 900 && gps_data.date.msec == 0)
                    {
                    }
                    else if (gps_data_last.date.msec + 100 == gps_data.date.msec)
                    {
                    }
                    else
                    {

                        // Serial.println("------------------------");
                        dataFile.println("--------------");
                    }
                    // Serial.println(buffer);
#endif
                    dataFile.println(buffer);
                    dataFile.flush();
                    //  Serial.println(buffer);
                }
            }
        }
    }
    //
    gps_data_last = gps_data;
    //
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

// 串口发送数据，需用户根据平台来实现该函数
void ublox_send_message(uint8_t *data, uint8_t length)
{
    // 根据平台实现串口发送函数
    for (uint8_t i = 0; i < length; i++)
    {
        gpsSerial.write(data[i]);
    }
}

// ublox数据解析函数，在串口中断或串口数据处理中调用该函数，传入串口数据
void ublox_decode(uint8_t data)
{
    ublox_parse_char(data);
}

// 毫秒级延时函数，用户根据平台来实现
void delay_ms(uint32_t ms)
{
    delay(ms);
}

/**********************************************************************************************************
 *函 数 名: ublox_init
 *功能说明: ublox配置初始化
 *形    参: 无
 *返 回 值: 无
 **********************************************************************************************************/
void ublox_init(void)
{
    // 根据平台实现串口初始化 MG90X默认波特率为115200
    // usart_init(GPS_DEFAULT_BAUDRATE);

    // GPS模块上电300ms后才能进行配置
    delay_ms(300);

    // ublox UART端口配置
    // 波特率115200
    // 输入协议NMEA+UBX+RTCM
    // 输出协议UBX
    ublox_cfg_port(BUAD);
    delay_ms(5);

    // 配置输出速率
    // 输入参数为时间间隔，单位ms
    // 旧版固件的M10，如果设置输出频率大于等于10Hz，会有16颗搜星数量的限制
    // 微空科技的M10模组均使用最新版固件，无此问题，可放心设置输出频率为10Hz
    ublox_cfg_rate(100); // 100ms输出间隔：10Hz
    delay_ms(5);

    // 导航参数配置
    // AIRBORNE_2G模式(无人机推荐)
    ublox_cfg_nav5();
    delay_ms(5);

    // 使能PVT语句输出
    // PVT语句里包含了经纬度、时间、速度等等所有导航需要用的数据，所以只输出并解析这一帧就足够了
    ublox_enable_message(UBLOX_NAV_CLASS, UBLOX_NAV_PVT, 1);
}

/**********************************************************************************************************
 *函 数 名: ublox_payload_decode
 *功能说明: ublox数据负载解析
 *形    参: 输入数据
 *返 回 值: 无
 **********************************************************************************************************/
static void ublox_payload_decode(UBLOX_RAW_t raw_data)
{
    if (raw_data.class1 == UBLOX_NAV_CLASS)
    {
        switch (raw_data.id)
        {
        case UBLOX_NAV_PVT:
            gps_data_2.time = raw_data.payload.pvt.iTOW;
            gps_data_2.latitude = (double)raw_data.payload.pvt.lat * (double)1e-7;  // 单位:deg
            gps_data_2.longitude = (double)raw_data.payload.pvt.lon * (double)1e-7; // 单位:deg
            gps_data_2.altitude = (float)raw_data.payload.pvt.hMSL * 0.001f;        // 单位:m
            gps_data_2.hAcc = (float)raw_data.payload.pvt.hAcc * 0.001f;            // 单位:m
            gps_data_2.vAcc = (float)raw_data.payload.pvt.vAcc * 0.001f;            // 单位:m
            gps_data_2.velN = (float)raw_data.payload.pvt.velN * 0.001f;            // 单位:m/s
            gps_data_2.velE = (float)raw_data.payload.pvt.velE * 0.001f;            // 单位:m/s
            gps_data_2.velD = (float)raw_data.payload.pvt.velD * 0.001f;            // 单位:m/s
            gps_data_2.speed = (float)raw_data.payload.pvt.gSpeed * 0.001f;         // 单位:m/s
            gps_data_2.heading = (float)raw_data.payload.pvt.heading * 1e-5f;       // 单位:deg
            gps_data_2.sAcc = (float)raw_data.payload.pvt.sAcc * 0.001f;            // 单位:m/s
            gps_data_2.cAcc = (float)raw_data.payload.pvt.cAcc * 1e-5f;             // 单位:deg
            gps_data_2.numSV = raw_data.payload.pvt.numSV;
            gps_data_2.fixed = raw_data.payload.pvt.gpsFix;
            gps_data_2.date.year = raw_data.payload.pvt.year;
            gps_data_2.date.month = raw_data.payload.pvt.month;
            gps_data_2.date.day = raw_data.payload.pvt.day;
            gps_data_2.date.hour = raw_data.payload.pvt.hour;
            gps_data_2.date.min = raw_data.payload.pvt.min;
            gps_data_2.date.sec = raw_data.payload.pvt.sec;
            gps_data_2.date.msec = (gps_data_2.time % 1000);
            // recordGps();
            xQueueSend(xQueue_gps_handle, &gps_data_2, portMAX_DELAY);

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
static void ublox_parse_char(uint8_t data)
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
static void ublox_enable_message(uint8_t class1, uint8_t id, uint8_t rate)
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
static void ublox_cfg_rate(uint16_t rate)
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
static void ublox_cfg_port(uint32_t baudrate)
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
static void ublox_cfg_nav5(void)
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

void HAL::GPS_Init()
{
    logger.LogInfo("init GPS");
    init_gps_queue();

    gpsSerial.begin(BUAD, SERIAL_8N1, RXD1, TXD1);
    // ublox_init();
    logger.LogInfo("init GPS ok");
}

void HAL::GPS_Update()
{
    while (gpsSerial.available() > 0)
    {
        uint8_t inByte = gpsSerial.read();
        Serial.print(inByte);
        ublox_decode(inByte);

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
