#ifndef __CONFIG_H
#define __CONFIG_H

/*=========================
   Application configuration
 *=========================*/
#define VERSION_FIRMWARE_NAME "RaceLap"
#define VERSION_SOFTWARE "v0.1"
#define VERSION_HARDWARE "v0.1"
#define VERSION_AUTHOR_NAME "Angrycans"
/*=========================
   Hardware Configuration
 *=========================*/

// #define TXD1 0 //串口 1使用的TX PIN
// #define RXD1 1 //串口 1使用的RX PIN

#define TXD1 18 //串口 1使用的TX PIN
#define RXD1 19 //串口 1使用的RX PIN

#define gpsSerial Serial1

#define LED_BUILTIN 4
#define BUAD 115200

// Sdcard luat esp32-c3
#define CONFIG_SDCARD_SCK 2
#define CONFIG_SDCARD_MOSI 3
#define CONFIG_SDCARD_MISO 10
#define CONFIG_SDCARD_CS 6

// Display
#define CONFIG_DISPLAY_SDA 8
#define CONFIG_DISPLAY_SCL 5

// // Display
// #define CONFIG_DISPLAY_SDA 19
// #define CONFIG_DISPLAY_SCL 18

// /* I2C */
// #define CONFIG_MCU_SDA 19 // 19
// #define CONFIG_MCU_SCL 18 // 18

/* I2C */
#define CONFIG_MCU_SDA 8 // 19
#define CONFIG_MCU_SCL 5 // 18

/* Encoder */
#define CONFIG_ENCODER_B_PIN 34
#define CONFIG_ENCODER_A_PIN 35
#define CONFIG_ENCODER_PUSH_PIN 27

/* Power */
#define CONFIG_POWER_EN_PIN 21

/* Debug USART */
#define CONFIG_DEBUG_SERIAL Serial

#endif
