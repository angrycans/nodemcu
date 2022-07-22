#include <ESP8266WiFi.h>
#include <FS.h>       // File System for Web Server Files
#include <LittleFS.h> // This file system is used.
#include <Wire.h>
#include "SSD1306Wire.h"
#include "SH1106Wire.h"
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <TinyGPS++.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
//#include "MillisTaskManager.h"

#include "SDLogger.h"

#include "helper.hpp"
#include "race.hpp"

#define LED D0 // Led in NodeMCU at pin GPIO16 (D0).

#define OLED13
#define NEO_M10

#define DEBUG

#if defined(NEO_M10)
const uint8_t UBLOX_INIT[] PROGMEM = {
    // Rate (pick one)
    0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12, //(10Hz)
    // 0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A, //(5Hz)
    // 0xB5,0x62,0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00,0x01,0x39, //(1Hz)
    // Disable specific NMEA sentences
    // 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x24, // GxGGA off
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x2B, // GxGLL off
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32, // GxGSA off
    // 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39, // GxGSV off
    // 0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x04,0x00,0x00,0x00,0x00,0x00,0x01,0x04,0x40, // GxRMC off
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x47, // GxVTG off
    // 57600
    0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xE1,
    0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xC3,
    0xb5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01, 0x08, 0x22};

SoftwareSerial ss(D4, D0); // RX, TX
#else
SoftwareSerial ss(D4, D0); // RX, TX
#endif

int debug = 0; // 是否是接了usb debug启动
SDLogger logger;
TinyGPSPlus gps;
// MillisTaskManager taskManager;
Race race;
// gps_fix fix;

File dataFile;

// File gpsFile;

char DataFileName[64] = ""; //"RL2022-05-18_14.txt";
char DataFileDir[24] = "/RLDATA/";

bool B_SD = false; // SD Card status

// SD Reader CS pin
const int chipSelect = 15;

char buffer[120];
char lastbuffer[100];
char logbuff[100];

int preRecordCd = 3;
int recordtoLoopCd = 10;
double RecordKmph = 3;

unsigned long lastdevtime = 0;
double lastkmph = 0;
double KMPH = 0; // current speed
// int totalLap = 0;

bool isSetTime = false;

#include "webserver_help.hpp"
#include "display_helper.hpp"
#include "gps_helper.hpp"

void initWifi()
{

  logger.LogInfo("init Wifi");
  // delay(250);
  const char *ssid = "RaceLap";      // Enter SSID here
  const char *password = "88888888"; // Enter Password here
  WiFi.softAP(ssid, password);
  // delay(250);
  logger.LogInfo("Soft-AP IP address = ");
  logger.LogInfo(WiFi.softAPIP().toString().c_str());
  // delay(250);

  // Serial.print("DHCP status:");
  // Serial.println(wifi_softap_dhcps_status());

  // IPAddress local_ip(192, 168, 4, 1);
  // IPAddress gateway(192, 168, 4, 1);
  // IPAddress subnet(255, 255, 255, 0);

  // WiFi.softAPConfig(local_ip, gateway, subnet);
  // delay(50);
  // WiFi.begin("wifi-acans", "85750218");

  // WiFi.begin("yunweizu", "yunweizubangbangda");
  // //   WiFi.begin("qianmi-mobile", "qianmi123");

  // // WiFi.begin("wifi-acans", "85750218");
  // logger.LogInfo("Connecting");

  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);

  //   logger.LogInfo(".");
  // }

  // logger.LogInfo("Connected, IP address: ");
  // logger.LogInfo(WiFi.localIP().toString().c_str());

  // sprintf(displayInfo.log, WiFi.localIP().toString().c_str());
  // showDisplay();
  // delay(2000);
}

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

void initSD()
{
  // // keep checking the SD reader for valid SD card/format
  delay(100);
  Serial.println("init SD Card");

  while (!SD.begin(chipSelect))
  {
    Serial.println("init SD Card Failed");
    ErrInfo += "SD CARD FAILED\n";
    showDisplay();
    B_SD = false;
  }

  B_SD = true;
  logger.Begin(B_SD);
  if (SD.mkdir("RLDATA"))
  {
    Serial.println("RLDATA dir is created.");
  }

  // File root = SD.open("/RLDATA");

  // printDirectory(root, 0);

  // root.close();

  // logger.LogInfo("print RLDATA Directory done!");

  getTrack();

  // sprintf(displayInfo.log, "init sd card done!");
  // showDisplay();
  delay(100);
  logger.LogInfo("init sd card done!");
  // ErrInfo += "sdcard init ok.\n";
}

