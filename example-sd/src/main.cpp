#include <Arduino.h>
#include <SPI.h>
#include "SdFat.h"
#include <USB.h>
#include <USBMSC.h>
#include <rom/rtc.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

namespace {

// 安全引脚
constexpr uint8_t kSdCardMiso = 5;
constexpr uint8_t kSdCardMosi = 6;
constexpr uint8_t kSdCardSck = 7;
constexpr uint8_t kSdCardCs = 4;

SdFat sd;
USBMSC usbMsc;
uint32_t sdSectorCount = 0;

// 高效对齐缓冲区
static uint8_t sector_buf[512] __attribute__((aligned(4)));

int32_t onUsbRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  if (!sd.card()->readSector(lba, sector_buf)) return -1;
  memcpy(buffer, sector_buf + offset, bufsize);
  return (int32_t)bufsize;
}

int32_t onUsbWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  if (lba >= sdSectorCount) return -1;

  // 重点：即便 offset 为 0，也必须拷贝到 aligned(4) 的 sector_buf 中转
  // 防止 USB 协议栈提供的 buffer 地址不对齐导致 DMA 写入失败
  if (offset == 0 && bufsize == 512) {
    memcpy(sector_buf, buffer, 512);
    return sd.card()->writeSector(lba, sector_buf) ? 512 : -1;
  }
  
  // 局部写入逻辑
  if (!sd.card()->readSector(lba, sector_buf)) return -1;
  memcpy(sector_buf + offset, buffer, bufsize);
  return sd.card()->writeSector(lba, sector_buf) ? (int32_t)bufsize : -1;
}

bool onUsbStartStop(uint8_t powerCondition, bool start, bool loadEject) {
  return true;
}

} // namespace

void setup() {
  // 禁用 Brownout 防止电压波动导致复位
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // 强制初始化硬件 UART 串口看日志
  Serial0.begin(115200);
  delay(2000);
  Serial0.printf("\n\n--- MSC STABILITY TEST (Reason: %d) ---\n", (int)rtc_get_reset_reason(0));

  // 1. 初始化 SPI 和 SdFat (提升到 20MHz)
  SPI.begin(kSdCardSck, kSdCardMiso, kSdCardMosi, kSdCardCs);
  if (sd.begin(SdSpiConfig(kSdCardCs, SHARED_SPI, SD_SCK_MHZ(20), &SPI))) {
    sdSectorCount = sd.card()->sectorCount();
    Serial0.printf("SD Ready: %u sectors\n", (unsigned)sdSectorCount);
  } else {
    Serial0.println("SD cardBegin FAILED! Check wiring on GPIO 4,5,6,7");
  }

  // 2. 配置 USB 
  USB.VID(0x303A);
  USB.PID(0x0002);
  USB.productName("ESP32S3_SD_DISK");

  usbMsc.vendorID("ESP32S3");
  usbMsc.productID("SD_Disk");
  usbMsc.onRead(onUsbRead);
  usbMsc.onWrite(onUsbWrite);
  usbMsc.mediaPresent(sdSectorCount > 0);

  if (sdSectorCount > 0) {
    usbMsc.begin(sdSectorCount, 512);
    Serial0.println("MSC Layer initialized.");
  }

  USB.begin();
  Serial0.println("Native USB Port is now a Pure U-Disk.");
  Serial0.println("Note: No logs on Native port. Use UART port for logs.");
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last > 3000) {
    last = millis();
    Serial0.println("System Running...");
  }
  delay(10);
}
