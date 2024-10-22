#pragma once

/* Common configs */
#define ACANS_BSP_VERISON "v0.1"
#define ACANS_PROJECT_NAME "XLAP"

/* Enable module Button */
#define BSP_MODULE_BUTTON 1
#if BSP_MODULE_BUTTON
/* Set pin -1 to disable */
#define BSP_BTN_A_PIN 9
#define BSP_BTN_B_PIN 0
#define BSP_BTN_C_PIN -1
#endif

#define BSP_MODULE_LED 1
#if BSP_MODULE_LED
#define BSP_LED_PIN 1
#endif

#define BSP_MODULE_SD 1

#define BSP_MODULE_LVGL 1

#define BSP_MODULE_GPS 1

#if BSP_MODULE_GPS
#define gpsSerial Serial1
#define TXD1 8  // 串口 1使用的TX PIN
#define RXD1 18 // 串口 1使用的RX PIN

#define BSP_MODULE_GPS_NOFLASH 1
#define gpsBUAD 115200

#if BSP_MODULE_GPS_NOFLASH
#define gpsCFGBUAD 9600

#endif

#endif
