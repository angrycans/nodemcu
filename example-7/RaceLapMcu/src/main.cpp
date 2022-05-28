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

char DataFileName[48] = "RL2022-05-18_14.txt";
char DataFileDir[24] = "RL20220518";

bool B_SD = false;      // SD Card status
bool B_SSD1306 = false; // 0.96 OLED display status

// SD Reader CS pin
const int chipSelect = 15;

uint32_t last_satellites = 0;

AsyncWebServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SoftwareSerial ss(D3, D4); // RX, TX

static unsigned long lastTime = 0;

static unsigned long lastSpeedTime = 0;
static unsigned long lastStartTime = 0;
char buffer[100];

struct TDisplayInfo
{
  char title[30];
  char gps[100];
  char time[32];
  char log[100];
  char speed[10];
};

typedef struct TDisplayInfo DisplayInfo;

DisplayInfo displayInfo;

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
      tree += F("<button class='buttons' onclick=\"location.href='/del?file=");
      tree += entry.name();
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

void initWifi()
{

  Log.traceln("initWifi");
  // WiFi.mode(WIFI_AP);
  // const char *ssid = "RaceLap";      // Enter SSID here
  // const char *password = "88888888"; // Enter Password here
  // WiFi.softAP(ssid, password);
  // delay(50);
  // Log.trace("Soft-AP IP address = ");
  // Log.traceln(WiFi.softAPIP());

  // Serial.print("DHCP status:");
  // Serial.println(wifi_softap_dhcps_status());

  // IPAddress local_ip(192, 168, 4, 1);
  // IPAddress gateway(192, 168, 4, 1);
  // IPAddress subnet(255, 255, 255, 0);

  // WiFi.softAPConfig(local_ip, gateway, subnet);
  // delay(50);

  WiFi.begin("yunweizu", "yunweizubangbangda");

  Log.traceln("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Log.traceln(".");
  }

  Log.trace("Connected, IP address: ");
  Log.traceln(WiFi.localIP());
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
                String message = "/RLDATA"+request->getParam("file")->value();
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
                String message = "/RLDATA"+request->getParam("file")->value();
                Log.traceln("delfile %s",message.c_str());
                //request->send(200, "text/plain", "Params ok");
               // root = SD.open("/RLDATA");
                SD.remove(message);

               
                  request->send(200, "text/plain", "file del  ok ");
              
              }else{
                request->send(200, "text/plain", "Params error");
              } });

  server.on("/getlocation", HTTP_GET, [](AsyncWebServerRequest *request)
            {
            if (gps.location.isValid())
            {
              

             char buf[64];
             sprintf(buf,"%.8f,%.8f",gps.location.lat(),gps.location.lng());

             SD.remove("/RLDATA/track.txt");
              trackfile = SD.open("/RLDATA/track.txt", FILE_WRITE);
              trackfile.println(buf);
              trackfile.close();
              sprintf(buf,"{e:{code:1},data:{lat:%.8f,lng:%.8f}}",gps.location.lat(),gps.location.lng());

        
              request->send(200, "text/plain", buf);
            }
            else
            {
              request->send(200, "text/plain", "{e:{code:-1,msg:'Gps not valid'}}");
            } });

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

    // if (!Wire.status())
    // {
    //   B_SSD1306 = false;
    //   return;
    // }

    delay(250);

    // // Clear the buffer.
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    sprintf(displayInfo.title, "RaceLap verion 0.1");
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

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(displayInfo.title);
    display.setCursor(0, 8);
    display.println(buffer);

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
  // Log.traceln("recordGps %T %T %T", gps.location.isValid(), gps.location.isUpdated()), gps.satellites.isValid();
  if (gps.satellites.isValid() && last_satellites != gps.satellites.value())
  {
    last_satellites = gps.satellites.value();
    Log.traceln("satellites=%d", gps.satellites.value());
  }
  //
  if (gps.location.isUpdated())
  // if (gps.location.isValid())
  {
    double lat = gps.location.lat();
    double lng = gps.location.lng();

    double altitude = gps.altitude.meters();

    double kmph = gps.speed.kmph();

    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();

    int hour = gps.time.hour();
    int minute = gps.time.minute();
    int second = gps.time.second();
    int csecond = gps.time.centisecond();

    // snprintf(buffer, sizeof(buffer),
    //              "Latitude: %.8f, Longitude: %.8f, Altitude: %.2f m, "
    //              "Date/Time: %d-%02d-%02d %02d:%02d:%02d",
    //              lat, lng, altitude, year, month, day, hour, minute, second);

    snprintf(buffer, sizeof(buffer),
             "%d-%02d-%02d%02d:%02d:%02d.%03d,%.8f,%.8f,%.2f,%.2f",
             year,
             month, day, hour, minute, second, csecond, lat, lng, altitude, kmph);

    Log.traceln(buffer);

    if (B_SD)
    {
      sprintf(DataFileDir, "/RLDATA/%04d%02d%02d/", year, month, day);
      if (!SD.exists(DataFileDir))
      {
        SD.mkdir(DataFileDir);
        Log.traceln("create dir %s", DataFileDir);
      }

      sprintf(DataFileName, "%sRL%04d%02d%02d%02d.txt", DataFileDir, year, month, day, hour);
      // sprintf(DataFileName, "%sRL%04d%02d%02d.txt", DataFileDir, year, month, day);
      dataFile = SD.open(DataFileName, FILE_WRITE);
      dataFile.println(buffer);
      dataFile.flush();
      //  dataFile.println(buffer);
      // dataFile.close();
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
  Log.traceln("init ok\n");
}

void loop()
{

  digitalWrite(LED, (millis() / 1000) % 2);

  if (millis() - lastTime > 1000)
  {
    showDisplay();
    lastTime = millis();
  }

  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      recordGps();
    }
  }
}
