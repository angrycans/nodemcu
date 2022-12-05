
#include "HAL/HAL.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "freertos/queue.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX "fb1e4002-54ae-4a28-9f74-dfccb248601d"
#define CHARACTERISTIC_UUID_TX "fb1e4003-54ae-4a28-9f74-dfccb248601d"

static BLECharacteristic *pCharacteristicTX;
static BLECharacteristic *pCharacteristicRX;
static bool deviceConnected = false;

bool compareData(std::string received, std::string predefined)
{
    int receivedLength = received.length();
    int predefinedLength = predefined.length();

    if ((receivedLength / 2) != predefinedLength)
    {
        return false;
    }

    for (int i = 0; i < predefinedLength; i++)
    {
        if (received[i * 2] != predefined[i])
        {
            return false;
        }
    }

    return true;
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
        logger.LogInfo("BLE Client Disconnect");
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
            std::string lwnCommand = "lwn";

            if (compareData(rxValue, lwnCommand))
            {
                Serial.println("lwn command received");
            }
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
    logger.LogInfo("Xlap BLE Service starting");
}
