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

#define TRACE(...) Serial.println(__VA_ARGS__)

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

AsyncWebServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SoftwareSerial ss(D3, D4); // RX, TX

char buffer[100];

struct TDisplayInfo
{
  char title[30];
  char gps[100];
  char time[32];
  char log[100];
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
      tree += F("<td data-value=\"");
      tree += entryName;
      tree += F("/\"><a class=\"icon dir\" href=\"");
      tree += entry.name();
      tree += F("\">");
      tree += entryName;
      tree += F("/</a></td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">-</td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">");
      tree += F("<button class='buttons' onclick=\"location.href='");
      tree += entry.name();
      tree += F("';\">show</button></td>");
      tree += F("</tr>");
      ListDirectory(entry);
    }
    else
    {
      tree += F("<tr>");
      tree += F("<td data-value=\"");
      tree += entry.name();
      tree += F("\"><a class=\"icon file\" draggable=\"true\" href=\"");
      tree += entry.name();
      tree += F("\">");
      tree += dir.name() + "/" + entryName;
      tree += F("</a></td>");
      tree += F("<td class=\"detailsColumn\" data-value=\")");
      tree += file_size(entry.size());
      tree += F("\">");
      tree += file_size(entry.size());
      tree += F("</td>");
      tree += F("<td class=\"detailsColumn\" data-value=\"0\">");
      tree += F("<button class='buttons' onclick=\"location.href='/downfile?file=");
      tree += entry.name();
      tree += F("';\">down</button></td>");
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
  const char *ssid = "RaceLap";      // Enter SSID here
  const char *password = "88888888"; // Enter Password here

  // IPAddress local_ip(192, 168, 2, 1);
  // IPAddress gateway(192, 168, 2, 1);
  // IPAddress subnet(255, 255, 255, 0);

  WiFi.softAP(ssid, password);
  // WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(250);
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
            tree=+"</table>";
              tree="{data:"+tree+"}";
              request->send(200, "text/plain", tree); });

  server.on("/getlocation", HTTP_GET, [](AsyncWebServerRequest *request)
            {
            if (gps.location.isValid())
            {
              

             char buf[64];
             sprintf(buf,"%.8f,%.8f",gps.location.lat(),gps.location.lng());
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
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Log.traceln("init Display Failed");
    B_SSD1306 = false;
  }
  else
  {
    B_SSD1306 = true;

    // File dataFile = SD.open(DataFileName, FILE_WRITE);
  }
  delay(250);

  // // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  sprintf(displayInfo.title, "RaceLap verion 0.04");
}

void showDisplay()
{

  // TRACE("showDisplay");
  if (B_SSD1306)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(displayInfo.title);
    // if (gps.location.isValid())
    // {
    //   sprintf(displayInfo.gps, "lat:%.8f,lng:%.8f", gps.location.lat(), gps.location.lng());
    // }
    // else
    // {
    //   sprintf(displayInfo.gps, "lat:NaN,lng:NaN");
    // }

    display.setCursor(0, 8);
    display.println(buffer);
    display.setCursor(0, 32);
    // display.println(displayInfo.time);
    // display.setCursor(0, 48);
    display.println(displayInfo.log);
    display.display();
  }
  // displayInfo.logo = "RaceLap verion 0.01";
}

void initGps()
{
  Log.traceln("init GPS");

  ss.begin(9600);
  delay(250);

  // TRACE(ss.available());
  //  // gps.send_P(&ss, F("PMTK251,115200")); // set baud rate
  //  gps.send_P(&ss, F("PUBX,41,1,3,3,115200,0"));
  //  delay(250);
  //  ss.flush(); // wait for the command to go out

  // ss.end(); // empty the input buffer, too

  // delay(500); // wait for the GPS device to change speeds
  // ss.begin(9600);
  // ss.begin(115200); // use the new baud rate
  //  gps.send_P(&ss, F("PMTK220,100")); // set 10Hz update rate

  // TRACE("init GPS ok\n");
}

void recordGps()
{

  //
  if (gps.location.isUpdated())
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
             "%d-%02d-%02d %02d:%02d:%02d.%04d,%.8f,%.8f,%.2f,%.2f",
             year,
             month, day, hour, minute, second, csecond, lat, lng, altitude, kmph);

    sprintf(DataFileDir, "/RLDATA/%04d%02d%02d/", year, month, day);

    if (!SD.exists(DataFileDir))
    {
      SD.mkdir(DataFileDir);
      Log.traceln("create dir %s", DataFileDir);
    }

    sprintf(DataFileName, "%sRL%04d%02d%02d%02d.txt", DataFileDir, year, month, day, hour);

    if (B_SD)
    {
      dataFile = SD.open(DataFileName, FILE_WRITE);
      dataFile.println(buffer);
      dataFile.close();
    }
  }
  // else
  // {

  //   // Log.traceln("gps.location.isUpdated()= %T", gps.location.isUpdated());
  // }
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
  // dataFile = SD.open("DataFileName.txt", FILE_WRITE);
  // Log.traceln("gps.location.isUpdated()= %T %d", gps.location.isUpdated(), repeat++);
  // dataFile.print("buffer");
  // dataFile.println(repeat);
  // dataFile.close();

  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      recordGps();
    }
  }
  showDisplay();
}
