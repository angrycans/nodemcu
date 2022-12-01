#include <SPI.h>
#include <SD.h>

#include "HAL/HAL.h"

bool B_SDCARDOK = false;

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

void HAL::SDCARD_Init()
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
        ErrInfo += "SD CARD FAILED\n";
        HAL::DISPLAY_Update();
    }

    B_SDCARDOK = true;
    ErrInfo = "";

    logger.Begin(B_SDCARDOK);
    // if (SD.mkdir("XLAPDATA"))
    // {
    //   Serial.println("XLAPDATA dir is created.");
    // }

    File root = SD.open("/XLAPDATA");

    printDirectory(root, 0);

    root.close();

    logger.LogInfo("getTrackInfo");
    race.getTrackInfo();

    // logger.LogInfo(race.getHeader(2022, 11, 29, 15, 16, 30));

    // logger.LogInfo("print XLAPDATA Directory done!");

#if defined(DEBUG)
    // SD.remove("/XLAPDATA/gps.txt");
    // gpsFile = SD.open("/XLAPDATA/gps.txt", FILE_WRITE);
#endif

    // getTrack();

    // sprintf(displayInfo.log, "init sd card done!");
    // showDisplay();
    // delay(100);
    logger.LogInfo("init sd card done!");
    // ErrInfo += "sdcard init ok.\n";
}
