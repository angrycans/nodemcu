
// #define LV_CONF_INCLUDE_SIMPLE 1
// #define LV_CONF_PATH "lv_conf.h"
#include "Wire.h"
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>


#include "ublox.h"

#define CONFIG_SDCARD_SCK 15
#define CONFIG_SDCARD_MISO 16
#define CONFIG_SDCARD_MOSI 5
#define CONFIG_SDCARD_CS 4

#define UBLOX_RX RX
#define UBLOX_TX TX

#define gpsSerial Serial1

#define LED_RUN 1

void printDirectory(File dir, int numTabs)
{
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
      // time_t cr = entry.getCreationTime();
      // time_t lw = entry.getLastWrite();
      // struct tm *tmstruct = localtime(&cr);
      // Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      //  tmstruct = localtime(&lw);
      // Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}

void SDCARD_Init()
{

  // // keep checking the SD reader for valid SD card/format
  delay(1000);
  Serial.println("init SD Card");

  SPI.begin(CONFIG_SDCARD_SCK, CONFIG_SDCARD_MISO, CONFIG_SDCARD_MOSI, -1);
  // while (!SD.begin(SD_CS, SPI, 2000000))
  // while (!SD.begin(CONFIG_SDCARD_CS, SPI, 2000000))
  // while (!SD.begin(CONFIG_SDCARD_CS, SPI, 80000000))
  while (!SD.begin(CONFIG_SDCARD_CS, SPI, 80000000))
  // while (!SD.begin(CONFIG_SDCARD_CS))
  {
    Serial.println("init SD Card Failed");
  }

  File root = SD.open("/");

  printDirectory(root, 0);

  root.close();
}

void GPS_Init()
{


    gpsSerial.begin(115200, SERIAL_8N1, UBLOX_RX, UBLOX_TX);
    // ublox_init();
    Serial.print("init GPS ok");
}


void GPS_Update()
{
    while (gpsSerial.available() > 0)
    {
        uint8_t inByte = gpsSerial.read();
        //Serial.print(inByte);
        ublox_decode(inByte);

    }
}


void setup(void)
{
  // SPIバスとパネルの初期化を実行すると使用可能になります。

  Serial.begin(115200);

  SDCARD_Init();
  delay(1000);
  Serial.println("test start");

 

  pinMode(LED_RUN, OUTPUT);
  //      Wire.begin(CTP_SDA, CTP_SCL);
  // cts.begin(SAFE);
  // cts.setTouchLimit(1);//from 1 to 5
}

void loop(void)
{
  digitalWrite(LED_RUN, (millis() / 1000) % 2);
}
