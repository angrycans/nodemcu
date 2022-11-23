#define D_TO_RADIANS (PI / 180.0f)
float EARTH_RADIUS = 3956.0; // In miles.

struct point_t
{
  double x, y;
};

// char *formatTime(unsigned long milli);
// String formatTime2(unsigned long milli);

char *formatTime(unsigned long milli)
{
  char *tmp = new char[10];
  // minutes
  int min = (milli / (1000 * 60)) % 60;

  // seconds
  int sec = (milli / 1000) % 60;

  // milliseconds
  int ms = milli % 1000;

  sprintf(tmp, "%02d:%02d.%03d", min, sec, ms);

  return tmp;
}

char *formatTimeMs(unsigned long milli)
{
  char *tmp = new char[15];

  // seconds
  int sec = (milli / 1000) % 60;

  String str = "";
  if (sec > 0)
  {
    str = String(sec);
  }

  // milliseconds
  int ms = milli % 1000;

  snprintf(tmp, 15, "%s.%03d", str.c_str(), ms);

  return tmp;
}

String formatTime2(unsigned long milli)
{
  // hours
  // int hr = (milli / (1000 * 60 * 60)) % 24;

  // minutes
  int min = (milli / (1000 * 60)) % 60;

  // seconds
  int sec = (milli / 1000) % 60;

  // milliseconds
  int ms = milli % 1000;

  char t[10];
  sprintf(t, "%02d:%02d.%03d", min, sec, ms);
  return t;
}

// char *dTime()
// {
//   char *tmp = new char[21];
//   unsigned long minutes = millis() / 60000;
//   unsigned long seconds = (millis() / 1000) - ((millis() / 60000) * 60);
//   unsigned long tenths = (millis() / 100) % 10;
//   sprintf(tmp, "%02lu:%02lu.%03lu", minutes, seconds, tenths);

//   return tmp;
// }

void IntersectPoint1(const point_t fl_p0, const point_t fl_p1, const point_t p1, const point_t p2, point_t *i)
{
  float denom, numera, mua; // numerb, mub;

  // No checks because it is assumed the lines intersect.
  denom = (fl_p1.y - fl_p0.y) * (p2.x - p1.x) - (fl_p1.x - fl_p0.x) * (p2.y - p1.y);
  numera = (fl_p1.x - fl_p0.x) * (p1.y - fl_p0.y) - (fl_p1.y - fl_p0.y) * (p1.x - fl_p0.x);
  // numerb = (p2.x - p1.x)*(p1.y - startingLine1.y) - (p2.y - p1.y)*(p1.x - startingLine1.x);

  mua = numera / denom;
  // mub = numerb/denom;
  i->x = p1.x + mua * (p2.x - p1.x);
  i->y = p1.y + mua * (p2.y - p1.y);
}

void IntersectPoint(float lat1, float lon1, float lat2, float lon2, float finishLineLat1, float finishLineLon1, float finishLineLat2, float finishLineLon2, point_t *i)
{
  float denom, numera, mua; // numerb, mub;

  // No checks because it is assumed the lines intersect.
  denom = (finishLineLat2 - finishLineLon1) * (lat2 - lat1) - (finishLineLat2 - finishLineLat1) * (lon2 - lon1);
  numera = (finishLineLat2 - finishLineLat1) * (lon1 - finishLineLon1) - (finishLineLat2 - finishLineLon1) * (lat1 - finishLineLat1);
  // numerb = (p2.x - p1.x)*(p1.y - startingLine1.y) - (p2.y - p1.y)*(p1.x - startingLine1.x);

  mua = numera / denom;
  // mub = numerb/denom;
  i->x = lat1 + mua * (lat2 - lat1);
  i->y = lon1 + mua * (lon2 - lon1);
}

/* static */
double distanceBetween(double lat1, double long1, double lat2, double long2)
{
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  double delta = radians(long1 - long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}

float Distance(const point_t t1, const point_t t2)
{
  float Lat1, Long1, Lat2, Long2; // Coordinates in degrees.
  float dlat, dlon;               // Change in location.
  float a, d;
  float c; // Great Circle distance (radians).

  Lat1 = (float)((uint32_t)(t1.y / 100.0f));
  Lat1 = (float)(Lat1 + (t1.y - Lat1 * 100.0f) / 60.0f);
  Lat1 = (float)(Lat1 * D_TO_RADIANS);

  Long1 = (float)((uint32_t)(t1.x / 100.0f));
  Long1 = (float)(Long1 + (t1.x - Long1 * 100.0f) / 60.0f);
  Long1 = (float)(Long1 * D_TO_RADIANS);

  Lat2 = (float)((uint32_t)(t2.y / 100.0f));
  Lat2 = (float)(Lat2 + (t2.y - Lat2 * 100.0f) / 60.0f);
  Lat2 = (float)(Lat2 * D_TO_RADIANS);

  Long2 = (float)((uint32_t)(t2.x / 100.0f));
  Long2 = (float)(Long2 + (t2.x - Long2 * 100.0f) / 60.0f);
  Long2 = (float)(Long2 * D_TO_RADIANS);

  dlat = Lat2 - Lat1;
  dlon = Long2 - Long1;

  a = (float)((sin(dlat / 2.0f) * sin(dlat / 2.0f)) + cos(Lat1) * cos(Lat2) * (sin(dlon / 2.0f) * sin(dlon / 2.0f)));
  c = (float)(2.0f * atan2(sqrt(a), sqrt(1.0f - a)));

  d = (float)(EARTH_RADIUS * c);

  return d;
}

boolean segmentsIntersect(float lat1, float lon1, float lat2, float lon2, float finishLineLat1, float finishLineLon1, float finishLineLat2, float finishLineLon2)
{
  // does line(p1, p2) intersect line(p3, p4)
  float fDeltaX = lat2 - lat1;
  float fDeltaY = lon2 - lon1;
  float da = finishLineLat2 - finishLineLat1;
  float db = finishLineLon2 - finishLineLon1;
  if ((da * fDeltaY - db * fDeltaX) == 0)
  {
    // The segments are parallel
    return false;
  }
  float s = (fDeltaX * (finishLineLon1 - lon1) + fDeltaY * (lat1 - finishLineLat1)) / (da * fDeltaY - db * fDeltaX);
  float t = (da * (lon1 - finishLineLon1) + db * (finishLineLat1 - lat1)) / (db * fDeltaX - da * fDeltaY);

  return (s >= 0) && (s <= 1) && (t >= 0) && (t <= 1);
}

// String formatSessionTime(unsigned long sessionTime)
// {
//   unsigned long minutes = sessionTime / 60000;
//   unsigned long seconds = (sessionTime / 1000) - ((sessionTime / 60000) * 60);
//   unsigned long tenths = (sessionTime / 100) % 10;
//   if (seconds < 10)
//     return String(minutes) + ":0" + String(seconds) + ":" + String(tenths);
//   else
//     return String(minutes) + ":" + String(seconds) + ":" + String(tenths);
// }

// String formatMs(unsigned long milli)
// {
//   // hours
//   // int hr = (milli / (1000 * 60 * 60)) % 24;

//   // minutes
//   int min = (milli / (1000 * 60)) % 60;

//   // seconds
//   int sec = (milli / 1000) % 60;

//   // milliseconds
//   int ms = milli % 1000;

//   char t[10];
//   sprintf(t, "%02d:%02d.%03d", min, sec, ms);
//   return t;
// }