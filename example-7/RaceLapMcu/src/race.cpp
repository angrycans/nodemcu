

#include "race.h"

Race::Race()
{
  r_status.status = d_Setup;
  r_status.timer = millis();
  lapInfoList = new ESPLinkedList<LapInfo>();
}

void Race::setStatus(DeviesStatus _status)
{
  r_status.status = _status;
  r_status.timer = millis();

  switch (_status)
  {
  case d_Recording:
    if (!sessionActive)
    {
      sessionActive = true;
      sessionTime = millis();
      totalLap = 0;
      maxspeed = 0;
      avespeed = 0;
      bestLap = 0;
      bestSessionTime = 0;
    }

    break;
  case d_Looping:

    break;

  default:
    break;
  }

  // if ()
}

RaceStatuS Race::getStatus()
{
  return r_status;
}

String Race::getStatusName()
{
  switch (r_status.status)
  {
  case d_Setup:
    return "setup";
    break;
  case d_Looping:
    return "Looping";
    break;
  case d_preRecord:
    return "preRecord";
    break;
  case d_Recording:
    return "Recording";
    break;
  case d_RecordToStop:
    return "RecordToStop";
    break;
  case d_Stop:
    return "Stop";
    break;

  default:
    return "Unknown";
    break;
  }
}

void Race::resetSession()
{
  sessionActive = false;

  sessionTime = 0;
  bestSessionTime = 0;

  currentLap = 0;
  bestLap = 0;
  totalLap = 0;
  maxspeed = 0;
}

void Race::setTrace(double arr[][4], int _size)
{
  trackplan_size = _size;
  trackplan = new double *[_size];

  for (int i = 0; i < trackplan_size; i++)
  {
    trackplan[i] = new double[4];
    trackplan[i][0] = arr[i][0];
    trackplan[i][1] = arr[i][1];
    trackplan[i][2] = arr[i][2];
    trackplan[i][3] = arr[i][3];
    // char tmp1[100];
    // sprintf(tmp1, "%d %.8lf %.8lf %.8lf %.8lf", i, trackplan[i][0], trackplan[i][1], trackplan[i][2], trackplan[i][3]);
    // Serial.println(tmp1);
  }
}

bool Race::nearTarck(float lat, float lon)
{

  return false;
  if (trackplan_size > 0)
  {

    unsigned long distanceKmToTarck =
        (unsigned long)TinyGPSPlus::distanceBetween(
            lat,
            lon,
            trackplan[trackplan_size - 1][0],
            trackplan[trackplan_size - 1][1]) /
        1000;

    if ((distanceKmToTarck - 5) < 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {

    return false;
  }
}

//计算圈速
void Race::computerSession(TinyGPSPlus *_gps)
{

  double speed = _gps->speed.kmph();

  //保存速度
  if (speed > maxspeed)
  {
    maxspeed = speed;
  }
  if (speed > lapmaxspeed)
  {
    lapmaxspeed = speed;
  }

  avespeed = (avespeed + speed) / 2;
  lapavespeed = (lapavespeed + speed) / 2;

  computerLapinfo(_gps);

  last_gps = *_gps;
}

void Race::computerLapinfo(TinyGPSPlus *_gps)
{
  // tmElements_t te1;
  // te1.Second = _gps->time.second();
  // te1.Hour = _gps->time.hour();
  // te1.Minute = _gps->time.minute();
  // te1.Day = _gps->date.day();
  // te1.Month = _gps->date.month();
  // te1.Year = _gps->date.year() - 1970; // Y2K, in seconds = 946684800UL

  // int timestamp = (int)(makeTime(te1) * 1000 + _gps->time.centisecond() * 10);

  float off = 0.0f;

  if (trackplan_size > 0)
  {
    if (segmentsIntersect(_gps->location.lat(), _gps->location.lng(), last_gps.location.lat(), last_gps.location.lng(), trackplan[trackplan_size - 1][0], trackplan[trackplan_size - 1][1], trackplan[trackplan_size - 1][2], trackplan[trackplan_size - 1][3]))
    {
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "segmentsIntersect checked");
      logger.LogInfo(logbuff);
#endif

      point_t cp;
      IntersectPoint1({_gps->location.lat(), _gps->location.lng()}, {last_gps.location.lat(), last_gps.location.lng()}, {trackplan[trackplan_size - 1][0], trackplan[trackplan_size - 1][1]}, {trackplan[trackplan_size - 1][2], trackplan[trackplan_size - 1][3]}, &cp);

      if (lapInfoList->size() > 0)
      {
        float d = Distance({_gps->location.lat(), _gps->location.lng()}, cp);
        off = (d * 1000 / _gps->speed.kmph()) * 60 * 60;
      }
      LapInfo lapinfo;
      lapinfo.time = millis();
      // lapinfo.time2 = timestamp;
      lapinfo.off = (int)off;
      lapinfo.difftime = lapInfoList->size() == 0 ? 0 : (int)(lapinfo.time - sessionTime);
      lapinfo.maxspeed = lapmaxspeed;
      lapinfo.avespeed = lapavespeed;
      // lapinfo.p0_at = makeTime(te0);
      // lapinfo.p1_at = makeTime(te1);
      // lapinfo.p0_at_ms = gps.time.centisecond();
      lapInfoList->add(lapinfo);

      currentLap += 1;
      totalLap += 1;

      // this is the best or first lap
      if ((bestSessionTime > lapinfo.time - sessionTime) || (bestSessionTime == 0))
      { // test for session time and also for the very first lap, when bestSesstionTime is 0
        bestSessionTime = lapinfo.time - sessionTime;
        bestLap = currentLap;
      }
#if defined(DEBUG)
      snprintf(logbuff, sizeof(logbuff), "[%s]currlap %d,totol %d,maxspeed:%f,bestlap:%d,bestsession:%lu", formatTime(millis()), currentLap, totalLap, maxspeed, bestLap, bestSessionTime);
      logger.LogInfo(logbuff);
      lapinfo.dump(&logger);
#endif

      // reset the session
      sessionTime = millis();
      lapmaxspeed = 0;
      lapavespeed = 0;
    }
  }
}

void Race::getTrackInfo()
{

  File file = SD.open("/RLDATA/track.json");

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (!error)
  {

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
    }

    setTrace(trackplan, size);

    const char *s = doc["trackname"];
    TrackName = s;
  }

  file.close();
}

String Race::getHeader(int year, int month, int day, int hour, int minute, int second)
{

  String header = F("<table>#V=");
  header += VERSION_FIRMWARE_NAME;
  header += F("=\n");
  header += F("#D=");
  header += DataFileName;
  header += F("=\n");
  header += F("#U==");
  header += UID;
  header += F("=\n");
  header += F("#N=");
  header += UNAME;
  header += F("=\n");
  header += F("#M=");
  header += UNAME;
  header += F("=\n");
  header += F("#H=");
  header += VERSION_HARDWARE;
  header += F("=\n");
  header += F("#B=");
  header += VERSION_SOFTWARE;
  header += F("=\n");
  header += F("#T=");
  header += TrackName;
  header += F("=\n");
  return header;
}