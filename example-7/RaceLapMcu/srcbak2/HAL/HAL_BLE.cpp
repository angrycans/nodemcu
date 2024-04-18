
#include "HAL/HAL.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SD.h>

#include "freertos/queue.h"

// #define SERVICE_UUID "6182d488-08a0-4889-bb3d-d90c8e351edd"
// #define CHARACTERISTIC_UUID_RX "fb1e4002-54ae-4a28-9f74-dfccb248601d"
// #define CHARACTERISTIC_UUID_TX "fb1e4003-54ae-4a28-9f74-dfccb248601d"

#define SERVICE_UUID(val) ("6182d488-" val "-4889-bb3d-d90c8e351edd")

static BLECharacteristic *pCharacteristicCMD;
static BLECharacteristic *pCharacteristicTrackInfo;
static BLECharacteristic *pCharacteristicListDir;
static BLECharacteristic *pCharacteristicDownloadFile;

// static BLECharacteristic *pCharacteristicTX;
// static BLECharacteristic *pCharacteristicRX;
static bool deviceConnected = false;
static bool deviceDisConnected = false;

BLEServer *pServer;

static const uint32_t crc_32_tab[] PROGMEM = {/* CRC polynomial 0xedb88320 */
                                              0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
                                              0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
                                              0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
                                              0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
                                              0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
                                              0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
                                              0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
                                              0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
                                              0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
                                              0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
                                              0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
                                              0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
                                              0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
                                              0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
                                              0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
                                              0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
                                              0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
                                              0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
                                              0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
                                              0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
                                              0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
                                              0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
                                              0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
                                              0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
                                              0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
                                              0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
                                              0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
                                              0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
                                              0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
                                              0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
                                              0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
                                              0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
                                              0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
                                              0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
                                              0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
                                              0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
                                              0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
                                              0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
                                              0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
                                              0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
                                              0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
                                              0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
                                              0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

uint32_t oldcrc32 = 0xFFFFFFFF;
uint32_t charcnt = 0;

inline uint32_t updateCRC32(uint8_t ch, uint32_t crc)
{
    uint32_t idx = ((crc) ^ (ch)) & 0xff;
    uint32_t tab_value = pgm_read_dword(crc_32_tab + idx);
    return tab_value ^ ((crc) >> 8);
}
uint32_t crc32(File &file, uint32_t &charcnt)
{
    uint32_t oldcrc32 = 0xFFFFFFFF;
    charcnt = 0;

    while (file.available())
    {
        uint8_t c = file.read();
        // Serial.println(c, HEX);
        charcnt++;
        oldcrc32 = updateCRC32(c, oldcrc32);
    }

    return ~oldcrc32;
}

uint32_t crc32_string(String str)
{

    // charcnt = 0;
    // String testString = "Test String";
    // uint8_t *c = (uint8_t *)str.c_str();
    // int length = str.length();
    // char c[length];
    // str.toCharArray(c, length);

    // Serial.println(length);

    for (int i = 0; i < str.length(); i++)
    {
        // uint8_t c = file.read();
        charcnt++;
        // Serial.println(length);
        // Serial.println((uint8_t)str[i], HEX);
        // Serial.println((uint8_t)str[i]);
        oldcrc32 = updateCRC32((uint8_t)str[i], oldcrc32);
    }

    // Serial.println("\n", HEX);

    return oldcrc32;
}

void test_crc32_string()
{
    oldcrc32 = 0xFFFFFFFF;
    charcnt = 0;
    oldcrc32 = crc32_string("Hello World1!\n");
    oldcrc32 = crc32_string("Hello World2!\n");
    oldcrc32 = crc32_string("Hello World3!\n");
    Serial.print("string CRC32 is 0x");
    Serial.println(~oldcrc32, HEX);

    // Serial.print("----->");
    // String str = "\n";
    // Serial.println((uint8_t)str[0]);

    oldcrc32 = 0xFFFFFFFF;
    charcnt = 0;
}

// QueueHandle_t xQueue_handle;
// static StaticQueue_t xStaticQueue;
// static QueueHandle_t queue_handle; // Send queue handle

QueueHandle_t xQueue_file_handle;
bool isDownloading = false;

void init_queue()
{
    xQueue_file_handle = xQueueCreate(5, sizeof(int));
    if (xQueue_file_handle == NULL)
    {
        logger.LogInfo("Queue initialisation failed...");
    }

    logger.LogInfo("Send queue initialised.");
}

void taskOne(void *parameter)
{

    logger.LogInfo("vtaskOne run");

    File dir = SD.open("/XLAPDATA", "r");
    String tree = "";
    int count = 0;

    while (true)
    {

        File entry = dir.openNextFile("r");

        if (!entry)
        {
            // no more files

            break;
        }

        if (!entry.isDirectory())
        {

            String str = entry.name();

            if (strcmp(DataFileName, str.c_str()) != 0)
            {
                if (str.indexOf("track.json") > -1 || str.indexOf("log.txt") > -1 || str[0] == '.')
                {
                }
                else
                {

                    String tree = DataFileDir + str; // + "_" + entry.size();
                    count++;
                    pCharacteristicListDir->setValue(tree.c_str());
                    pCharacteristicListDir->notify();
                    // xQueueSend(queue_handle, &tree, portMAX_DELAY);
                    // taskYIELD();
                }
            }
        }

        entry.close();
    }

    dir.close();

    tree = "files:";
    tree += count;
    // logger.LogInfo(tree);
    pCharacteristicListDir->setValue(tree.c_str());
    pCharacteristicListDir->notify();

    vTaskDelete(NULL);
}

void taskDelfile(void *parameter)
{

    logger.LogInfo("taskDelfile run");

    char params[36];

    sprintf(params, (char *)parameter);

    // Serial.println((char *)parameter);
    String rst = "delfile:";
    rst.concat(params);
    if (SD.remove(params))
    {
        rst += ":ok";
    }
    else
    {
        rst += ":error";
    }
    logger.LogInfo(rst);
    pCharacteristicCMD->setValue(rst.c_str());
    pCharacteristicCMD->notify();

    vTaskDelete(NULL);
}

void taskTrackInfo(void *parameter)
{

    logger.LogInfo("taskTrackInfo run");

    File file = SD.open("/XLAPDATA/track.json");

    if (!file)
    {
        String rst = "taskTrackInfo:trackinfo_not_found";
        pCharacteristicCMD->setValue(rst.c_str());
        pCharacteristicCMD->notify();
    }
    else
    {
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, file);
        if (!error)
        {

            String output;
            serializeJson(doc, output);
            pCharacteristicCMD->setValue(output.c_str());
            pCharacteristicCMD->notify();
        }
        else
        {
            String rst = "taskTrackInfo:deserializefailed";
            pCharacteristicCMD->setValue(rst.c_str());
            pCharacteristicCMD->notify();
        }
    }

    vTaskDelete(NULL);
}

