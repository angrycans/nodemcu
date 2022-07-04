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
#include <ArduinoJson.h>
#include "SDLogger.h"
#include "race.hpp"

void showDisplay();
void setFinishLinePos();
void handleSetSpeed(AsyncWebServerRequest *request);

#define LED D0 // Led in NodeMCU at pin GPIO16 (D0).

#define OLED13
#define NEO_M10

#define DEBUG

#if defined(OLED13)
SH1106Wire display(0x3c, SDA, SCL); // 1.3 SH1106 d2 d1
#else
SSD1306Wire display(0x3c, SDA, SCL); // 0.96 ssd1306 d2 d1
#endif
//
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
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39, // GxGSV off
    // 0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x04,0x00,0x00,0x00,0x00,0x00,0x01,0x04,0x40, // GxRMC off
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x47, // GxVTG off
    // 57600
    0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xE1,
    0x00, 0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0xC3,
    0xb5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01, 0x08, 0x22};
SoftwareSerial ss(D4, D0); // RX, TX

#else
SoftwareSerial ss(D4, D0);           // RX, TX
#endif

AsyncWebServer server(80);

ADC_MODE(ADC_VCC);

int debug = 0; // 是否是接了usb debug启动
SDLogger logger;
TinyGPSPlus gps;
Race race;
// gps_fix fix;

File dataFile;

char DataFileName[64] = ""; //"RL2022-05-18_14.txt";
char DataFileDir[24] = "/RLDATA/";

bool B_SD = false;       // SD Card status
bool IS_DISPLAY = false; //  OLED display status

// SD Reader CS pin
const int chipSelect = 15;

char buffer[100];
char lastbuffer[100];
char logbuff[100];

unsigned long lastDisplayTime = 0;

int preRecordCd = 3;
int recordtoLoopCd = 30;
double RecordKmph = 30;

unsigned long lastdevtime = 0;
double lastkmph = 0;
double KMPH = 0;

class TDisplayInfo
{
public:
  char title[30];
  char log[100];
  char status[20];
};

// typedef struct TDisplayInfo DisplayInfo;

TDisplayInfo displayInfo;

char *dTime()
{
  char *tmp = new char[21];
  unsigned long minutes = millis() / 60000;
  unsigned long seconds = (millis() / 1000) - ((millis() / 60000) * 60);
  unsigned long tenths = (millis() / 100) % 10;
  sprintf(tmp, "%02lu:%02lu.%03lu", minutes, seconds, tenths);

  return tmp;
}

String file_size(int bytes)
{
  String fsize = "";
  if (bytes < 1024)
    fsize = String(bytes) + " B";
  else if (bytes < (1024 * 1024))
    fsize = String(bytes / 1024.0, 3) + " KB";
  else if (bytes < (1024 * 1024 * 1024))
    fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
  else
    fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
  return fsize;
}

String ListDirectoryJSON()
{

  File dir = SD.open("/RLDATA");
  String tree = "";
  int count = 0;
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry || count > 30)
    {
      // no more files
      break;
    }

    if (!entry.isDirectory())
    {

      String str = entry.name();
      if (strcmp(DataFileName, str.c_str()) != 0)
      {
        tree = tree + "/" + dir.name() + "/" + entry.name() + "_" + entry.size() + ",";
        count++;
      }
    }
    entry.close();
  }

  dir.close();

  return tree;
}

String ListDirectory()
{

  File dir = SD.open("/RLDATA");
  String tree = F("<table>");

  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }

    if (entry.isDirectory())
    {
      // tree += F("<tr>");
      // tree += F("<td>");
      // tree += entry.name();
      // tree += F("/</td><td></td>");
      // tree += F("<td class=\"detailsColumn\" data-value=\"0\">-</td>");
      // tree += F("<td class=\"detailsColumn\" data-value=\"0\">");
      // tree += F("<button class='buttons' onclick=\"location.href='/del?dir=/");
      // tree = tree + dir.name() + "/" + entry.name();
      // tree += F("';\">del</button></td>");
      // tree += F("</tr>");
    }
    else
    {
      tree += F("<tr><td></td>");
      tree += F("<td>");
      tree += entry.name();
      tree += F("</td>");
      tree += F("<td class=\"detailsColumn\">");
      tree += file_size(entry.size());
      tree += F("</td>");
      tree += F("<td class=\"detailsColumn\">");
      tree += F("<button class='buttons' onclick=\"location.href='/down?file=/");
      tree = tree + dir.name() + "/" + entry.name();
      tree += F("'\">down</button></td>");
      tree += F("<td class=\"detailsColumn\">");
      tree += F("<button class='buttons' onclick=\"location.href='/del?file=/");
      tree = tree + dir.name() + "/" + entry.name();
      tree += F("'\">del</button></td>");
      tree += F("</tr>");
    }
    entry.close();
  }

  tree += F("</table>");

  Serial.println(tree);
  return tree;
}

