#include <ESP8266WiFi.h>
#include <FS.h>       // File System for Web Server Files
#include <LittleFS.h> // This file system is used.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoLog.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include "SDLogger.h"

#define LED D0 // Led in NodeMCU at pin GPIO16 (D0).

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C

SDLogger logger;
TinyGPSPlus gps;
// gps_fix fix;

File dataFile;

enum RACESTATUS
{
  _Setup,
  _Starting,
  _Recording,
  _RecordToSleep,
  _Sleep,
};

RACESTATUS RaceStatus = _Setup;

char DataFileName[64] = ""; //"RL2022-05-18_14.txt";
char DataFileDir[24] = "/RLDATA/";

bool B_SD = false;      // SD Card status
bool B_SSD1306 = false; // 0.96 OLED display status

// SD Reader CS pin
const int chipSelect = 15;

uint32_t last_satellites = 0;

AsyncWebServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SoftwareSerial ss(D3, D4); // RX, TX

static unsigned long lastDisplayTime = 0;

static unsigned long lastSpeedLimitTime = 0;

unsigned long StartTime = 0;
unsigned long lastStartTime = 0;
char buffer[100];
char lastbuffer[100];
char logbuff[100];

double RecordKmph = 30;
int SleepCD = 60;

double KMPH = 0;

double lastkmph = 0;

double last_lat = 0;
double last_lng = 0;
double last_altitude = 0;
double last_KMPH = 0;
int last_year = 0;
int last_month = 0;
int last_day = 0;
int last_hour = 0;
int last_minute = 0;
int last_second = 0;
int last_csecond = 0;

class TDisplayInfo
{
public:
  char title[30];
  char log[100];

  // void showlog(String _log)
  // {
  //   if (_log != log)
  //   {
  //   }
  // }
};

// typedef struct TDisplayInfo DisplayInfo;

TDisplayInfo displayInfo;

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
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }

    if (!entry.isDirectory())
    {

      tree = tree + "/" + dir.name() + "/" + entry.name() + ",";
    }
    entry.close();
  }

  dir.close();

  return tree;
}

String ListDirectory()
{

  File dir = SD.open("/RLDATA");
  String tree = "<table>";
  String entryName = "";

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
      tree += F("<tr>");
      tree += F("<td>");
      tree += entry.name();
      tree += F("/</td><td></td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">-</td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">");
      tree += F("<button class='buttons' onclick=\"location.href='/del?dir=/");
      tree = tree + dir.name() + "/" + entry.name();
      tree += F("';\">del</button></td>");
      tree += F("</tr>");
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
    serializeJson(doc, *response);
    request->send(response);
  }

  // Close the file
}

void initWifi()
{

  logger.LogInfo("initWifi");

  // const char *ssid = "RaceLap";      // Enter SSID here
  // const char *password = "88888888"; // Enter Password here
  // WiFi.softAP(ssid, password);
  // delay(250);
  // logger.LogInfo("Soft-AP IP address = ");
  // logger.LogInfo(WiFi.softAPIP().toString().c_str());

  // Serial.print("DHCP status:");
  // Serial.println(wifi_softap_dhcps_status());

  // IPAddress local_ip(192, 168, 4, 1);
  // IPAddress gateway(192, 168, 4, 1);
  // IPAddress subnet(255, 255, 255, 0);

  // WiFi.softAPConfig(local_ip, gateway, subnet);
  // delay(50);
  // WiFi.begin("wifi-acans", "85750218");

  WiFi.begin("yunweizu", "yunweizubangbangda");

  logger.LogInfo("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    logger.LogInfo(".");
  }

  logger.LogInfo("Connected, IP address: ");
  logger.LogInfo(WiFi.localIP().toString().c_str());
}