void initGps()
{

  logger.LogInfo("init GPS");
  delay(2000);
#if defined(NEO_M10)
  logger.LogInfo("init GPS NEO_M10");

  ss.begin(9600);
  // delay(250);
  while (1)
  {
    if (ss.available() > 0)
    {
      Serial.println("gpsSerial.available");

      for (uint8_t i = 0; i < sizeof(UBLOX_INIT); i++)
      {
        ss.write(pgm_read_byte(UBLOX_INIT + i));
      }

      delay(2000);
      break;
    }
    delay(500);
    Serial.println("gpsSerial connet .");
  }
  // delay(250);

  logger.LogInfo("init GPS NEO_M10 end");
  delay(1000);
  ss.begin(57600);
#else
  ss.begin(57600);
#endif

  // int at = millis();

  smartDelay(1000);

  //   if (gps.charsProcessed() < 10)
  //   {
  //     ErrInfo += "GPS init failed.\n";
  // #if defined(DEBUG)
  //     logger.LogInfo("GPS init failed. No GPS data received: check wiring");
  // #endif
  //   }
  // Serial.println(F("No GPS data received: check wiring"));
  int at = millis();
  while (!ss.available())
  {
    if (millis() - at > 5000)
    {
      ErrInfo += "GPS init failed.\n";
#if defined(DEBUG)
      logger.LogInfo("init GPS failed.");
#endif
      break;
    }
  }
#if defined(DEBUG)
  logger.LogInfo("init GPS ok");
#endif

  lastdevtime = 0;
}

void recordGps()
{

  if ((race.getStatus().status == d_gps_searching) && ((int)(millis() - race.getStatus().timer) > 5000 && gps.charsProcessed() < 10))
  {
    ErrInfo = "No GPS data received,reboot";
#if defined(DEBUG)
    snprintf(logbuff, sizeof(logbuff), "No GPS data received");
    logger.LogInfo(logbuff);
#endif
  }

  // if (gps.location.isUpdated())
  if (gps.location.isValid())
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

    snprintf(buffer, sizeof(buffer),
             "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%lu,%d",
             year,
             month, day, hour, minute, second, csecond, lat, lng, altitude, KMPH, deg, millis(), satls);

#if defined(DEBUG)
    // Serial.println(buffer);
#endif

    if (B_SD)
    {
      if ((race.getStatus().status == d_Recording || race.getStatus().status == d_RecordToStop))
      {

        if (!dataFile)
        {
          if (strcmp(DataFileName, "") == 0)
          {
            sprintf(DataFileName, "%sRL%04d%02d%02d%02d%02d%02d.txt", DataFileDir, year, month, day, hour, minute, second);
          }
#if defined(DEBUG)
          snprintf(logbuff, sizeof(logbuff), "[%s]new DataFileName recording %s", formatTime(millis()), DataFileName);
          logger.LogInfo(logbuff);
#endif
          dataFile = SD.open(DataFileName, FILE_WRITE);
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
      snprintf(logbuff, sizeof(logbuff), "[%s]d_preRecord to d_Recording >preRecordCd %d", formatTime(millis()), preRecordCd);
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
    race.computerLapinfo(&gps);

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

void initLed()
{
  pinMode(LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void setup()
{

  Serial.begin(9600);
  race.setStatus(d_Setup);
  Serial.print("setup...");
  Serial.println(debug);
  initLed();
  Serial.println("initDisplay...");
  initDisplay();
  showLogoDisplay();

  Serial.println("init LittleFS filesystem...");
  if (!LittleFS.begin())
  {
    logger.LogInfo("could not mount LittleFS filesystem...");
    delay(100);
    ErrInfo += "GPS init failed.\n";
    // ESP.restart();
  }

  initSD();
  initWifi();
  initWebServer();
  initGps();

  race.setStatus(d_gps_searching);

  snprintf(logbuff, sizeof(logbuff), "init ok debug=%d", debug);
  logger.LogInfo(logbuff);

  delay(250);
  setDisplayFrame(0);
}

void loop()
{
  digitalWrite(LED_BUILTIN, (millis() / 1000) % 2);

  // #if defined(DEBUG)
  //   snprintf(logbuff, sizeof(logbuff), "satellites %d", (int)gps.satellites.value());
  //   Serial.println(logbuff);
  // #endif
  showDisplay();
  while (ss.available() > 0)
  {
    // char inByte = ss.read();
    // Serial.print(inByte);
    //  #if defined(DEBUG)
    //      // snprintf(logbuff, sizeof(logbuff), "satellites %d", (int)gps.satellites.value());
    //      gpsFile.print(inByte);
    //      gpsFile.flush();
    //  #endif
    if (gps.encode(ss.read()))
    {
      // printData();
      recordGps();
    }
  }
  //   taskManager.Running(millis());
}