void notFound(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    request->send(404);
  }
}

void handleGetMcuCfg(AsyncWebServerRequest *request)
{
  File file = SD.open("/RLDATA/track.txt");

  StaticJsonDocument<200> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (!error)
  {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<512> ret;

    JsonObject e = ret.createNestedObject("e");
    e["code"] = 1;
    ret["data"] = doc.as<JsonObject>();

    serializeJson(ret, *response);
    request->send(response);
  }
  else
  {
    logger.LogInfo(F("Failed to read track.txt"));
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<200> ret;

    JsonObject e = ret.createNestedObject("e");
    e["code"] = -1;
    e["msg"] = "Failed to read track.txt";

    serializeJson(ret, *response);
    request->send(response);
  }
  file.close();
}
void handleSysinfo(AsyncWebServerRequest *request)
{

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  StaticJsonDocument<200> doc;

  JsonObject e = doc.createNestedObject("e");
  e["code"] = 1;
  JsonObject data = doc.createNestedObject("data");
  // String str =
  data["datafilename"] = String(DataFileName);
  data["RaceStatus"] = race.getStatus().status;
  data["RaceStatusEnum"] = "0 _Setup 1_preRecord 2_Recording 3_RecordToSleep 4_Sleep";
  data["RAM"] = ESP.getFreeHeap();
  data["FreeHeap"] = ESP.getMaxFreeBlockSize();
  data["Vcc"] = ESP.getVcc() / 1024.0;
  data["Version"] = ESP.getSdkVersion();
  data["CPU"] = ESP.getCpuFreqMHz();
  data["ResetReason"] = ESP.getResetReason();

  serializeJson(doc, *response);
  request->send(response);
}

void handleGetLocation(AsyncWebServerRequest *request)
{

  if (gps.location.isValid())
  {

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<200> doc;

    JsonObject e = doc.createNestedObject("e");
    e["code"] = 1;
    JsonObject data = doc.createNestedObject("data");
    data["lat"] = gps.location.lat();
    data["lng"] = gps.location.lng();

    serializeJson(doc, *response);
    request->send(response);
  }
  else
  {

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<200> doc;

    JsonObject e = doc.createNestedObject("e");
    e["code"] = 0;
    e["msg"] = "Gps not valid";

    serializeJson(doc, *response);
    request->send(response);
  }
}

void handleSetLocation(AsyncWebServerRequest *request)
{
  StaticJsonDocument<512> params;
  params["lat1"] = request->getParam("lat1")->value();
  params["lng1"] = request->getParam("lng1")->value();
  params["lat2"] = request->getParam("lat2")->value();
  params["lng2"] = request->getParam("lng2")->value();
  params["trackname"] = request->getParam("trackname")->value();

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  StaticJsonDocument<200> doc;

  JsonObject e = doc.createNestedObject("e");

  SD.remove("/RLDATA/track.txt");

  File file = SD.open("/RLDATA/track.txt", FILE_WRITE);
  if (!file)
  {
    e["code"] = 1;
    e["msg"] = "file open error";
    serializeJson(doc, *response);
    request->send(response);
    file.close();
    return;
  }

  // Serialize JSON to file
  String output;
  serializeJson(doc, output);

  if (serializeJson(params, file) == 0)
  {
    e["code"] = 1;
    e["msg"] = "serializeJson error";
    file.close();
    serializeJson(doc, *response);
    request->send(response);
  }
  else
  {
    e["code"] = 1;
    e["msg"] = "set ok";
    file.close();

    setFinishLinePos();
    serializeJson(doc, *response);
    request->send(response);
  }

  // Close the file
}