void initWebServer()
{

  logger.LogInfo("initWebServer");
  server.onNotFound(notFound);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.on("/listsd", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", ListDirectory()); });

  server.on("/listsdjson", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", ListDirectoryJSON()); });

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
      SD.remove(message);

      request->send(200, "text/plain", "file del  ok ");
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
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm *tmstruct = localtime(&cr);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}

void initSD()
{
  // // keep checking the SD reader for valid SD card/format
  Serial.println("init SD Card");
  // while (!SD.begin(chipSelect))
  // {
  //   TRACE("SD Card Failed, Will Retry...");
  // }
  if (!SD.begin(chipSelect))
  {
    Serial.println("init SD Card Failed");
    B_SD = false;
  }
  else
  {
    B_SD = true;
    logger.Begin();
    if (SD.mkdir("RLDATA"))
    {
      Serial.println("dir is created.");
    }

    Log.begin(LOG_LEVEL_VERBOSE, &Serial);

    File root = SD.open("/RLDATA");

    printDirectory(root, 0);

    root.close();

    logger.LogInfo("print RLDATA Directory done!");

    File file = SD.open("/RLDATA/track.txt");

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (!error)
    {
      String output;
      serializeJson(doc, output);
      logger.LogInfo("track.txt " + output);
    }

    file.close();

    if (dataFile)
    {
      dataFile.close();
    }
  }
}

void initDisplay()
{
  logger.LogInfo("init Display");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    logger.LogInfo("init Display Failed");
    B_SSD1306 = false;
  }
  else
  {

    B_SSD1306 = true;
    delay(250);

    sprintf(displayInfo.title, "RaceLap  0.5");

    logger.LogInfo("init Display ok");
  }
}

void showDisplay()
{
  // TRACE("showDisplay");
  if (B_SSD1306)
  {

    display.clearDisplay();
    display.setTextColor(WHITE);

    if (KMPH - RecordKmph > 0)
    {
      display.setTextSize(7);
      display.setCursor(20, 0);
      display.println((int)KMPH);
    }
    else
    {
      display.setTextSize(1);
      display.setCursor(0, 0);

      display.print(displayInfo.title);
      display.print(F(" "));
      display.print(RaceStatus);
      display.print(F(" "));
      display.println(last_satellites);
      if (strcmp(buffer, "") != 0)
      {

        display.setCursor(0, 8);
        display.println(buffer);
      }

      if (strcmp(displayInfo.log, "") != 0)
      {
        display.setCursor(0, 48);
        display.println(displayInfo.log);
      }
    }

    // display.setTextSize(2);
    // display.setCursor(50, 48);
    // display.println(displayInfo.speed);
    display.display();
  }
  // displayInfo.logo = "RaceLap verion 0.01";
}

void initGps()
{
  logger.LogInfo("init GPS");

  // ss.begin(9600);
  ss.begin(57600);
  delay(250);

  while (!ss.available())
  {
    logger.LogInfo("GPS invaild");
  }
  logger.LogInfo("GPS init OK");
}

void recordGps()
{
  //
  // power 45 second no record
  if ((millis() - StartTime) < 30 * 1000)
  {
    RaceStatus = _Starting;
    sprintf(displayInfo.log, "%ld", 30 - (millis() - StartTime) / 1000);
    return;
  }

  // sleep cd time
  if (RaceStatus == _RecordToSleep)
  {
    int sleep_t = (millis() - lastSpeedLimitTime) - SleepCD * 1000;
    // logger.LogInfo("..%u..%d..%T..%d", (millis() - lastSpeedLimitTime), (millis() - lastSpeedLimitTime) - SleepCD * 1000, (millis() - lastSpeedLimitTime) - SleepCD * 1000 > 0, sleep_t);
    if (sleep_t > 0)
    {

      if (dataFile)
      {
        snprintf(logbuff, sizeof(logbuff), "close DataFileName %s", DataFileName);
        logger.LogInfo(logbuff);
        dataFile.close();
        strcpy(DataFileName, "");
      }

      sprintf(displayInfo.log, "Sleep ,speed=%0.2f", KMPH);

      RaceStatus = _Sleep;
      logger.LogInfo("CD ok,form _RecordToSleep to _Sleep");
    }
    else
    {

      // logger.LogInfo("speed = %D cd=%d", KMPH, (millis() - lastSpeedLimitTime) / 1000);

      sprintf(displayInfo.log, "speed = %0.2f cd= %ld", KMPH, (millis() - lastSpeedLimitTime) / 1000);
    }
  }

  if (!gps.location.isValid())
  {
    sprintf(displayInfo.log, "GPS notValid");
    KMPH = 0;
    if (RaceStatus == _Recording)
    {

      RaceStatus = _RecordToSleep;
      logger.LogInfo("GPS not valid form _Recording to _RecordToSleep");
    }
    // return;
  }

  // logger.LogInfo("recordGps %T %T %T", gps.location.isValid(), gps.location.isUpdated()), gps.satellites.isValid();
  if (gps.satellites.isValid() && last_satellites != gps.satellites.value())
  {
    last_satellites = gps.satellites.value();
    // snprintf(logbuff, sizeof(logbuff), "satellites=%d", gps.satellites.value());
    // logger.LogInfo(logbuff);
  }

  //
  if (gps.location.isUpdated())
  // if (gps.location.isValid())
  {
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

    snprintf(buffer, sizeof(buffer),
             "%d%02d%02d%02d%02d%02d%03d,%.8f,%.8f,%.2f,%.2f,%lu",
             year,
             month, day, hour, minute, second, csecond, lat, lng, altitude, KMPH, millis());

    // logger.LogInfo(buffer);

    // gps init datetime not correct
    if (year < 2022)
    {
      return;
    }
    /*

    if (lastStartTime == 0)
    {
      lastStartTime = millis();
    }

    long diff = millis() - lastStartTime;

    if (diff >= 0 && diff < 20000)
    {
      KMPH = 21.0;
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
      lastStartTime = millis();
    }

    if (lastkmph != KMPH)
    {
      logger.LogInfo("kmph from %D to %D", lastkmph, KMPH);
      lastkmph = KMPH;
    }
*/
    if (B_SD)
    {

      if (KMPH - RecordKmph > 0)
      {
        if (RaceStatus != _Recording)
        {
          snprintf(logbuff, sizeof(logbuff), "from %d into _Recording", RaceStatus);
          logger.LogInfo(logbuff);
          RaceStatus = _Recording;
        }

        lastSpeedLimitTime = millis();

        if (!dataFile)
        {
          if (strcmp(DataFileName, "") == 0)
          {
            sprintf(DataFileName, "%sRL%04d%02d%02d%02d%02d%02d.txt", DataFileDir, year, month, day, hour, minute, second);
          }
          snprintf(logbuff, sizeof(logbuff), "new DataFileName recording %s", DataFileName);
          logger.LogInfo(logbuff);
          dataFile = SD.open(DataFileName, FILE_WRITE);
        }
        if (dataFile)
        {
          dataFile.println(buffer);
          dataFile.flush();
        }

        // logger.LogInfo(buffer);
      }
      else
      {

        if (RaceStatus == _Recording)
        {
          RaceStatus = _RecordToSleep;
          logger.LogInfo("speed <limit form _RecordToSleep to _Sleep");
        }
        if (dataFile && RaceStatus == _RecordToSleep)
        {
          dataFile.println(buffer);
          dataFile.flush();
        }
      }
    }
  }
}

void initLed()
{
  pinMode(LED, OUTPUT);
}

void setup()
{

  RaceStatus = _Setup;
  Serial.begin(9600);
  initSD();

  initLed();

  // Serial.setDebugOutput(false);

  // put your setup code here, to run once:
  logger.LogInfo("setup...");
  logger.LogInfo("Mounting the filesystem...");
  if (!LittleFS.begin())
  {
    logger.LogInfo("could not mount the filesystem...");
    delay(2000);
    ESP.restart();
  }

  initDisplay();

  initWifi();
  initWebServer();
  initGps();

  StartTime = millis();
  logger.LogInfo("init ok\n");
}

void loop()
{
  digitalWrite(LED, (millis() / 1000) % 2);

  if (millis() - lastDisplayTime > 1000)
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
