/**
   The MIT License (MIT)

   Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
   Copyright (c) 2018 by Fabrice Weinberg

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   ThingPulse invests considerable time and money to develop these open source libraries.
   Please support us by buying our products (and not the clones) from
   https://thingpulse.com

*/

// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>        // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
// or #include "SH1106Wire.h", legacy include: `#include "SH1106.h"`
// For a connection via I2C using brzo_i2c (must be installed) include
// #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Brzo.h"
// #include "SH1106Brzo.h"
// For a connection via SPI include
// #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Spi.h"
// #include "SH1106SPi.h"

// Use the corresponding display class:

// Initialize the OLED display using SPI
// D5 -> CLK
// D7 -> MOSI (DOUT)
// D0 -> RES
// D2 -> DC
// D8 -> CS
// SSD1306Spi        display(D0, D2, D8);
// or
// SH1106Spi         display(D0, D2);

// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
// SSD1306Brzo display(0x3c, D3, D5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

// Initialize the OLED display using Wire library
#include "OLEDDisplayUi.h"
//#include "font.h"

SSD1306Wire display(0x3c, SDA, SCL); // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h

OLEDDisplayUi ui(&display);

String twoDigits(int digits)
{
  if (digits < 10)
  {
    String i = '0' + String(digits);
    return i;
  }
  else
  {
    return String(digits);
  }
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

void printBuffer(void)
{
  // Initialize the log buffer
  // allocate memory to store 8 lines of text and 30 chars per line.
  display.setLogBuffer(5, 30);

  // Some test data
  const char *test[] = {
      "Hello",
      "World",
      "----",
      "Show off",
      "how",
      "the log buffer",
      "is",
      "working.",
      "Even",
      "scrolling is",
      "working"};

  for (uint8_t i = 0; i < 11; i++)
  {
    display.clear();
    // Print to the screen
    display.println(test[i]);
    // Draw it to the internal screen buffer
    display.drawLogBuffer(0, 0);
    // Display it on the screen
    display.display();
    delay(500);
  }
}

// Adapted from Adafruit_SSD1306
void drawBattery(OLEDDisplay *display, int x, int y, int n)
{

  // Serial.println("rank");
  // int rank = n / 25;
  // Serial.println(rank);

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

void clockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  //   int screenW = 128;
  // int screenH = 64;
  // int clockCenterX = screenW / 2;
  // int clockCenterY = ((screenH - 16) / 2); // top yellow part is 16 px height
  drawBattery(display, 100, 1, 74);
  // drawBattery(0, 16, 50);
  // drawBattery(0, 32, 25);

  // drawBattery(0, 48, 0);
  drawSatles(display, 0, 1, 3);
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
  drawBattery(display, 100, 0, 100);

  drawSatles(display, 0, 0, 12);

  display->drawLine(0, 14, 0 + 128, 14);
}

FrameCallback frames[] = {retFrame, clockFrame};
int frameCount = 2;

void setup()
{
  Serial.begin(9600);
  delay(1000);

  ui.setTargetFPS(30);

  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  ui.disableAllIndicators();
  ui.disableAutoTransition();

  ui.init();

  // display.init();
  // display.clear();
  display.flipScreenVertically();
  // display.invertDisplay();

  ui.switchToFrame(1);
}

void loop()
{

  ui.update();
}