void handleSetSpeed(AsyncWebServerRequest *request)
{

  int speed = request->getParam("speed")->value().toInt();

  if (speed == -1)
  {

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<200> doc;

    JsonObject e = doc.createNestedObject("e");
    e["code"] = 1;
    JsonObject data = doc.createNestedObject("data");
    data["speed"] = RecordKmph;

    serializeJson(doc, *response);
    request->send(response);
  }
  else if (speed > 0)
  {

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<200> doc;

    RecordKmph = speed;

    JsonObject e = doc.createNestedObject("e");
    e["code"] = 1;
    e["msg"] = "set ok";

#if defined(DEBUG)
    snprintf(logbuff, sizeof(logbuff), "[%s]SetSpeed %d %f", dTime(), speed, RecordKmph);
    logger.LogInfo(logbuff);
#endif
    serializeJson(doc, *response);
    request->send(response);
  }
}

void initWifi()
{

  logger.LogInfo("init Wifi");
  sprintf(displayInfo.log, "init Wifi...");
  showDisplay();
  delay(250);

  const char *ssid = "RaceLap";      // Enter SSID here
  const char *password = "88888888"; // Enter Password here
  WiFi.softAP(ssid, password);
  delay(250);
  logger.LogInfo("Soft-AP IP address = ");
  logger.LogInfo(WiFi.softAPIP().toString().c_str());
  sprintf(displayInfo.log, WiFi.softAPIP().toString().c_str());
  showDisplay();
  delay(250);

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

void initWebServer()
{

  logger.LogInfo("initWebServer");

  server.onNotFound(notFound);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.on("/listsd1", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", ListDirectory()); });

  server.on("/listsdjson", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", ListDirectoryJSON()); });
  server.on("/sysinfo", HTTP_GET, handleSysinfo);

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("file"))
              {
                String message = request->getParam("file")->value();
                  snprintf(logbuff, sizeof(logbuff), "downfile %s",message.c_str());
                 logger.LogInfo(logbuff);
     
               File sfile = SD.open(message, FILE_READ);

               if (sfile){
                  request->send(sfile, message.c_str(), "text/xhr");
               }else{
                  request->send(200, "text/plain", "file no Exists ");
               }
              }else{
                request->send(200, "text/plain", "Params error");
              } });

  server.on("/del", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("file"))
    {
      String message =  request->getParam("file")->value();
       snprintf(logbuff, sizeof(logbuff), "delfile %s", message.c_str());
       logger.LogInfo(logbuff);
     
      // request->send(200, "text/plain", "Params ok");
      // root = SD.open("/RLDATA");
      
      if (SD.remove(message)){
            request->send(200, "text/plain", "{\"e\":{\"code\":1}}");
      }else{
          request->send(200, "text/plain", "{\"e\":{\"code\":-1}}");
      }

      
    }
    else if (request->hasParam("dir"))
    {
      String message =  request->getParam("dir")->value();


      SD.rmdir(message);
      request->send(200, "text/plain", "dir del  ok ");

    }else
      {
        request->send(200, "text/plain", "Params error");
      } });

  server.on("/getlocation", HTTP_GET, handleGetLocation);
  server.on("/getmcucfg", HTTP_GET, handleGetMcuCfg);

  server.on("/setlocation", HTTP_GET, handleSetLocation);
  server.on("/setspeed", HTTP_GET, handleSetSpeed);

  server.begin();
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

void initDisplay()
{
  logger.LogInfo("init Display");

  display.init();

  IS_DISPLAY = true;
  delay(250);

  sprintf(displayInfo.title, "RaceLap  0.7");

  logger.LogInfo("init Display ok");
  // }
}

