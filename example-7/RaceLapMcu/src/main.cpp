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

  // // Clear the buffer.
  // display.clearDisplay();

  // // Display Text
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  // display.setCursor(0, 0);
  // display.println("Hello world!");
  // display.setCursor(0, 16);
  // display.println("test");
  // display.display();
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
  TRACE("recordGps");
  //
  if (gps.location.isUpdated())
  {
    double lat = gps.location.lat();
    double lng = gps.location.lng();

    double altitude = gps.altitude.meters();

    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();

    int hour = gps.time.hour();
    int minute = gps.time.minute();
    int second = gps.time.second();

    snprintf(buffer, sizeof(buffer),
             "Latitude: %.8f, Longitude: %.8f, Altitude: %.2f m, "
             "Date/Time: %d-%02d-%02d %02d:%02d:%02d",
             lat, lng, altitude, year, month, day, hour, minute, second);

    Serial.println(buffer);

    sprintf(DataFileName_Last, "RL%04d-%02d-%02d_%02d.txt", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour());

    TRACE(DataFileName);
    TRACE(DataFileName_Last);

    // sprintf(Gpsbuff, "", fix.valid.locationfix.latitude(), fix.longitude(), fix.altitude());

    gps_str = "";
    gps_str += String(gps.date.year());
    gps_str += String(gps.date.month());
    gps_str += String(gps.date.day());
    gps_str += String(gps.time.value());

    gps_str += "," + String(gps.location.lat()) + "," + String(gps.location.lng());

    gps_str += "," + String(gps.altitude.meters());
    gps_str += "," + String(gps.speed.mph());

    // TRACE(gps.satellites.value());

    TRACE(gps_str);

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
      dataFile.println(gps_str);
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
  initLed();
  Serial.begin(9600);
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

  // initDisplay();
  initSD();
  // initwifi();
  initGps();
  TRACE("init ok\n");
}

void loop()
{

  TRACE("loop start");
  digitalWrite(LED, HIGH); // turn the LED off.(Note that LOW is the voltage level but actually
                           // the LED is on; this is because it is acive low on the ESP8266.
  delay(5000);             // wait for 1 second.
  TRACE("loop led 5 s");
  // digitalWrite(LED, LOW);  // turn the LED on.
  // delay(1000);             // wait for 1 second.
  server.handleClient();

  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      recordGps();
    }
  }
}
