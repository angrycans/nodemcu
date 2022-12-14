
#include "HAL/HAL.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

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

void checkToReconnect() // added
{
    // disconnected so advertise
    if (deviceDisConnected)
    {
        delay(500);                  // give the bluetooth stack the chance to get things ready
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
            Serial.println("\nreceive ok");

            String cmd = doc["cmd"];
            cmd.toLowerCase();

            if (cmd == "listdir")
            {
                Serial.println("run cmd listdir");
            }
        }
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onNotify(BLECharacteristic *pCharacteristic)
    {
        uint8_t *pData;
        std::string value = pCharacteristic->getValue();
        int len = value.length();
        pData = pCharacteristic->getData();
        if (pData != NULL)
        {
            //        Serial.print("Notify callback for characteristic ");
            //        Serial.print(pCharacteristic->getUUID().toString().c_str());
            //        Serial.print(" of data length ");
            //        Serial.println(len);
            Serial.print("TX  ");
            for (int i = 0; i < len; i++)
            {
                Serial.printf("%02X ", pData[i]);
            }
            Serial.println();
        }
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

    BLEDevice::init("XLAPBLE01");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID("0000"));

    pCharacteristicCMD = pService->createCharacteristic(SERVICE_UUID("0001"), BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    pCharacteristicTrackInfo = pService->createCharacteristic(SERVICE_UUID("0002"), BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristicListDir = pService->createCharacteristic(SERVICE_UUID("0003"), BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    pCharacteristicDownloadFile = pService->createCharacteristic(SERVICE_UUID("0004"), BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);

    // pCharacteristicTX = pService->createCharacteristic(SERVICE_UUID("0002"), BLECharacteristic::PROPERTY_NOTIFY);
    // pCharacteristicRX = pService->createCharacteristic(SERVICE_UUID("0003"), BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    // pCharacteristicRX->setCallbacks(new MyCallbacks());
    // pCharacteristicTX->setCallbacks(new MyCallbacks());
    pCharacteristicCMD->setCallbacks(new CmdCallbacks());
    pCharacteristicTrackInfo->setCallbacks(new MyCallbacks());
    pCharacteristicListDir->setCallbacks(new MyCallbacks());
    pCharacteristicDownloadFile->setCallbacks(new MyCallbacks());

    // pCharacteristicTX->addDescriptor(new BLE2902());
    // pCharacteristicTX->setNotifyProperty(true);
    // pCharacteristicCMD->addDescriptor(new BLE2902());
    // pCharacteristicCMD->setNotifyProperty(true);

    // pCharacteristicTrackInfo->addDescriptor(new BLE2902());
    // pCharacteristicTrackInfo->setNotifyProperty(true);
    pService->start();

    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID("0000"));
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    logger.LogInfo("Xlap BLE Service starting");
}

void HAL::BLE_Update()
{
    checkToReconnect();
}