void showDisplay()
{
  // TRACE("showDisplay");
  if (IS_DISPLAY)
  {

    display.clear();

    switch (race.getStatus().status)
    {
    case d_Looping:
      char tmpbuff[100];
      if (race.sessionActive)
      {

        sprintf(tmpbuff, "Best Session:%s", race.formatSessionTime(race.bestSessionTime).c_str());
        display.drawString(0, 0, tmpbuff);

        sprintf(tmpbuff, "total lap:%d best:%d", race.totolLap, race.bestLap);
        display.drawString(0, 0, tmpbuff);
        sprintf(tmpbuff, "maxspeed:%.2f", race.maxspeed);
        display.drawString(0, 0, tmpbuff);

        return;
      }

      display.setFont(ArialMT_Plain_10);

      if (gps.satellites.isValid())
      {
        sprintf(tmpbuff, "Sat:%d", gps.satellites.value());
      }
      else
      {
        sprintf(tmpbuff, "Sat:Unknow");
      }
      display.drawString(0, 0, tmpbuff);

      if (gps.location.isValid() && gps.satellites.value() > 3)
      {
        sprintf(tmpbuff, "lat:%.8f lng:%.8f", gps.location.lat(), gps.location.lng());
        display.drawString(0, 20, tmpbuff);
        sprintf(tmpbuff, "speed:%.2f", gps.speed.kmph());
        display.drawString(0, 40, tmpbuff);
      }
      else
      {

        if ((millis() / 1000) % 3 == 0)
        {
          display.drawString(0, 20, "Gps search.");
        }
        else if ((millis() / 1000) % 3 == 1)
        {
          display.drawString(0, 20, "Gps search..");
        }
        else
        {
          display.drawString(0, 20, "Gps search...");
        }
      }

      break;
    case d_Recording:
      display.setFont(ArialMT_Plain_24);

      // display.drawString(0, 20, race.getStatusName().c_str());
      sprintf(displayInfo.status, "%d", (int)KMPH);
      display.drawString(20, 0, displayInfo.status);

      break;
    case d_preRecord:
      display.setFont(ArialMT_Plain_10);

      // display.drawString(0, 20, race.getStatusName().c_str());
      sprintf(displayInfo.status, "%s %d %0.2f", race.getStatusName().c_str(), preRecordCd - (int)((millis() - race.getStatus().timer) / 1000), KMPH);
      display.drawString(0, 30, displayInfo.status);
      break;
    case d_RecordToLoop:
      display.setFont(ArialMT_Plain_10);

      // display.drawString(0, 20, race.getStatusName().c_str());
      sprintf(displayInfo.status, "%s %d %0.2f", race.getStatusName().c_str(), recordtoLoopCd - (int)((millis() - race.getStatus().timer) / 1000), KMPH);
      display.drawString(0, 30, displayInfo.status);
      break;
    default:
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 0, displayInfo.title);

      sprintf(displayInfo.status, "%s %0.2f", race.getStatusName().c_str(), KMPH);
      display.drawString(0, 30, displayInfo.status);
      if (strcmp(DataFileName, "") != 0)
      {
        display.drawString(0, 40, DataFileName);
      }

      if (strcmp(displayInfo.log, "") != 0)
      {
        display.drawString(0, 50, displayInfo.log);
      }
      break;
    }
    display.display();
  }
}

void initSD()
{
  // // keep checking the SD reader for valid SD card/format
  sprintf(displayInfo.log, "init SD card ...");
  showDisplay();
  delay(250);
  Serial.println("init SD Card");

  while (!SD.begin(chipSelect))
  {
    Serial.println("init SD Card Failed");
    B_SD = false;
    sprintf(displayInfo.log, "init SD Card Failed");

    if (millis() - lastDisplayTime > 500)
    {
      showDisplay();
      lastDisplayTime = millis();
    }
  }

  B_SD = true;
  logger.Begin(B_SD);
  if (SD.mkdir("RLDATA"))
  {
    Serial.println("RLDATA dir is created.");
  }

  File root = SD.open("/RLDATA");

  printDirectory(root, 0);

  root.close();

  logger.LogInfo("print RLDATA Directory done!");

  setFinishLinePos();

  sprintf(displayInfo.log, "init sd card done!");
  showDisplay();
  delay(250);
  logger.LogInfo("init sd card done!");
}

void initGps()
{
  logger.LogInfo("init GPS");
  sprintf(displayInfo.log, "init GPS");
  showDisplay();

  delay(1000);
#if defined(NEO_M10)
  logger.LogInfo("init GPS NEO_M10");
  ss.begin(9600);
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
    delay(100);
    Serial.println("gpsSerial connet .");
  }
  delay(100);

  logger.LogInfo("init GPS NEO_M10 end");
