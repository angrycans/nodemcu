#ifndef display_helper_HPP
#define display_helper_HPP

#include <Wire.h>
#include "SSD1306Wire.h"
#include "SH1106Wire.h"
#include "OLEDDisplayUi.h"

const uint8_t wifi_logo[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0xfe, 0x7f, 0x00, 0x00,
    0xfc, 0x3f, 0xf8, 0x1f, 0x00, 0x00, 0xf0, 0x0f, 0xe0, 0x07, 0x00, 0x00,
    0xc0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00};

#if defined(OLED13)
SH1106Wire display(0x3c, SDA, SCL); // 1.3 SH1106 d2 d1
#else
SSD1306Wire display(0x3c, SDA, SCL); // 0.96 ssd1306 d2 d1
#endif

OLEDDisplayUi ui(&display);

String ErrInfo = "";

void drawWifi(OLEDDisplay *display, int x, int y);
void drawBattery(OLEDDisplay *display, int x, int y, int n);
void drawSatles(OLEDDisplay *display, int x, int y, int n);
void drawErrInfo(OLEDDisplay *display, int x, int y, int n);

void logoFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void clockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void retFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void lapFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);

void initDisplay();
void showDisplay();
// void showLogoDisplay();

FrameCallback frames1[] = {logoFrame};
int frameCount1 = 1;

FrameCallback frames2[] = {clockFrame, retFrame, lapFrame};
int frameCount2 = 3;

void setDisplayFrame(int f)
{

  if (ui.getUiState()->currentFrame != f)
  {
    ui.switchToFrame(f);
  }
}
void showDisplay()
{

  switch (race.getStatus().status)
  {
  case d_Looping:

    if (race.sessionActive)
    {

      ui.enableAutoTransition();
      // setDisplayFrame(2);
    }
    else
    {
      ui.disableAutoTransition();
      setDisplayFrame(0);
    }

    break;

  case d_Recording:
    ui.disableAutoTransition();
    setDisplayFrame(0);
    break;

  default:
    break;
  }

  ui.update();
}

void initDisplay()
{
  ui.setTargetFPS(30);

  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames

  ui.setFrames(frames1, frameCount1);

  ui.disableAllIndicators();
  ui.disableAutoTransition();

  ui.init();

  // display.init();
  // display.clear();
  display.flipScreenVertically();
  // display.invertDisplay();

  ui.switchToFrame(0);
}

void drawWifi(OLEDDisplay *display, int x, int y)
{
  display->drawXbm(x, y, 12, 12, wifi_logo);
}

void logoFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 12 + y, "RaceLap");
}

void clockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  if (ErrInfo != "")
  {
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(x, 12 + y, ErrInfo);
    return;
  }
  drawBattery(display, 104, 1, 100);

  drawSatles(display, 0, 1, gps.satellites.isValid() ? (int)gps.satellites.value() : -1);

  display->drawLine(0, 12, 0 + 128, 12);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  if (strcmp(DataFileName, "") > 0 && dataFile)
  {
    (millis() / 1000) % 2 ? display->drawString(64, 0, "Rec") : display->drawString(64, 0, "");
  }
  // digitalWrite(LED_BUILTIN, (millis() / 1000) % 2);

  // #if defined(DEBUG)
  //   snprintf(logbuff, sizeof(logbuff), "satellites d", gps.satellites.value());
  //   Serial.println(logbuff);
  // #endif
  if ((!gps.location.isUpdated()) && (int)gps.satellites.value() < 3)
  {

    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_16);
    (millis() / 1000) % 2 ? display->drawString(64 + x, 32 + y, "GPS Search...") : display->drawString(64 + x, 32 + y, "");

    return;
  }

  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);

  if (!race.nearTarck(gps.location.lat(), gps.location.lng()))
  // if (false)
  {
    (millis() / 1000) % 2 ? display->drawString(x, 20 + y, "no track") : display->drawString(+x, 20 + y, "");
  }
  else
  {
    display->drawString(x, 20 + y, "LAP");
    display->setFont(ArialMT_Plain_16);
    display->drawString(22 + x, 16 + y, String(race.totalLap));
  }

  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(104 + x, 16 + y, String(KMPH, 0));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(105 + x, 21 + y, "KMH");

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);

  race.sessionActive ? display->drawString(64 + x, 32 + y, formatTime2(millis() - race.sessionTime)) : display->drawString(64 + x, 32 + y, formatTime2(0));
  // display->drawString(64 + x, 32 + y, formatMs(millis()));
}

void retFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  // display->drawString(0 + x, 0 + y, "LOG");
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 0 + y, "2022/07/08 15:30");
  // display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);

  char buff[64];
  sprintf(buff, "LAP COUNT %d BEST %d", race.totalLap, race.bestLap);
  display->drawString(x, 12 + y, buff);

  sprintf(buff, "BEST  %s", formatTime(race.bestSessionTime));
  display->drawString(x, 24 + y, buff);

  sprintf(buff, "SESSION  %s", formatTime(race.sessionTime));
  display->drawString(x, 36 + y, buff);

  sprintf(buff, "TOP %dKMH AVE %dKMH", (int)race.maxspeed, (int)race.avespeed);
  display->drawString(x, 48 + y, "TOP 189KMH AVE 98KMH");
}

void lapFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  /*
  class LapInfo
{
public:
  int maxspeed;
  int avespeed;
  unsigned long time;
  unsigned long difftime;
  // bool isbest;
};

  */

  for (int i = 0; i < race.lapInfoList->size(); i++)
  {
    char tmp[48];
    sprintf(tmp, "%d %s +%s %d %d", i, formatTime(race.lapInfoList->get(i).time), formatTimeMs(race.lapInfoList->get(i).difftime), race.lapInfoList->get(i).maxspeed, race.lapInfoList->get(i).avespeed);

    display->drawString(64 + x, 12 * i + y, tmp);
  }
}

