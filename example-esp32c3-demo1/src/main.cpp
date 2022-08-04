#include <Arduino.h>
#include <FS.h> // File System for Web Server Files
#include <SPI.h>
#include <SD.h>

#include "SH1106Wire.h"
#include "OLEDDisplayUi.h"

// SD Reader CS pin
const int chipSelect = 6;
bool B_SD = false;
#define SCK 2
#define MOSI 3
#define MISO 10
#define SD_CS 6

int led1 = 12;
int led2 = 13;

SH1106Wire display(0x3c, 8, 5); // 1.3 SH1106 gpio4 gpio5
OLEDDisplayUi ui(&display);

void retFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);

  display->drawString(x, y, "123");
}

void lapFrame(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);

  display->drawString(x, y, "234");
}

FrameCallback frames[] = {retFrame, lapFrame};
int frameCount = 2;

void printDirectory(File dir, int numTabs)
{
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
      // time_t cr = entry.getCreationTime();
      // time_t lw = entry.getLastWrite();
      // struct tm *tmstruct = localtime(&cr);
      // Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      //  tmstruct = localtime(&lw);
      // Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}

void initDisplay()
{
  // display.init();
  // display.clear();
  // // display.flipScreenVertically();
  // display.setFont(ArialMT_Plain_24);
  // display.setTextAlignment(TEXT_ALIGN_CENTER);
  // display.drawString(64, 12, "RaceLap2");
  // display.display();

  ui.setTargetFPS(30);

  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames

  ui.setFrames(frames, frameCount);

  // ui.disableAllIndicators();
  // ui.disableAutoTransition();

  ui.init();

  // // display.init();
  // // display.clear();
  display.flipScreenVertically();
}

void initSD()
{
  // // keep checking the SD reader for valid SD card/format
  delay(100);
  Serial.println("init SD Card");

  SPI.begin(SCK, MISO, MOSI, -1);

  Serial.println("SD test");
  while (!SD.begin(SD_CS, SPI, 2000000))
  {
    Serial.println("Card Mount Failed");

    B_SD = false;
  }

  // while (!SD.begin(chipSelect))
  // {
  //   Serial.println("init SD Card Failed");

  //   B_SD = false;
  // }

  B_SD = true;
  if (SD.mkdir("RLDATA"))
  {
    Serial.println("RLDATA dir is created.");
  }

  File root = SD.open("/RLDATA");

  printDirectory(root, 0);

  root.close();

  Serial.println("print RLDATA Directory done!");

  delay(100);
}

void setup()
{
  Serial.begin(115200);
  initDisplay();
  // initSD();

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  delay(1000);
  Serial.println("setup ok");
}

void loop()
{

  digitalWrite(led1, HIGH);
  digitalWrite(led2, LOW);
  delay(1000);
  digitalWrite(led1, LOW);
  digitalWrite(led2, HIGH);
  delay(1000);

  ui.update();
}