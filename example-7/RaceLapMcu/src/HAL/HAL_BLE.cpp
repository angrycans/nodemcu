
#include "HAL/HAL.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX "fb1e4002-54ae-4a28-9f74-dfccb248601d"
#define CHARACTERISTIC_UUID_TX "fb1e4003-54ae-4a28-9f74-dfccb248601d"

static BLECharacteristic *pCharacteristicTX;
static BLECharacteristic *pCharacteristicRX;

uint8_t updater[16384];
uint8_t updater2[16384];
static bool deviceConnected = false, sendMode = false, sendSize = true;
static bool current = true;
static int parts = 0, next = 0, cur = 0, MTU = 0;
static int writeLen = 0, writeLen2 = 0;
static bool writeFile = false, request = false;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{

    //    void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code) {
    //      Serial.print("Status ");
    //      Serial.print(s);
    //      Serial.print(" on characteristic ");
    //      Serial.print(pCharacteristic->getUUID().toString().c_str());
    //      Serial.print(" with code ");
    //      Serial.println(code);
    //    }

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
        std::string value = pCharacteristic->getValue();
        int len = value.length();
        pData = pCharacteristic->getData();
        if (pData != NULL)
        {
            Serial.print("Write callback for characteristic ");
            Serial.print(pCharacteristic->getUUID().toString().c_str());
            Serial.print(" of data length ");
            Serial.println(len);
            Serial.print("RX  ");
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

    BLEDevice::init("XLAPBLE");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristicRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
    pCharacteristicRX->setCallbacks(new MyCallbacks());
    pCharacteristicTX->setCallbacks(new MyCallbacks());
    pCharacteristicTX->addDescriptor(new BLE2902());
    pCharacteristicTX->setNotifyProperty(true);
    pService->start();

    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}
