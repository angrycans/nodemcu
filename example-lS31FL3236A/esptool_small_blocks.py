#!/usr/bin/env python3

import os
import site
import sys

core_dir = os.environ.get(
    "PLATFORMIO_CORE_DIR", os.path.expanduser("~/.platformio")
)
package_dir = os.path.join(core_dir, "packages", "tool-esptoolpy")
site.addsitedir(os.path.join(package_dir, "_contrib"))
sys.path.insert(0, package_dir)

import esptool
from esptool.targets.esp32s3 import ESP32S3ROM

# This board's native USB link fails with larger ROM-loader packets.
ESP32S3ROM.FLASH_WRITE_SIZE = 0x100

esptool._main()
