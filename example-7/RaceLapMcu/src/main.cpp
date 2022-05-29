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

#define LED D0 // Led in NodeMCU at pin GPIO16 (D0).

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C

TinyGPSPlus gps;
// gps_fix fix;

File root;
File dataFile;
File trackfile;

String tree = "";

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
unsigned long lastSpeedZeroTime = 0;

unsigned long StartTime = 0;
char buffer[100];
double RecordKmph = 20;
double KMPH = 0;

double lastkmph = 0;

double TRACK_LAT = 0, TRACK_LNG = 0;
char TRACKNAME[30];

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

bool readLine(File &f, char *line, size_t maxLen)
{
  for (size_t n = 0; n < maxLen; n++)
  {
    int c = f.read();
    if (c < 0 && n == 0)
      return false; // EOF
    if (c < 0 || c == '\n')
    {
      line[n] = 0;
      return true;
    }
    line[n] = c;
  }
  return false; // line too long
}

int csvReadText(File *file, char *str, size_t size, char delim)
{
  uint8_t ch;
  int rtn;
  size_t n = 0;
  while (true)
  {
    // check for EOF
    if (!file->available())
    {
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1)
    {
      // read error
      rtn = -1;
      break;
    }
    // Delete CR.
    if (ch == '\r')
    {
      continue;
    }
    if (ch == delim || ch == '\n')
    {
      rtn = ch;
      break;
    }
    if ((n + 1) >= size)
    {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';

  Log.traceln("readText %s", str);
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadInt32(File *file, int32_t *num, char delim)
{
  char buf[20];
  char *ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0)
    return rtn;
  *num = strtol(buf, &ptr, 10);
  if (buf == ptr)
    return -3;
  while (isspace(*ptr))
    ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadInt16(File *file, int16_t *num, char delim)
{
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0)
    return rtn;
  if (tmp < INT_MIN || tmp > INT_MAX)
    return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadUint32(File *file, uint32_t *num, char delim)
{
  char buf[20];
  char *ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0)
    return rtn;
  *num = strtoul(buf, &ptr, 10);
  if (buf == ptr)
    return -3;
  while (isspace(*ptr))
    ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadUint16(File *file, uint16_t *num, char delim)
{
  uint32_t tmp;
  int rtn = csvReadUint32(file, &tmp, delim);
  if (rtn < 0)
    return rtn;
  if (tmp > UINT_MAX)
    return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadDouble(File *file, double *num, char delim)
{
  char buf[20];
  char *ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0)
    return rtn;
  *num = strtod(buf, &ptr);
  if (buf == ptr)
    return -3;
  while (isspace(*ptr))
    ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadFloat(File *file, float *num, char delim)
{
  double tmp;
  int rtn = csvReadDouble(file, &tmp, delim);
  if (rtn < 0)
    return rtn;
  // could test for too large.
  *num = tmp;
  return rtn;
}

void ListDirectoryJSON(File dir)
{

  // dir = SD.open("/RLDATA");
  String entryName = "";

  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }

    entryName = entry.name();
    if (entry.isDirectory())
    {

      ListDirectoryJSON(entry);
    }
    else
    {

      tree = tree + "/" + dir.name() + "/" + entry.name() + ",";
    }
    entry.close();
  }
}

void ListDirectory(File dir)
{

  // dir = SD.open("/RLDATA");
  String entryName = "";

  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }

    entryName = entry.name();
    if (entry.isDirectory())
    {
      tree += F("<tr>");
      tree += F("<td>");
      tree += entryName;
      tree += F("/</td><td></td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">-</td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">");
      tree += F("<button class='buttons' onclick=\"location.href='/del?dir=/");
      tree = tree + dir.name() + "/" + entry.name();
      tree += F("';\">del</button></td>");
      tree += F("</tr>");
      ListDirectory(entry);
    }
    else
    {
      tree += F("<tr><td></td>");
      tree += F("<td data-value=\"");
      tree += entry.name();
      tree += F("\">");
      tree += entryName;
      tree += F("</a></td>");
      tree += F("<td class=\"detailsColumn\">");
      tree += file_size(entry.size());
      tree += F("</td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">");
      tree += F("<button class='buttons' onclick=\"location.href='/down?file=/");
      tree = tree + dir.name() + "/" + entry.name();
      tree += F("'\">down</button></td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">");
      tree += F("<button class='buttons' onclick=\"location.href='/del?file=/");
      tree = tree + dir.name() + "/" + entry.name();
      tree += F("'\">del</button></td>");
      tree += F("</tr>");
    }
    entry.close();
  }
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void handleGetLocation(AsyncWebServerRequest *request)
{
  char buf[64];

  if (gps.location.isValid())
  {

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<200> doc;

    JsonObject e = doc.createNestedObject("e");
    e["code"] = 1;
    JsonObject data = doc.createNestedObject("data");
    data["lat1"] = gps.location.lat();
    data["lng1"] = gps.location.lng();

    serializeJson(doc, *response);
    request->send(response);
  }
  else
  {

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<200> doc;

    JsonObject e = doc.createNestedObject("e");
    e["code"] = 1;
    e["msg"] = "Gps not valid";

    serializeJson(doc, *response);
    request->send(response);
  }
}

void handleSetLocation(AsyncWebServerRequest *request)
{
  char buf[64];

  if (request->hasParam("start"))
  {
    if (gps.location.isValid())
    {

      sprintf(buf, "%.8f,%.8f,track", gps.location.lat(), gps.location.lng());

      SD.remove("/RLDATA/track.txt");
      trackfile = SD.open("/RLDATA/track.txt", FILE_WRITE);
      trackfile.println(buf);
      trackfile.close();
      sprintf(buf, "{e:{code:1},data:{lat:%.8f,lng:%.8f}}", gps.location.lat(), gps.location.lng());
      request->send(200, "text/plain", buf);

      AsyncResponseStream *response = request->beginResponseStream("application/json");
      StaticJsonDocument<200> doc;

      JsonObject e = doc.createNestedObject("e");
      e["code"] = 1;
      JsonObject data = doc.createNestedObject("data");
      data["lat1"] = gps.location.lat();
      data["lng1"] = gps.location.lng();

      serializeJson(doc, *response);
      request->send(response);
    }
    else
    {
      request->send(200, "text/plain", "{e:{code:-1,msg:'Gps not valid'}}");
    }
  }
  else
  {
    request->send(200, "text/plain", "{e:{code:-1,msg:'params error'}}");
  }
}

void initWifi()
{

  Log.traceln("initWifi");

  const char *ssid = "RaceLap";      // Enter SSID here
  const char *password = "88888888"; // Enter Password here
  WiFi.softAP(ssid, password);
  delay(250);
  Log.trace("Soft-AP IP address = ");
  Log.traceln(WiFi.softAPIP());

  // Serial.print("DHCP status:");
  // Serial.println(wifi_softap_dhcps_status());

  // IPAddress local_ip(192, 168, 4, 1);
  // IPAddress gateway(192, 168, 4, 1);
  // IPAddress subnet(255, 255, 255, 0);

  // WiFi.softAPConfig(local_ip, gateway, subnet);
  // delay(50);

  // WiFi.begin("yunweizu", "yunweizubangbangda");
  // WiFi.begin("wifi-acans", "85750218");
  // Log.traceln("Connecting");
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Log.traceln(".");
  // }

  // Log.trace("Connected, IP address: ");
  // Log.traceln(WiFi.localIP());
}

void initWebServer()
{

  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send(200, "text/plain", "Hello, world"); });

  Log.traceln("initWebServer");
  server.onNotFound(notFound);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
  if (request->method() == HTTP_OPTIONS) {
    request->send(200);
  } else {
    request->send(404);
  } });
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.on("/listsd", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              tree="<table>";
              root = SD.open("/RLDATA");
              ListDirectory(root);
            tree+="</table>";
              request->send(200, "text/plain", tree); });

  server.on("/listsdjson", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              tree = "";
              root = SD.open("/RLDATA");
              ListDirectoryJSON(root);


              request->send(200, "text/plain", tree); });

  server.on("/down", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("file"))
              {
                String message = request->getParam("file")->value();
                Log.traceln("downfile %s",message.c_str());
                //request->send(200, "text/plain", "Params ok");
               // root = SD.open("/RLDATA");
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
      Log.traceln("delfile %s", message.c_str());
      // request->send(200, "text/plain", "Params ok");
      // root = SD.open("/RLDATA");
      SD.remove(message);

      request->send(200, "text/plain", "file del  ok ");
    }
    else if (request->hasParam("dir"))
    {
      String message =  request->getParam("dir")->value();
      Log.traceln("deldir %s", message.c_str());

      SD.rmdir(message);
      request->send(200, "text/plain", "dir del  ok ");

    }else
      {
        request->send(200, "text/plain", "Params error");
      } });

  server.on("/getlocation", HTTP_GET, [](AsyncWebServerRequest *request)
            {
           
          
             char buf[64]="";
             
             trackfile = SD.open("/RLDATA/track.txt", FILE_READ);
              if (trackfile){
                if (!readLine(trackfile, buf, sizeof(buf)))
                {
                // request->send(200, "text/plain", "");
                  Log.traceln("tract.txt readline error");
                }
               trackfile.close();
               Log.traceln("tract.txt %s",buf);
            
        
              request->send(200, "text/plain", buf);
            }else{
              request->send(200, "text/plain", "");
            } });

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
  Log.traceln("init SD Card");
  // while (!SD.begin(chipSelect))
  // {
  //   TRACE("SD Card Failed, Will Retry...");
  // }
  if (!SD.begin(chipSelect))
  {
    Log.traceln("init SD Card Failed");
    B_SD = false;
  }
  else
  {
    B_SD = true;

    if (SD.mkdir("RLDATA"))
    {
      Log.traceln("dir is created.");
    }

    root = SD.open("/RLDATA");

    printDirectory(root, 0);

    Log.traceln("print RLDATA Directory done!");

    // read track.txt
    trackfile = SD.open("/RLDATA/track.txt", FILE_READ);

    while (trackfile.available())
    {
      if (csvReadDouble(&trackfile, &TRACK_LAT, ',') != ',' || csvReadDouble(&trackfile, &TRACK_LNG, ',') != ',' || csvReadText(&trackfile, TRACKNAME, sizeof(TRACKNAME), ',') != '\n')
      {
        Log.traceln("track.txt read error");
      }

      Log.traceln("track.txt %D %D %s", TRACK_LAT, TRACK_LNG, TRACKNAME);
    }
    trackfile.close();
  }
}

