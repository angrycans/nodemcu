#ifndef webserver_helper_HPP
#define webserver_helper_HPP

AsyncWebServer server(80);

String file_size(int bytes);
String ListDirectoryJSON();
void getTrack();
void notFound(AsyncWebServerRequest *request);
void handleGetMcuCfg(AsyncWebServerRequest *request);
void handleSysinfo(AsyncWebServerRequest *request);
void handleGetLocation(AsyncWebServerRequest *request);
void handleSetSpeed(AsyncWebServerRequest *request);

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
  // data["RaceStatusEnum"] = "0 _Setup 1_preRecord 2_Recording 3_RecordToSleep 4_Sleep";
  // data["RAM"] = ESP.getFreeHeap();
  // data["FreeHeap"] = ESP.getMaxFreeBlockSize();
  // data["Vcc"] = ESP.getVcc() / 1024.0;
  // data["Version"] = ESP.getSdkVersion();
  // data["CPU"] = ESP.getCpuFreqMHz();
  // data["ResetReason"] = ESP.getResetReason();

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
  params["trackname"] = request->getParam("trackname")->value();

  JsonArray trackplan = params.createNestedArray("trackplan");

  StaticJsonDocument<JSON_ARRAY_SIZE(50)> arraydoc;
  JsonArray item = arraydoc.to<JsonArray>();
  item.add(request->getParam("lat1")->value());
  item.add(request->getParam("lng1")->value());
  item.add(request->getParam("lat2")->value());
  item.add(request->getParam("lng2")->value());
  // item.add(1111111111.0009);
  // item.add(1111111111.0006);
  // item.add(1111111111.0007);
  // item.add(1111111111.0008);
  trackplan.add(item);

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  StaticJsonDocument<200> doc;

  JsonObject e = doc.createNestedObject("e");

  SD.remove("/RLDATA/track.json");

  File file = SD.open("/RLDATA/track.json", FILE_WRITE);
  if (!file)
  {
    e["code"] = 1;
    e["msg"] = "file open error";
    serializeJson(doc, *response);
    request->send(response);
    file.close();
    return;
  }

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

    // setFinishLinePos();
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
    snprintf(logbuff, sizeof(logbuff), "[%s]SetSpeed %d %f", formatTime(millis()), speed, RecordKmph);
    logger.LogInfo(logbuff);
#endif
    serializeJson(doc, *response);
    request->send(response);
  }
}

void getTrack()
{
  File file = SD.open("/RLDATA/track.json");

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (!error)
  {
// String output;
// serializeJson(doc, output);
// logger.LogInfo("track.txt " + output);

// race.lat1 = atof(doc["lat1"]);
// race.lng1 = atof(doc["lng1"]);
// race.lat2 = atof(doc["lat2"]);
// race.lng2 = atof(doc["lng2"]);

// var trackplan = [
//   [ 31.934493, 118.986260, 31.934659, 118.986156 ],
//   [ 31.935279, 118.986374, 31.935097, 118.986298 ],
//   [ 31.934911, 118.985879, 31.934865, 118.986073 ],
//   [ 31.934722, 118.985327, 31.934918, 118.985345 ],
//   [ 31.935798, 118.986160, 31.935994, 118.986176 ],
//   [ 31.935468, 118.986605, 31.935468, 118.986803 ],
//   [ 31.934884, 118.987121, 31.934764, 118.987279 ],
//   [ 31.934679, 118.986682, 31.934449, 118.986870 ]
// ]

// // char tmp[1000];
// Serial.println();
// serializeJson(doc, Serial);
// Serial.println();
#if defined(DEBUG)
    // snprintf(logbuff, sizeof(logbuff), "No GPS data received");
    String output;
    serializeJson(doc, output);
    logger.LogInfo(output);
#endif

    JsonArray array = doc["trackplan"];

    int size = array.size();

    double trackplan[size][4];

    for (int i = 0; i < size; i++)
    {

      JsonArray arr = array[i];

      trackplan[i][0] = arr[0].as<double>();

      trackplan[i][1] = arr[1].as<double>();
      trackplan[i][2] = arr[2].as<double>();
      trackplan[i][3] = arr[3].as<double>();

      // char tmp[100];
      // sprintf(tmp, "%d %.8f %.8f %.8f %.8f", i, trackplan[i][0], trackplan[i][1], trackplan[i][2], trackplan[i][3], trackplan[i][3]);
      // logger.LogInfo(tmp);
    }

    race.setTrace(trackplan, size);

    // char tmp[100];
    // sprintf(tmp, "finishline %.8f %.8f %.8f %.8f", race.lat1, race.lng1, race.lat2, race.lng2);
    // logger.LogInfo(tmp);
  }

  file.close();
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

#endif