void taskdDownloadfile(void *parameter)
{

    logger.LogInfo("taskdDownloadfile run");
    isDownloading = true;
    char params[36];
    sprintf(params, (char *)parameter);

    // Serial.println((char *)parameter);
    String rst = "downloadfile:";
    rst.concat(params);

    File myFile = SD.open(params, "r");
    String line;
    oldcrc32 = 0xFFFFFFFF;
    charcnt = 0;
    int size = 512;
    char readbuf[size];
    int readSize = 0;
    int count = 0;
    unsigned long startMillis = millis();
    if (myFile)
    {
        long intReceivedValue;
        count++;
        readSize = myFile.readBytes(readbuf, size);
        for (int i = 0; i < readSize; i++)
        {
            oldcrc32 = updateCRC32(readbuf[i], oldcrc32);
        }
        pCharacteristicDownloadFile->setValue((uint8_t *)readbuf, readSize);
        pCharacteristicDownloadFile->notify();
        while (1)
        {
            if (xQueueReceive(xQueue_file_handle, &intReceivedValue, 2000 / portTICK_PERIOD_MS) == pdPASS)
            {
                // Serial.print("Received = ");
                // Serial.println(intReceivedValue);
                if (!myFile.available())
                {
                    Serial.print("CRC32 is 0x");
                    Serial.println(~oldcrc32, HEX);

                    rst += ":crc32:" + String(~oldcrc32) + ":" + String(millis() - startMillis) + ":" + String(count);
                    pCharacteristicDownloadFile->setValue(rst.c_str());
                    pCharacteristicDownloadFile->notify();
                    break;
                }
                count++;
                readSize = myFile.readBytes(readbuf, size);
                for (int i = 0; i < readSize; i++)
                {
                    oldcrc32 = updateCRC32(readbuf[i], oldcrc32);
                }

                pCharacteristicDownloadFile->setValue((uint8_t *)readbuf, readSize);
                pCharacteristicDownloadFile->notify();
                // startMillis = millis();
            }
            else
            {
                // Serial.println("xQueueReceive break");
                rst.concat(":xQueueReceive_timeout");
                pCharacteristicDownloadFile->setValue(rst.c_str());
                pCharacteristicDownloadFile->notify();
                break;
            }

            taskYIELD();
        }
    }
    else
    {
        rst += ":openfile_error";
        pCharacteristicCMD->setValue(rst.c_str());
        pCharacteristicCMD->notify();
    }

    myFile.close();
    isDownloading = false;

    // rst += ":ok:" + String((millis() - startMillis));
    logger.LogInfo(rst);

    vTaskDelete(NULL);
}

