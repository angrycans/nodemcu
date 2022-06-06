#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "SH1106Wire.h"
#include "SDLogger.h"

#define LED D0        // Led in NodeMCU at pin GPIO16 (D0).
#define OLED_RESET -1 // GPIO0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
static unsigned long lastDisplayTime = 0;

SDLogger logger;
SSD1306Wire display(0x3c, SDA, SCL); // 0.86

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

void setup()
{
  Serial.begin(9600);

  logger.Begin(false);
  initDisplay();
  // put your setup code here, to run once:
}

void loop()
{

  digitalWrite(LED, (millis() / 1000) % 2);

  if (millis() - lastDisplayTime > 1000)
  {
    showDisplay();
    lastDisplayTime = millis();
  }

  // put your main code here, to run repeatedly:
}