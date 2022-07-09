
#ifndef UTILS_HPP
#define UTILS_HPP
// String formatMs(unsigned long milli);

String formatMs(unsigned long milli)
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

#endif