void checkToReconnect() // added
{
    // disconnected so advertise
    if (deviceDisConnected)
    {
        // delay(500);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Disconnected: start advertising");
        deviceDisConnected = false;
    }
}

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        // init_queue();
        logger.LogInfo("BLE Client Connected");
    }
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        deviceDisConnected = true;
        logger.LogInfo("BLE Client Disconnect");
    }
};

class CmdCallbacks : public BLECharacteristicCallbacks
{

    void onWrite(BLECharacteristic *pCharacteristic)
    {

        uint8_t *pData;
        std::string rxValue = pCharacteristic->getValue();

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, rxValue.c_str());

        // Test if parsing succeeded.
        if (error)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.f_str());
        }
        else
        {
            // serializeJson(doc, Serial);

            String cmd = doc["cmd"];
            cmd.toLowerCase();

            if (cmd == "listdir")
            {
                xTaskCreate(
                    taskOne,   /*任务函数*/
                    "TaskOne", /*带任务名称的字符串*/
                    10000,     /*堆栈大小，单位为字节*/
                    NULL,      /*作为任务输入传递的参数*/
                    1,         /*任务的优先级*/
                    0);        /*任务句柄*/
                logger.LogInfo("cmd listdir run ok");
                // Serial.println("run cmd listdir");
            }
            else if (cmd == "delfile")
            {
                const char *params = doc["params"];
                //  char name[36] = "";
                // name = params.c_str();
                xTaskCreate(
                    taskDelfile,    /*任务函数*/
                    "taskDelfile",  /*带任务名称的字符串*/
                    10000,          /*堆栈大小，单位为字节*/
                    (void *)params, /*作为任务输入传递的参数*/
                    1,              /*任务的优先级*/
                    0);             /*任务句柄*/
            }
            else if (cmd == "downloadfile")
            {

                if (!isDownloading)
                {
                    const char *params = doc["params"];

                    xTaskCreate(
                        taskdDownloadfile,   /*任务函数*/
                        "taskdDownloadfile", /*带任务名称的字符串*/
                        10000,               /*堆栈大小，单位为字节*/
                        (void *)params,      /*作为任务输入传递的参数*/
                        1,                   /*任务的优先级*/
                        0);                  /*任务句柄*/
                }
                else
                {
                    pCharacteristicCMD->setValue(String("downloadfile:error:has_file_downloading").c_str());

                    pCharacteristicCMD->notify();
                }
            }
            else if (cmd == "baseinfo")
            {
                // const char *params = doc["params"];
                pCharacteristicCMD->setValue(("battery:" + (String)(100 - usage) + ";satellites:" + (String)(gps_data_2.numSV) + ";").c_str());

                pCharacteristicCMD->notify();
            }
            else if (cmd == "trackinfo")
            {
                // const char *params = doc["params"];

                xTaskCreate(
                    taskTrackInfo,   /*任务函数*/
                    "taskTrackInfo", /*带任务名称的字符串*/
                    10000,           /*堆栈大小，单位为字节*/
                    NULL,            /*作为任务输入传递的参数*/
                    1,               /*任务的优先级*/
                    0);              /*任务句柄*/
            }
            else if (cmd == "poweroff")
            {
                // const char *params = doc["params"];
                pCharacteristicCMD->setValue("poweroff:1");
                pCharacteristicCMD->notify();
            }
            else
            {
                logger.LogInfo("cmd unknown" + cmd);
                pCharacteristicCMD->setValue(("unknown:" + cmd).c_str());
                pCharacteristicCMD->notify();
            }
        }
    }
};

