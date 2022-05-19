#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>       // File System for Web Server Files
#include <LittleFS.h> // This file system is used.

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define TRACE(...) Serial.printf(__VA_ARGS__)

ESP8266WebServer server(80);

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

void setup()
{
  Serial.begin(9600);
  Serial.setDebugOutput(false);

  // put your setup code here, to run once:
  TRACE("\nsetup...\n");
  TRACE("Mounting the filesystem...\n");
  if (!LittleFS.begin())
  {
    TRACE("could not mount the filesystem...\n");
    delay(2000);
    ESP.restart();
  }
  initwifi();
  TRACE("initwifi ok\n");
}

void loop()
{
  server.handleClient();
}
