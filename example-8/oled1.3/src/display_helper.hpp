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

String formatMs(unsigned long milli);
void drawWifi(OLEDDisplay *display, int x, int y);
void drawBattery(OLEDDisplay *display, int x, int y, int n);
void drawSatles(OLEDDisplay *display, int x, int y, int n);
void clockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void retFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void lapFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);

void initDisplay();

FrameCallback frames[] = {clockFrame, retFrame, lapFrame};
int frameCount = 3;

void initDisplay()
{
  ui.setTargetFPS(30);

  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames

  ui.setFrames(frames, frameCount);

  ui.disableAllIndicators();
  // ui.disableAutoTransition();

  ui.init();

  // display.init();
  // display.clear();
  display.flipScreenVertically();
  // display.invertDisplay();

  ui.switchToFrame(2);
}

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

void drawWifi(OLEDDisplay *display, int x, int y)
{
  display->drawXbm(x, y, 16, 16, wifi_logo);
}

void clockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  //   int screenW = 128;
  // int screenH = 64;
  // int clockCenterX = screenW / 2;
  // int clockCenterY = ((screenH - 16) / 2); // top yellow part is 16 px height
  drawBattery(display, 100, 1, -1);
  // drawBattery(0, 16, 50);
  // drawBattery(0, 32, 25);

  // drawBattery(0, 48, 0);
  drawSatles(display, 0, 1, 3);

  drawWifi(display, 80, -1);
  // drawSatles(0, 16, 6);
  // drawSatles(0, 32, 9);
  // drawSatles(0, 48, 12);

  display->drawLine(0, 14, 0 + 128, 14);

  // String timenow = String(millis() / 60000) + ":" + twoDigits(millis() / 1000) + "." + twoDigits(millis());

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 0, "Rec");
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, 20 + y, "LAP");
  display->setFont(ArialMT_Plain_16);
  display->drawString(22 + x, 16 + y, "5");
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(104 + x, 16 + y, "119");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(105 + x, 21 + y, "KMH");

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);

  display->drawString(64 + x, 32 + y, formatMs(millis()));
}

void retFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  // drawBattery(display, 100, 0, 100);
  // drawSatles(display, 0, 0, 12);
  // display->drawLine(0, 14, 0 + 128, 14);

  // display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  // display->drawString(0 + x, 0 + y, "LOG");
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 0 + y, "2022/07/08 15:30");
  // display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, 12 + y, "LAP COUNT 10 BEST 4");
  display->drawString(x, 24 + y, "BEST  00:56.381");
  // display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(x, 36 + y, "SESSION  8:32.980");
  // display->setTextAlignment(TEXT_ALIGN_LEFT);
  // display->setFont(ArialMT_Plain_10);
  display->drawString(x, 48 + y, "TOP 189KMH AVE 98KMH");
}

void lapFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);

  for (int i = 0; i < 5; i++)
  {
    display->drawString(64 + x, 12 * i + y, "1 00:56.381 116 101");
  }
}