#endif

  delay(500);
  ss.begin(57600);
  while (!ss.available())
  {
    logger.LogInfo("GPS module invaild");
    sprintf(displayInfo.log, "GPS module invaild");

    if (millis() - lastDisplayTime > 500)
    {
      showDisplay();
      lastDisplayTime = millis();
    }
  }
#if defined(DEBUG)
  logger.LogInfo("init GPS ok");
#endif
  sprintf(displayInfo.log, "init GPS ok");

  showDisplay();
  delay(250);

  lastdevtime = 0;
}

void recordGps()
{
  if (gps.location.isUpdated())
  {
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
    int csecond = gps.time.centisecond();
    double deg = gps.course.deg();
    int satls = gps.satellites.value();

    if (race.last_gps.location.lat() == lat && race.last_gps.location.lng() == lng)
    {
      return;
    }

    //计算圈速
    if (race.segmentsIntersect(lat, lng, race.last_gps.location.lat(), race.last_gps.location.lng(), race.lat1, race.lng1, race.lat2, race.lng2))
    {
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "segmentsIntersect checked");
      logger.LogInfo(logbuff);
#endif
      if (!race.sessionActive)
      {
        race.sessionActive = true;
        race.sessionTime = millis();
        race.totolLap = 0;
        race.maxspeed = 0;
      }
      else
      {
        race.currentLap += 1;
        race.totolLap += 1;
        if (KMPH > race.maxspeed)
        {
          race.maxspeed = KMPH;
        }
        // this is the best or first lap
        if ((race.bestSessionTime > millis() - race.sessionTime) || (race.bestSessionTime == 0))
        { // test for session time and also for the very first lap, when bestSesstionTime is 0
          race.bestSessionTime = millis() - race.sessionTime;
          race.bestLap = race.currentLap;
        }

#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "[%s]currlap %d,totol %d,maxspeed:%f,bestlap:%d,bestsession:%lu", dTime(), race.currentLap, race.totolLap, race.maxspeed, race.bestLap, race.bestSessionTime);
        logger.LogInfo(logbuff);
#endif
      }
      // reset the sessionTime
      race.sessionTime = millis();
    }

    race.last_gps = gps;

    snprintf(buffer, sizeof(buffer),
             "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%.2f,%lu",
             year,
             month, day, hour, minute, second, csecond, lat, lng, altitude, KMPH, deg, millis(), satls);

    if (B_SD)
    {
      if ((race.getStatus().status == d_Recording || race.getStatus().status == d_RecordToLoop))
      {

        if (!dataFile)
        {
          if (strcmp(DataFileName, "") == 0)
          {
            sprintf(DataFileName, "%sRL%04d%02d%02d%02d%02d%02d.txt", DataFileDir, year, month, day, hour, minute, second);
          }
#if defined(DEBUG)
          snprintf(logbuff, sizeof(logbuff), "[%s]new DataFileName recording %s", dTime(), DataFileName);
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

  // GPS 长时间没有更新数据

  /*
    if (lastdevtime == 0)
    {
      lastdevtime = millis();
    }

    long diff = millis() - lastdevtime;

    if (diff >= 0 && diff < 3000)
    {
      KMPH = 0.5;
    }
    else if (diff > 3000 && diff < 5000)
    {
      KMPH = 30;
    }
    else if (diff > 5000 && diff < 9000)
    {
      KMPH = 2;
    }
    else if (diff > 9000 && diff < 20000)
    {
      KMPH = 23;
    }
    else if (diff > 20000 && diff < 35000)
    {
      KMPH = 0.5;
    }
    else if (diff > 35000 && diff < 58000)
    {
      KMPH = 30;
    }
    else if (diff > 58000 && diff < 65000)
    {
      KMPH = 2;
    }
    else if (diff > 65000)
    {
      KMPH = 5;
      lastdevtime = millis();
    }

    if (lastkmph != KMPH)
    {
      snprintf(logbuff, sizeof(logbuff), "kmph from %f to %f", lastkmph, KMPH);
      logger.LogInfo(logbuff);
      lastkmph = KMPH;
    }
    */

  switch (race.getStatus().status)
  {
  case d_Looping:

    if (KMPH > RecordKmph)
    {
      race.setStatus(d_preRecord);
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "[%s]d_Looping to d_preRecord kmph=%.2f", dTime(), KMPH);
      logger.LogInfo(logbuff);
#endif
    }

    break;
  case d_preRecord:
    if (KMPH < RecordKmph)
    {
      race.setStatus(d_Looping);
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "[%s]d_preRecord to d_Looping kmph=%.2f RecordKmph=%.2f", dTime(), KMPH, RecordKmph);
      logger.LogInfo(logbuff);
#endif
      return;
    }
    if (int((millis() - race.getStatus().timer) / 1000) > preRecordCd)
    {
      race.resetSession();
      race.setStatus(d_Recording);
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "[%s]d_preRecord to d_Recording >preRecordCd %d", dTime(), preRecordCd);
      logger.LogInfo(logbuff);
#endif
    }

    break;
  case d_Recording:
    if (int((millis() - race.lastGpsUpdateTimer) / 1000) > 5)
    {
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "[%s]GPS no signal %d", dTime(), int((millis() - race.lastGpsUpdateTimer) / 1000));
      logger.LogInfo(logbuff);
#endif
      KMPH = 0;
    }
    if (KMPH < RecordKmph)
    {
      race.setStatus(d_RecordToLoop);
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "[%s]d_Recording to d_RecordToLoop kmph=%.2f <RecordKmph=%.2f", dTime(), KMPH, RecordKmph);
      logger.LogInfo(logbuff);
#endif
    }

    break;

  case d_RecordToLoop:
    if (KMPH > RecordKmph)
    {
      race.setStatus(d_Recording);
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "[%s]d_RecordToLoop to d_Recording kmph=%.2f > RecordKmph=%.2f", dTime(), KMPH, RecordKmph);
      logger.LogInfo(logbuff);