class downloadfileCallbacks : public BLECharacteristicCallbacks
{
    void onNotify(BLECharacteristic *pCharacteristic)
    {
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        uint8_t *pData;
        std::string rxValue = pCharacteristic->getValue();

        int val = stoi(rxValue);

        xQueueSend(xQueue_file_handle, &val, portMAX_DELAY);
        // Serial.print("resv file count = ");
        // Serial.println(val);
        taskYIELD();
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onNotify(BLECharacteristic *pCharacteristic)
    {
        // uint8_t *pData;
        // std::string value = pCharacteristic->getValue();
        // int len = value.length();
        // pData = pCharacteristic->getData();
        // if (pData != NULL)
        // {
        //     //        Serial.print("Notify callback for characteristic ");
        //     //        Serial.print(pCharacteristic->getUUID().toString().c_str());
        //     //        Serial.print(" of data length ");
        //     //        Serial.println(len);
        //     Serial.print("TX  ");
        //     for (int i = 0; i < len; i++)
        //     {
        //         Serial.printf("%02X ", pData[i]);
        //     }
        //     Serial.println();
        // }
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        uint8_t *pData;
        std::string rxValue = pCharacteristic->getValue();

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, rxValue);

        // Test if parsing succeeded.
        if (error)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.f_str());
        }
        else
        {
            serializeJson(doc, Serial);
            Serial.println("receive ok");
        }

        return;

        int len = rxValue.length();
        pData = pCharacteristic->getData();
        if (pData != NULL)
        {
            Serial.print("Write callback for characteristic ");
            Serial.print(pCharacteristic->getUUID().toString().c_str());
            Serial.print(" of data length ");
            Serial.println(len);
            Serial.print("Cmd: ");
            Serial.println(rxValue.c_str());
            Serial.print("RX  ");
            // std::string lwnCommand = "lwn";
            for (int i = 0; i < len; i++)
            { // leave this commented
                Serial.printf("%02X ", pData[i]);
            }
            Serial.println();
        }
    }
};

void HAL::BLE_Init()
{
    Serial.println("Starting BLE work!");

    uint64_t chipid;

    // chipid = ESP.getEfuseMac();                                      // The chip ID is essentially its MAC address(length: 6 bytes).
    // Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32)); // print High 2 bytes
    // Serial.printf("%08X\n", (uint32_t)chipid);                       // print Low 4bytes.

    // Serial.println(WiFi.macAddress());
    // Serial.println(("XLAPBLE_" + WiFi.macAddress().substring(0, 2) + WiFi.macAddress().substring(3, 5) + WiFi.macAddress().substring(6, 8) + WiFi.macAddress().substring(9, 11) + WiFi.macAddress().substring(12, 14) + WiFi.macAddress().substring(15, 17)).c_str());
    BLEDevice::init(("XLAP_" + WiFi.macAddress().substring(0, 2) + WiFi.macAddress().substring(3, 5) + WiFi.macAddress().substring(6, 8) + WiFi.macAddress().substring(9, 11) + WiFi.macAddress().substring(12, 14) + WiFi.macAddress().substring(15, 17)).c_str());
    // BLEDevice::init(("XLAPBLE_" + WiFi.macAddress()).c_str());

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID("0000"));

    pCharacteristicCMD = pService->createCharacteristic(SERVICE_UUID("0001"), BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    pCharacteristicTrackInfo = pService->createCharacteristic(SERVICE_UUID("0002"), BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristicListDir = pService->createCharacteristic(SERVICE_UUID("0003"), BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristicDownloadFile = pService->createCharacteristic(SERVICE_UUID("0004"), BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

    // pCharacteristicTX = pService->createCharacteristic(SERVICE_UUID("0002"), BLECharacteristic::PROPERTY_NOTIFY);
    // pCharacteristicRX = pService->createCharacteristic(SERVICE_UUID("0003"), BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    // pCharacteristicRX->setCallbacks(new MyCallbacks());
    // pCharacteristicTX->setCallbacks(new MyCallbacks());
    pCharacteristicCMD->setCallbacks(new CmdCallbacks());
    pCharacteristicTrackInfo->setCallbacks(new MyCallbacks());
    pCharacteristicListDir->setCallbacks(new MyCallbacks());
    pCharacteristicDownloadFile->setCallbacks(new downloadfileCallbacks());

    // pCharacteristicTX->addDescriptor(new BLE2902());
    // pCharacteristicTX->setNotifyProperty(true);
    // pCharacteristicCMD->addDescriptor(new BLE2902());
    // pCharacteristicCMD->setNotifyProperty(true);

    pCharacteristicCMD->addDescriptor(new BLE2902());
    pCharacteristicCMD->setNotifyProperty(true);
    pCharacteristicListDir->addDescriptor(new BLE2902());
    pCharacteristicListDir->setNotifyProperty(true);
    pCharacteristicDownloadFile->addDescriptor(new BLE2902());
    pCharacteristicDownloadFile->setNotifyProperty(true);
    pService->start();

    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID("0000"));
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    logger.LogInfo("Xlap BLE Service starting");

    init_queue();

    // test_crc32_string();
}

void HAL::BLE_Update()
{
    checkToReconnect();
}