
#include "bsp.h"

BSP::BSP()
{
    init();
}

void BSP::init()
{
    printBspInfos();
    lfs.init();

    sd.init();

#if BSP_MODULE_LVGL
    lvgl.init();
#endif
    printf("%s\n", "BSP init ok");
}

void BSP::printBspInfos()
{
    /* Print Logo */
    printf("%s\n", Logo.c_str());
    printf(" AcansBSP %s ;)\n Author: angrycans\n", ACANS_BSP_VERISON);
    printf(" Project: %s\n", ACANS_PROJECT_NAME);

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        printf("Get flash size failed");
        return;
    }
    printf(" %luMB %s flash\n", flash_size / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    printf(" Minimum free heap size: %ld bytes\n\n", esp_get_minimum_free_heap_size());

    // 初始化PSRAM
    if (!psramFound())
    {
        Serial.println("PSRAM not found");
        return;
    }
    else
    {
        size_t psramSize = ESP.getPsramSize();
        Serial.print("Total PSRAM size: ");
        Serial.print(psramSize);
        Serial.println(" bytes");

        // 获取可用的PSRAM大小
        size_t freePsram = ESP.getFreePsram();
        Serial.print("Free PSRAM: ");
        Serial.print(freePsram);
        Serial.println(" bytes");

        // 获取PSRAM堆信息
        multi_heap_info_t heap_info;
        heap_caps_get_info(&heap_info, MALLOC_CAP_SPIRAM);
        Serial.print("Total PSRAM heap size: ");
        Serial.print(heap_info.total_free_bytes + heap_info.total_allocated_bytes);
        Serial.println(" bytes");
        Serial.print("Free PSRAM heap size: ");
        Serial.print(heap_info.total_free_bytes / (1024 * 1024));
        Serial.println(" M");
    }

    printf("%s\n", "BSP init...");
}