void initDisplay()
{
  Log.traceln("init Display");
  Wire.begin();

  Log.traceln("wire %T", Wire.status());
  //  Log.traceln("wire %T", Wire.available());

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Log.traceln("init Display Failed");
    B_SSD1306 = false;
  }
  else
  {

    B_SSD1306 = true;
    delay(250);

    sprintf(displayInfo.title, "RaceLap  0.4");

    Log.traceln("init Display ok");
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
      display.setCursor(30, 0);
      display.println((int)KMPH);
    }
    else
    {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println(displayInfo.title);
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
  Log.traceln("init GPS");

  // ss.begin(9600);
  ss.begin(57600);
  delay(250);

  while (!ss.available())
  {
    Log.traceln("GPS invaild");
  }
  Log.traceln("GPS init OK");
}

void recordGps()
{
  //
  // power 45 second no record
  if ((millis() - StartTime) < 45 * 1000)
  {
    sprintf(displayInfo.log, "%ld", 45 - (millis() - StartTime) / 1000);
    return;
  }

  if (!gps.location.isValid())
  {
    sprintf(displayInfo.log, "GPS isValid False");
    return;
  }
  // Log.traceln("recordGps %T %T %T", gps.location.isValid(), gps.location.isUpdated()), gps.satellites.isValid();
  if (gps.satellites.isValid() && last_satellites != gps.satellites.value())
  {
    last_satellites = gps.satellites.value();
    // Log.traceln("satellites=%d", gps.satellites.value());
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

    double distanceMToTrack = 0;
    if (TRACK_LAT != 0)
    {
      distanceMToTrack =
          TinyGPSPlus::distanceBetween(
              lat,
              lng,
              TRACK_LAT,
              TRACK_LNG);
    }

    // snprintf(buffer, sizeof(buffer),
    //              "Latitude: %.8f, Longitude: %.8f, Altitude: %.2f m, "
    //              "Date/Time: %d-%02d-%02d %02d:%02d:%02d",
    //              lat, lng, altitude, year, month, day, hour, minute, second);

    snprintf(buffer, sizeof(buffer),
             "%d-%02d-%02d%02d:%02d:%02d.%03d,%.8f,%.8f,%.2f,%.2f,%9.7f",
             year,
             month, day, hour, minute, second, csecond, lat, lng, altitude, KMPH, distanceMToTrack);

    // Log.traceln(buffer);

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

        if (diff > 0 && diff < 20000)
        {
          KMPH = 21.0;
        }
        else if (diff > 20000 && diff < 35000)
        {
          KMPH = 0.5;
        }
        else if (diff > 35000 && diff < 50000)
        {
          KMPH = 30;
        }
        else if (diff > 50000 && diff < 65000)
        {
          KMPH = 7;
        }
        else if (diff > 65000)
        {
          KMPH = 5;
        }

        if (lastkmph != KMPH)
        {
          Log.traceln("kmph %D %D", KMPH, lastkmph);
          lastkmph = KMPH;
        }
    */
    if (B_SD)
    {

      if (KMPH - RecordKmph > 0)
      {

        lastSpeedLimitTime = millis();
        if (strcmp(DataFileName, "") == 0)
        {
          sprintf(DataFileName, "%sRL%04d%02d%02d%02d%02d%02d.txt", DataFileDir, year, month, day, hour, minute, second);
          Log.traceln("new DataFileName %s", DataFileName);
        }
        if (!dataFile)
        {
          dataFile = SD.open(DataFileName, FILE_WRITE);
        }

        dataFile.println(buffer);
        dataFile.flush();
        Log.traceln(buffer);
      }
      else
      {
        if ((millis() - lastSpeedLimitTime) > 30 * 1000)
        {

          if (dataFile)
          {
            Log.traceln("close DataFileName %s", DataFileName);
            dataFile.close();
            strcpy(DataFileName, "");
          }
          else
          {
            Log.traceln("Sleep ,speed= %D", KMPH);

            sprintf(displayInfo.log, "Sleep ,speed=%0.2f", KMPH);
          }

          // lastSpeedLimitTime = 0;
        }
        else
        {
          if (dataFile)
          {
            dataFile.println(buffer);
            dataFile.flush();
            // Log.traceln(buffer);
          }
          Log.traceln("speed = %D cd=%d", KMPH, (millis() - lastSpeedLimitTime) / 1000);

          sprintf(displayInfo.log, "speed = %0.2f cd= %ld", KMPH, (millis() - lastSpeedLimitTime) / 1000);
        }
      }
    }
  }
}

// void GpsLoop()
// {

//   while (ss.available() > 0)
//   {
//     if (gps.encode(ss.read()))
//     {
//       printData();
//       recordGps();
//     }
//   }
// }

void initLed()
{
  pinMode(LED, OUTPUT);
}

void setup()
{
  Serial.begin(9600);
  initLed();
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  // Serial.setDebugOutput(false);

  // put your setup code here, to run once:
  Log.traceln("setup...");
  Log.traceln("Mounting the filesystem...\n");
  if (!LittleFS.begin())
  {
    Log.traceln("could not mount the filesystem...\n");
    delay(2000);
    ESP.restart();
  }

  initDisplay();
  initSD();
  initWifi();
  initWebServer();
  initGps();

  StartTime = millis();
  Log.traceln("init ok\n");
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
