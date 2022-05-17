#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>       // File System for Web Server Files
#include <LittleFS.h> // This file system is used.
#include "builtinfiles.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define TRACE(...) Serial.printf(__VA_ARGS__)

// mark parameters not used in example
#define UNUSED __attribute__((unused))

ESP8266WebServer server(80);

void handleRedirect()
{
  TRACE("Redirect...");
  String url = "/index.html";

  if (!LittleFS.exists(url))
  {
    url = "/$update.html";
  }

  server.sendHeader("Location", url, true);
  server.send(302);
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

// ===== Request Handler class used to answer more complex requests =====

// The FileServerHandler is registered to the web server to support DELETE and UPLOAD of files into the filesystem.
class FileServerHandler : public RequestHandler
{
public:
  // @brief Construct a new File Server Handler object
  // @param fs The file system to be used.
  // @param path Path to the root folder in the file system that is used for serving static data down and upload.
  // @param cache_header Cache Header to be used in replies.
  FileServerHandler()
  {
    TRACE("FileServerHandler is registered\n");
  }

  // @brief check incoming request. Can handle POST for uploads and DELETE.
  // @param requestMethod method of the http request line.
  // @param requestUri request ressource from the http request line.
  // @return true when method can be handled.
  bool canHandle(HTTPMethod requestMethod, const String UNUSED &_uri) override
  {
    return ((requestMethod == HTTP_POST) || (requestMethod == HTTP_DELETE));
  } // canHandle()

  bool canUpload(const String &uri) override
  {
    // only allow upload on root fs level.
    return (uri == "/");
  } // canUpload()

  bool handle(ESP8266WebServer &server, HTTPMethod requestMethod, const String &requestUri) override
  {
    // ensure that filename starts with '/'
    String fName = requestUri;
    if (!fName.startsWith("/"))
    {
      fName = "/" + fName;
    }

    if (requestMethod == HTTP_POST)
    {
      // all done in upload. no other forms.
    }
    else if (requestMethod == HTTP_DELETE)
    {
      if (LittleFS.exists(fName))
      {
        LittleFS.remove(fName);
      }
    } // if

    server.send(200); // all done.
    return (true);
  } // handle()

  // uploading process
  void upload(ESP8266WebServer UNUSED &server, const String UNUSED &_requestUri, HTTPUpload &upload) override
  {
    // ensure that filename starts with '/'
    String fName = upload.filename;
    if (!fName.startsWith("/"))
    {
      fName = "/" + fName;
    }

    if (upload.status == UPLOAD_FILE_START)
    {
      // Open the file
      if (LittleFS.exists(fName))
      {
        LittleFS.remove(fName);
      } // if
      _fsUploadFile = LittleFS.open(fName, "w");
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
      // Write received bytes
      if (_fsUploadFile)
      {
        _fsUploadFile.write(upload.buf, upload.currentSize);
      }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
      // Close the file
      if (_fsUploadFile)
      {
        _fsUploadFile.close();
      }
    } // if
  }   // upload()

protected:
  File _fsUploadFile;
};

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
  // serve a built-in htm page
  server.on("/$upload.html", []()
            { server.send(200, "text/html", FPSTR(uploadContent)); });

  server.on("/$list", HTTP_GET, handleListFiles);

  server.enableCORS(true);
  server.serveStatic("/", LittleFS, "/");

  server.addHandler(new FileServerHandler());

  server.begin();
}

void setup()
{
  Serial.begin(9600);
  Serial.setDebugOutput(false);

  // put your setup code here, to run once:
  TRACE("setup...");
  TRACE("Mounting the filesystem...\n");
  if (!LittleFS.begin())
  {
    TRACE("could not mount the filesystem...\n");
    delay(2000);
    ESP.restart();
  }
  initwifi();
  TRACE("initwifi ok");
}

void loop()
{
  server.handleClient();
}
