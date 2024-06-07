Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/espressif32/esp32-s3-devkitc-1.html
PLATFORM: Espressif 32 (2024.4.13+sha.3f01ec5) > Espressif ESP32-S3-DevKitC-1-N8 (8 MB QD, No PSRAM)
HARDWARE: ESP32S3 240MHz, 320KB RAM, 8MB Flash
DEBUG: Current (esp-builtin) On-board (esp-builtin) External (cmsis-dap, esp-bridge, esp-prog, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa)
PACKAGES:

- framework-arduinoespressif32 @ 3.0.0+sha.08e138f
- tool-esptoolpy @ 4.7.2
- tool-mkfatfs @ 2.0.1
- tool-mklittlefs @ 3.2.0
- tool-riscv32-esp-elf-gdb @ 12.1.0+20221002
- tool-xtensa-esp-elf-gdb @ 12.1.0+20221002
- toolchain-riscv32-esp @ 12.2.0+20230208
- toolchain-xtensa-esp32s3 @ 12.2.0+20230208
  LDF: Library Dependency Finder -> https://bit.ly/configure-pio-ldf
  LDF Modes: Finder ~ chain, Compatibility ~ soft
  Found 26 compatible libraries
  Scanning dependencies...
  Dependency Graph
  |-- Wire @ 2.0.0
  |-- SPI @ 2.0.0
  Building in debug mode
  Retrieving maximum program size .pio/build/esp32s3/firmware.elf
  Checking size .pio/build/esp32s3/firmware.elf
  Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
  RAM: [ ] 3.8% (used 12440 bytes from 327680 bytes)
  Flash: [= ] 5.3% (used 178765 bytes from 3342336 bytes)
  Configuring upload protocol...
  AVAILABLE: cmsis-dap, esp-bridge, esp-builtin, esp-prog, espota, esptool, iot-bus-jtag, jlink, minimodule, olimex-arm-usb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa
  CURRENT: upload_protocol = esptool
  Looking for upload port...
  Auto-detected: /dev/cu.usbmodem2
  Uploading .pio/build/esp32s3/firmware.bin
  esptool.py v4.7.2
  Serial port /dev/cu.usbmodem2
  Connecting...
  Chip is ESP32-S3 (QFN56) (revision v0.1)
  Features: WiFi, BLE
  Crystal is 40MHz
  MAC: f4:12:fa:41:57:dc
  Uploading stub...
  Running stub...
  Stub running...
  Changing baud rate to 460800
  Changed.
  Configuring flash size...
  Auto-detected Flash size: 8MB
  Flash will be erased from 0x00000000 to 0x00003fff...
  Flash will be erased from 0x00008000 to 0x00008fff...
  Flash will be erased from 0x0000e000 to 0x0000ffff...
  Flash will be erased from 0x00010000 to 0x0003bfff...
  SHA digest in image updated
  Compressed 13440 bytes to 9667...
  Writing at 0x00000000... (100 %)
  Wrote 13440 bytes (9667 compressed) at 0x00000000 in 0.5 seconds (effective 236.0 kbit/s)...
  Hash of data verified.
  Compressed 3072 bytes to 146...
  Writing at 0x00008000... (100 %)
  Wrote 3072 bytes (146 compressed) at 0x00008000 in 0.1 seconds (effective 259.5 kbit/s)...
  Hash of data verified.
  Compressed 8192 bytes to 47...
  Writing at 0x0000e000... (100 %)
  Wrote 8192 bytes (47 compressed) at 0x0000e000 in 0.1 seconds (effective 569.3 kbit/s)...
  Hash of data verified.
  Compressed 179136 bytes to 121592...
  Writing at 0x00010000... (12 %)
  Writing at 0x0001847f... (25 %)
  Writing at 0x0001e1d6... (37 %)
  Writing at 0x0002393c... (50 %)
  Writing at 0x00029066... (62 %)
  Writing at 0x0002e747... (75 %)
  Writing at 0x00033baf... (87 %)
  Writing at 0x00039354... (100 %)
  Wrote 179136 bytes (121592 compressed) at 0x00010000 in 2.8 seconds (effective 513.4 kbit/s)...
  Hash of data verified.