// Adapted from Adafruit_SSD1306
void drawBattery(OLEDDisplay *display, int x, int y, int n)
{

  // Serial.println("rank");
  // int rank = n / 25;
  // Serial.println(rank);

  if (n < 0)
  {
    display->drawRect(x, y + 3, 2, 6);
    display->drawLine(x + 2, y + 1, x + 2, y + 10);
    display->drawRect(x + 3, y, 24, 12);
    display->drawLine(x + 3 + 24, y + 1, x + 3 + 24, y + 10);
    int x1 = x + 9;
    int y1 = y + 2;
    display->fillTriangle(x1, y1, x1 + 7, y1 + 7, x1 + 7, y1 + 3);
    display->fillTriangle(x1 + 7, y1 + 3, x1 + 7, y1, x1 + 14, y1 + 7);
    return;
  }

  switch (n / 25)
  {
  case 0:
    display->drawRect(x, y + 3, 2, 6);
    display->drawLine(x + 2, y + 1, x + 2, y + 10);
    display->drawRect(x + 3, y, 24, 12);
    // display.fillRect(x + 4 + 20, y + 1, 22 - 20, 10);
    display->drawLine(x + 3 + 24, y + 1, x + 3 + 24, y + 10);
    break;

  case 1:
    display->drawRect(x, y + 3, 2, 6);
    display->drawLine(x + 2, y + 1, x + 2, y + 10);
    display->drawRect(x + 3, y, 24, 12);
    display->fillRect(x + 4 + 15, y + 1, 22 - 15, 10);
    display->drawLine(x + 3 + 24, y + 1, x + 3 + 24, y + 10);
    break;
  case 2:
    display->drawRect(x, y + 3, 2, 6);
    display->drawLine(x + 2, y + 1, x + 2, y + 10);
    display->drawRect(x + 3, y, 23, 12);
    display->fillRect(x + 4 + 10, y + 1, 21 - 10, 10);
    display->drawLine(x + 3 + 23, y + 1, x + 3 + 23, y + 10);
    break;
  case 3:
    display->drawRect(x, y + 3, 2, 6);
    display->drawLine(x + 2, y + 1, x + 2, y + 10);
    display->drawRect(x + 3, y, 23, 12);
    display->fillRect(x + 4, y + 1, 21, 10);
    display->drawLine(x + 3 + 23, y + 1, x + 3 + 23, y + 10);
    break;

  case 4:
    display->drawRect(x, y + 3, 2, 6);
    display->drawLine(x + 2, y + 1, x + 2, y + 10);
    display->drawRect(x + 3, y, 23, 12);
    display->fillRect(x + 4, y + 1, 21, 10);
    display->drawLine(x + 3 + 23, y + 1, x + 3 + 23, y + 10);
    break;

  default:
    display->drawRect(x, y + 3, 2, 6);
    display->drawLine(x + 2, y + 1, x + 2, y + 10);
    display->drawRect(x + 3, y, 23, 12);
    display->fillRect(x + 4, y + 1, 21, 10);
    display->drawLine(x + 3 + 23, y + 1, x + 3 + 23, y + 10);

    break;
  }
}
void drawSatles(OLEDDisplay *display, int x, int y, int n)
{

  display->drawLine(x, y, x + 4, y);
  display->drawLine(x, y, x + 2, y + 3);
  display->drawLine(x + 4, y, x + 2, y + 3);
  display->drawLine(x + 2, y + 1, x + 2, y + 10);

  switch (n / 3)
  {
  case 0:
    display->drawRect(x + 5, y + 6, 3, 5);
    display->drawRect(x + 5 + 3 + 1, y + 6 - 2, 3, 5 + 2);
    display->drawRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 5 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 5 + 2 * 3);
    break;

  case 1:

    display->fillRect(x + 5, y + 6, 3, 5);
    display->drawRect(x + 5 + 3 + 1, y + 6 - 2, 3, 5 + 2);
    display->drawRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 5 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 5 + 2 * 3);
    break;
  case 2:
    display->fillRect(x + 5, y + 6, 3, 5);
    display->fillRect(x + 5 + 3 + 1, y + 6 - 2, 3, 5 + 2);
    display->drawRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 5 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 5 + 2 * 3);

    break;
  case 3:
    display->fillRect(x + 5, y + 6, 3, 5);
    display->fillRect(x + 5 + 3 + 1, y + 6 - 2, 3, 5 + 2);
    display->fillRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 5 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 5 + 2 * 3);

    break;

  default:

    display->fillRect(x + 5, y + 6, 3, 5);
    display->fillRect(x + 5 + 3 + 1, y + 6 - 2, 3, 5 + 2);
    display->drawRect(x + 5 + (3 + 1) * 2, y + 6 - 2 * 2, 3, 5 + 2 * 2);
    display->drawRect(x + 5 + (3 + 1) * 3, y + 6 - 2 * 3, 3, 5 + 2 * 3);

    break;
  }
}

#endif