#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>       // File System for Web Server Files
#include <LittleFS.h> // This file system is used.
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

#include <ArduinoLog.h>
#include <TinyGPS++.h>

#define LED D0 // Led in NodeMCU at pin GPIO16 (D0).

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C

#define TRACE(...) Serial.println(__VA_ARGS__)

TinyGPSPlus gps;
// gps_fix fix;

File dataFile;
char DataFileName[24] = "RL2022-05-18_14.txt";
char DataFileName_Last[24] = "RL2022-05-18_14.txt";
String gps_str;

bool B_SD = false;      // SD Card status
bool B_SSD1306 = false; // 0.96 OLED display status

// SD Reader CS pin
const int chipSelect = 15;

ESP8266WebServer server(80);
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

void handleRedirect()
{
  TRACE("Redirect...");
  String url = "/index.html";

  if (!LittleFS.exists(url))
  {
    server.send(404, "text/plain", "Not found");
  }
  else
  {
    server.sendHeader("Location", url, true);
    server.send(302);
  }

} // handleRedirect()

// This function is called when the WebServer was requested to list all existing files in the filesystem.
// a JSON array with file information is returned.
void handleListFiles()
{
  Dir dir = LittleFS.openDir("/");
  String result;

  result += "[\n";
  while (dir.next())
  {
    if (result.length() > 4)
    {
      result += ",";
    }
    result += "  {";
    result += " \"name\": \"" + dir.fileName() + "\", ";
    result += " \"size\": " + String(dir.fileSize()) + ", ";
    result += " \"time\": " + String(dir.fileTime());
    result += " }\n";
    // jc.addProperty("size", dir.fileSize());
  } // while
  result += "]";
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/javascript; charset=utf-8", result);
} // handleListFiles()

void handleSysInfo()
{
  String result;

  FSInfo fs_info;
  LittleFS.info(fs_info);

  result += "{\n";
  result += "  \"flashSize\": " + String(ESP.getFlashChipSize()) + ",\n";
  result += "  \"freeHeap\": " + String(ESP.getFreeHeap()) + ",\n";
  result += "  \"fsTotalBytes\": " + String(fs_info.totalBytes) + ",\n";
  result += "  \"fsUsedBytes\": " + String(fs_info.usedBytes) + ",\n";
  result += "}";

  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/javascript; charset=utf-8", result);
} // handleSysInfo()

void handle_NotFound()
{ // routine for missing webpage
  server.send(404, "text/plain", "Not found");
}

void initwifi()
{
  const char *ssid = "RaceLap";      // Enter SSID here
  const char *password = "88888888"; // Enter Password here

  // IPAddress local_ip(192, 168, 2, 1);
  // IPAddress gateway(192, 168, 2, 1);
  // IPAddress subnet(255, 255, 255, 0);

  WiFi.softAP(ssid, password);
  // WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(500);
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRedirect);

  server.onNotFound(handle_NotFound); // if webpage not found run handle_notfound routine

  server.on("/$sysinfo", HTTP_GET, handleSysInfo);

  server.on("/$list", HTTP_GET, handleListFiles);

  server.enableCORS(true);
  server.serveStatic("/", LittleFS, "/");

  server.begin();
}

void initSD()
{
  // // keep checking the SD reader for valid SD card/format
  TRACE("init SD Card\n");
  // while (!SD.begin(chipSelect))
  // {
  //   TRACE("SD Card Failed, Will Retry...");
  // }
  if (!SD.begin(chipSelect))
  {
    TRACE("init SD Card Failed");
    B_SD = false;
  }
  else
  {
    B_SD = true;
  }
}

void initDisplay()
{

  TRACE("init Display");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    TRACE("init Display Failed");
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
  // display.setCursor(0, 0);
  // display.println("Hello world!");
  // display.setCursor(0, 16);
  // display.println("test");
  // display.display();

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
    if (gps.location.isValid())
    {
      sprintf(displayInfo.gps, "lat:%.8f,lng:%.8f", gps.location.lat(), gps.location.lng());
    }
    else
    {
      sprintf(displayInfo.gps, "lat:NaN,lng:NaN");
    }

    // if (gps.date.isValid())
    // {
    //   int year = gps.date.year();
    //   int month = gps.date.month();
    //   int day = gps.date.day();

    //   int hour = gps.time.hour();
    //   int minute = gps.time.minute();
    //   int second = gps.time.second();
    //   sprintf(displayInfo.time,
    //           "%d/%02d/%02d %02d:%02d:%02d",
    //           year,
    //           month, day, hour, minute, second);
    // }
    // else
    // {
    //   sprintf(displayInfo.time, "Datetime INVALID");
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
  TRACE("init GPS");

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
             "%d-%02d-%02d %02d:%02d:%02d.%04d,%.8f, %.8f, %.2f,%.2f",
             year,
             month, day, hour, minute, second, csecond, lat, lng, altitude, kmph);

    // TRACE(buffer);

    sprintf(DataFileName_Last, "RL%04d-%02d-%02d_%02d.txt", year, month, day, hour);

    // TRACE(DataFileName);
    // TRACE(DataFileName_Last);

    // sprintf(Gpsbuff, "", fix.valid.locationfix.latitude(), fix.longitude(), fix.altitude());

    if (B_SD)
    {
      if (strcmp(DataFileName_Last, DataFileName) == 0)
      {
        if (!dataFile)
        {
          dataFile = SD.open(DataFileName, FILE_WRITE);
        }
      }
      else
      {
        sprintf(DataFileName, DataFileName_Last);
        dataFile.close();
        dataFile = SD.open(DataFileName, FILE_WRITE);
      }
      dataFile.println(buffer);
    }
  }
  else
  {

    // Log.traceln("gps.location.isUpdated()= %T", gps.location.isUpdated());
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
  initLed();
  Serial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  // Serial.setDebugOutput(false);

  // put your setup code here, to run once:
  TRACE("\nsetup...\n");
  TRACE("Mounting the filesystem...\n");
  if (!LittleFS.begin())
  {
    TRACE("could not mount the filesystem...\n");
    delay(2000);
    ESP.restart();
  }

  initDisplay();
  initSD();
  // initwifi();
  initGps();
  TRACE("init ok\n");
}

void loop()
{

  digitalWrite(LED, (millis() / 1000) % 2);
  server.handleClient();

  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      recordGps();
    }
  }
  showDisplay();
}