#endif
      return;
    }
    if (int((millis() - race.getStatus().timer) / 1000) > recordtoLoopCd)
    {
      if (dataFile)
      {
#if defined(DEBUG)
        snprintf(logbuff, sizeof(logbuff), "[%s]close DataFileName %s", dTime(), DataFileName);
        logger.LogInfo(logbuff);
#endif
        dataFile.close();
        strcpy(DataFileName, "");
      }
      race.setStatus(d_Looping);
    }
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
  //等待4s 连接
  while (Serial.available() == 0 && millis() < 2000)
    ;
  if (Serial.available() > 0)
  {
    // If data is available, we enter here.
    int test = Serial.read(); // We then clear the input buffer

    Serial.println("USB DEBUG"); // Give feedback indicating mode

    debug = 1; // Enable debug
  }
  else
  {
    //  Serial.println("USB DEBUG"); // Give feedback indicating mode

    debug = 0; // Enable debug
  }

  race.setStatus(d_Setup);
  Serial.print("setup...");
  Serial.println(debug);
  initLed();
  Serial.println("initDisplay...");
  initDisplay();

  Serial.println("init LittleFS filesystem...");
  if (!LittleFS.begin())
  {
    logger.LogInfo("could not mount LittleFS filesystem...");
    delay(2000);
    ESP.restart();
  }

  initSD();

  initWifi();
  initWebServer();
  initGps();

  race.setStatus(d_Looping);

  snprintf(logbuff, sizeof(logbuff), "init ok debug=%d", debug);
  logger.LogInfo(logbuff);
}

void loop()
{
  digitalWrite(LED, (millis() / 1000) % 2);

  if (millis() - lastDisplayTime > 500)
  {
    showDisplay();
    lastDisplayTime = millis();
  }

  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      recordGps();
    }
  }
}

void setFinishLinePos()
{
  File file = SD.open("/RLDATA/track.txt");

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (!error)
  {
    // String output;
    // serializeJson(doc, output);
    // logger.LogInfo("track.txt " + output);

    race.lat1 = atof(doc["lat1"]);
    race.lng1 = atof(doc["lng1"]);
    race.lat2 = atof(doc["lat2"]);
    race.lng2 = atof(doc["lng2"]);

    char tmp[100];
    sprintf(tmp, "finishline %.8f %.8f %.8f %.8f", race.lat1, race.lng1, race.lat2, race.lng2);
    logger.LogInfo(tmp);
  }

  file.close();
}
