#pragma once
#include <Arduino.h>
#include <FS.h>
#include <SD.h>

#define CONFIG_SDCARD_SCK 40
#define CONFIG_SDCARD_MISO 39
#define CONFIG_SDCARD_MOSI 41
#define CONFIG_SDCARD_CS 42

class BSPSD
{
private:
    bool _inited;

public:
    inline BSPSD() : _inited(false) {}
    inline ~BSPSD() {}

    /**
     * @brief
     *
     * @return true Success
     * @return false Failed
     */
    inline bool init()
    {
        /* Init spi */
        SPIClass *sd_spi = new SPIClass(HSPI);
        sd_spi->begin(CONFIG_SDCARD_SCK, CONFIG_SDCARD_MISO, CONFIG_SDCARD_MOSI, CONFIG_SDCARD_CS);
        bool ret = SD.begin(CONFIG_SDCARD_CS, *sd_spi, 40000000);

        if (!ret)
        {
            printf("[SD] init failed\n");
            return ret;
        }
        printf("[SD] init success, ");

        /* Get SD card info */
        uint8_t cardType = SD.cardType();
        if (cardType == CARD_MMC)
            printf("MMC");
        else if (cardType == CARD_SD)
            printf("SDSC");
        else if (cardType == CARD_SDHC)
            printf("SDHC");
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);

        _inited = true;
        return ret;
    }

    inline bool isInited() { return _inited; }
};
