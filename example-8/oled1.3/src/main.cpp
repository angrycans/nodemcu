#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SSD1306Wire.h"
#include "SH1106Wire.h"
#include "OLEDDisplayUi.h"
#include <TimeLib.h>

#include "SDLogger.h"
#include "images.h"

#define LED D0        // Led in NodeMCU at pin GPIO16 (D0).
#define OLED_RESET -1 // GPIO0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
static unsigned long lastDisplayTime = 0;

const int chipSelect = D8;

int debug;

SDLogger logger;
SSD1306Wire display(0x3c, SDA, SCL); // 0.96
// SH1106Wire display(0x3c, SDA, SCL); // 1.3

void initDisplay()
{
  logger.LogInfo("init Display");

  // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.init();
}

void showDisplay()
{
  // TRACE("showDisplay");

  display.clear();

  display.drawString(0, 0, "Hello world22");
  display.display();
}

void initsd()
{

  // pinMode(D8, OUTPUT);

  if (!SD.begin(chipSelect))
  {
    return;
  }
}

void setup()
{

  if (Serial)
    debug = 1;
  else
    debug = 0;

  Serial.begin(9600);
  initsd();
  Serial.println("initialization done.");

  Serial.print("-----");
  Serial.println(debug);

  logger.Begin(true);

  initDisplay();
}

void loop()
{

  digitalWrite(LED, (millis() / 1000) % 2);
  digitalWrite(buildinled, (millis() / 1000) % 2);

  if (millis() - lastDisplayTime > 1000)
  {
    showDisplay();
    lastDisplayTime = millis();
  }
}
