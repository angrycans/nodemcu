
// #define LV_CONF_INCLUDE_SIMPLE 1
// #define LV_CONF_PATH "lv_conf.h"
#include "Wire.h"
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

TaskHandle_t Task1, Task2;

#define CONFIG_SDCARD_SCK 15
#define CONFIG_SDCARD_MISO 16
#define CONFIG_SDCARD_MOSI 5
#define CONFIG_SDCARD_CS 4

#define LED_RUN 1

void print_fibonacci()
{
  int n1 = 0;
  int n2 = 1;
  int term = 0;
  char print_buf[300];
  sprintf(print_buf, "Term %d: %d\n", term, n1);
  Serial.print(print_buf);
  term = term + 1;
  sprintf(print_buf, "Term %d: %d\n", term, n1);
  Serial.print(print_buf);
  for (;;)
  {
    term = term + 1;
    int n3 = n1 + n2;
    if (term % 5 == 0)
    {
      sprintf(print_buf, "Term %d: %d\n", term, n3);
      Serial.println(print_buf);
    }
    n1 = n2;
    n2 = n3;

    if (term >= 25)
      break;
  }
}

void sum_numbers()
{
  int n1 = 1;
  int sum = 1;
  char print_buf[300];
  for (;;)
  {
    if (n1 % 5 == 0)
    {
      sprintf(print_buf, "                                                            Term %d: %d\n", n1, sum);
      Serial.println(print_buf);
    }
    n1 = n1 + 1;
    sum = sum + n1;
    if (n1 >= 100)
      break;
  }
}

void codeForTask1(void *parameter)
{
  for (;;)
  {
    Serial.print("Code is running on Core: ");
    Serial.println(xPortGetCoreID());
    print_fibonacci();
  }
}

void codeForTask2(void *parameter)
{
  for (;;)
  {
    Serial.print("                                                            Code is running on Core: ");
    Serial.println(xPortGetCoreID());
    sum_numbers();
  }
}

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

void SDCARD_Init()
{

  // // keep checking the SD reader for valid SD card/format
  delay(100);
  Serial.println("init SD Card");

  SPI.begin(CONFIG_SDCARD_SCK, CONFIG_SDCARD_MISO, CONFIG_SDCARD_MOSI, -1);
  // while (!SD.begin(SD_CS, SPI, 2000000))
  // while (!SD.begin(CONFIG_SDCARD_CS, SPI, 2000000))
  // while (!SD.begin(CONFIG_SDCARD_CS, SPI, 80000000))
  while (!SD.begin(CONFIG_SDCARD_CS, SPI, 80000000))
  // while (!SD.begin(CONFIG_SDCARD_CS))
  {
    Serial.println("init SD Card Failed");
  }

  File root = SD.open("/");

  printDirectory(root, 0);

  root.close();
}

// RT-Thread function called by "RT_T.begin()"
void rt_setup(void)
{
}

void setup(void)
{
  // SPIバスとパネルの初期化を実行すると使用可能になります。

  Serial.begin(115200);

  SDCARD_Init();
  delay(1000);
  Serial.println("test start");

  xTaskCreatePinnedToCore(codeForTask1, "FibonacciTask", 5000, NULL, 2, &Task1, 0);
  // delay(500);  // needed to start-up task1
  xTaskCreatePinnedToCore(codeForTask2, "SumTask", 5000, NULL, 2, &Task2, 1);

  // pinMode(9,OUTPUT);

  //  digitalWrite(9, 0);
  //  delay(10);
  //    digitalWrite(9, 1);
  //  delay(50);

  pinMode(LED_RUN, OUTPUT);
  //      Wire.begin(CTP_SDA, CTP_SCL);
  // cts.begin(SAFE);
  // cts.setTouchLimit(1);//from 1 to 5
}

void loop(void)
{
  digitalWrite(LED_RUN, (millis() / 1000) % 2);
}