// Adapted from Adafruit_SSD1306
void drawBattery(OLEDDisplay *display, int x, int y, int n)
{

  // Serial.println("rank");
  // int rank = n / 25;
  // Serial.println(rank);

#if defined(OLED13)
  int y0 = 2;
  int y1 = 5;
  int x0 = 1;
  int x1 = 2;
  int w0 = 18;
  int h0 = 9;
#else
  int y0 = 3;
  int y1 = 6;
  int x0 = 2;
  int x1 = 3;
  int w0 = 24;
  int h0 = 12;
#endif
  if (n < 0)
  {
    display->drawRect(x, y + y0, 1, y1);
    display->drawLine(x + x0, y + 1, x + x0, y + h0 - 2);
    display->drawRect(x + x1, y, w0, h0);
    display->drawLine(x + x1 + w0, y + 1, x + x1 + w0, y + h0 - 2);
    int x1 = x + 7;
    int y1 = y + 2;
    display->fillTriangle(x1, y1, x1 + 4, y1 + 4, x1 + 4, y1 + 2);
    display->fillTriangle(x1 + 4, y1 + 2, x1 + 4, y1, x1 + 8, y1 + 4);
    return;
  }

  switch (n / 25)
  {
  case 0:
    display->drawRect(x, y + y0, 1, y1);
    display->drawLine(x + x0, y + 1, x + x0, y + h0 - 2);
    display->drawRect(x + x1, y, w0, h0);
    display->drawLine(x + x1 + w0, y + 1, x + x1 + w0, y + h0 - 2);
    break;

  case 1:
    display->drawRect(x, y + y0, 1, y1);
    display->drawLine(x + x0, y + 1, x + x0, y + h0 - 2);
    display->drawRect(x + x1, y, w0, h0);
    display->fillRect(x + x1 + 14, y, 4, h0);
    display->drawLine(x + x1 + w0, y + 1, x + x1 + w0, y + h0 - 2);

    break;
  case 2:
    display->drawRect(x, y + y0, 1, y1);
    display->drawLine(x + x0, y + 1, x + x0, y + h0 - 2);
    display->drawRect(x + x1, y, w0, h0);
    display->fillRect(x + x1 + 10, y, 8, h0);
    display->drawLine(x + x1 + w0, y + 1, x + x1 + w0, y + h0 - 2);
    break;
  case 3:
    display->drawRect(x, y + y0, 1, y1);
    display->drawLine(x + x0, y + 1, x + x0, y + h0 - 2);
    display->drawRect(x + x1, y, w0, h0);
    display->fillRect(x + x1 + 5, y, 13, h0);
    display->drawLine(x + x1 + w0, y + 1, x + x1 + w0, y + h0 - 2);
    break;

  case 4:
    display->drawRect(x, y + y0, 1, y1);
    display->drawLine(x + x0, y + 1, x + x0, y + h0 - 2);
    display->drawRect(x + x1, y, w0, h0);
    display->fillRect(x + x1, y, 18, h0);
    display->drawLine(x + x1 + w0, y + 1, x + x1 + w0, y + h0 - 2);
    break;

  default:
    display->drawRect(x, y + y0, 1, y1);
    display->drawLine(x + x0, y + 1, x + x0, y + h0 - 2);
    display->drawRect(x + x1, y, w0, h0);
    display->fillRect(x + x1, y, 18, h0);
    display->drawLine(x + x1 + w0, y + 1, x + x1 + w0, y + h0 - 2);
    break;
  }
}
void drawSatles(OLEDDisplay *display, int x, int y, int n)
{
  display->drawLine(x, y, x + 4, y);
  display->drawLine(x, y, x + 2, y + 3);
  display->drawLine(x + 4, y, x + 2, y + 3);
  display->drawLine(x + 2, y + 1, x + 2, y + 9);

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 24, y, String(n));

  if (n == -1)
  {
    display->drawLine(x + 5, y + 4, x + 10, y + 9);
    display->drawLine(x + 10, y + 4, x + 5, y + 9);
    return;
  }

  switch (n / 3)
  {
  case 0:
    display->drawRect(x + 5, y + 6, 3, 4);
    // display->drawRect(x + 5 + 3 + 1, y + 6 - 2, 3, 4 + 2);
    // display->drawRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 4 + 2 * 2);
    // display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 4 + 2 * 3);
    return;
    break;

  case 1:
    display->fillRect(x + 5, y + 6, 3, 4);
    display->drawRect(x + 5 + 3 + 1, y + 6 - 2, 3, 4 + 2);
    display->drawRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 4 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 4 + 2 * 3);
    break;
  case 2:
    display->fillRect(x + 5, y + 6, 3, 4);
    display->fillRect(x + 5 + 3 + 1, y + 6 - 2, 3, 4 + 2);
    display->drawRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 4 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 4 + 2 * 3);
    break;
  case 3:
    display->fillRect(x + 5, y + 6, 3, 4);
    display->fillRect(x + 5 + 3 + 1, y + 6 - 2, 3, 4 + 2);
    display->fillRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 4 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 4 + 2 * 3);
    break;

  default:
    display->fillRect(x + 5, y + 6, 3, 4);
    display->fillRect(x + 5 + 3 + 1, y + 6 - 2, 3, 4 + 2);
    display->fillRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 4 + 2 * 2);
    display->fillRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 4 + 2 * 3);

    break;
  }
}

#endif