#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SSD1306Wire.h"
#include "SH1106Wire.h"
#include "SDLogger.h"

#define LED D0        // Led in NodeMCU at pin GPIO16 (D0).
#define OLED_RESET -1 // GPIO0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
static unsigned long lastDisplayTime = 0;

const int chipSelect = D8;

File myFile;

SDLogger logger;
// SSD1306Wire display(0x3c, SDA, SCL); // 0.96

SH1106Wire display(0x3c, SDA, SCL); // 1.3

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
  Serial.print("initsd...");

  // pinMode(D8, OUTPUT);

  if (!SD.begin(chipSelect))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile)
  {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile)
  {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available())
    {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
void setup()
{
  Serial.begin(9600);
  initsd();
  Serial.println("initialization done.");

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