#include "AXP2101.h"

namespace {
constexpr uint8_t REG_STATUS1 = 0x00;
constexpr uint8_t REG_STATUS2 = 0x01;
constexpr uint8_t REG_COMMON_CONFIG = 0x10;
constexpr uint8_t REG_BATFET_CONTROL = 0x12;
constexpr uint8_t REG_MIN_SYS_VOLTAGE = 0x14;
constexpr uint8_t REG_INPUT_VOLTAGE_LIMIT = 0x15;
constexpr uint8_t REG_INPUT_CURRENT_LIMIT = 0x16;
constexpr uint8_t REG_MODULE_ENABLE = 0x18;
constexpr uint8_t REG_LOW_BATTERY_WARNING = 0x1A;
constexpr uint8_t REG_POWERON_STATUS = 0x20;
constexpr uint8_t REG_POWEROFF_STATUS = 0x21;
constexpr uint8_t REG_POWEROFF_ENABLE = 0x22;
constexpr uint8_t REG_DCDC_PROTECTION = 0x23;
constexpr uint8_t REG_VSYS_POWEROFF = 0x24;
constexpr uint8_t REG_PWROK_CONFIG = 0x25;
constexpr uint8_t REG_SLEEP_WAKE = 0x26;
constexpr uint8_t REG_BUTTON_TIMING = 0x27;
constexpr uint8_t REG_ADC_ENABLE = 0x30;
constexpr uint8_t REG_VBAT_HIGH = 0x34;
constexpr uint8_t REG_VBUS_HIGH = 0x38;
constexpr uint8_t REG_VSYS_HIGH = 0x3A;
constexpr uint8_t REG_TDIE_HIGH = 0x3C;
constexpr uint8_t REG_IRQ_ENABLE0 = 0x40;
constexpr uint8_t REG_IRQ_STATUS0 = 0x48;
constexpr uint8_t REG_PRECHARGE_CURRENT = 0x61;
constexpr uint8_t REG_CHARGE_CURRENT = 0x62;
constexpr uint8_t REG_TERMINATION_CURRENT = 0x63;
constexpr uint8_t REG_CHARGE_VOLTAGE = 0x64;
constexpr uint8_t REG_BATTERY_DETECTION = 0x68;
constexpr uint8_t REG_CHARGE_LED = 0x69;
constexpr uint8_t REG_DCDC_ENABLE = 0x80;
constexpr uint8_t REG_DCDC_MODE = 0x81;
constexpr uint8_t REG_DCDC1_VOLTAGE = 0x82;
constexpr uint8_t REG_LDO_ENABLE0 = 0x90;
constexpr uint8_t REG_LDO_ENABLE1 = 0x91;
constexpr uint8_t REG_ALDO1_VOLTAGE = 0x92;
constexpr uint8_t REG_BATTERY_PERCENT = 0xA4;

const char *const CHARGE_STATES[] = {
    "trickle charge", "pre-charge", "constant current", "constant voltage",
    "charge done",    "not charging", "reserved",         "reserved"};

uint16_t decodeDcdc23(uint8_t raw) {
  const uint8_t code = raw & 0x7F;
  if (code <= 70) {
    return 500 + code * 10;
  }
  if (code <= 87) {
    return 1220 + (code - 71) * 20;
  }
  return 0;
}

uint16_t decodeDcdc3(uint8_t raw) {
  const uint8_t code = raw & 0x7F;
  if (code <= 70) {
    return 500 + code * 10;
  }
  if (code <= 87) {
    return 1220 + (code - 71) * 20;
  }
  if (code <= 106) {
    return 1600 + (code - 88) * 100;
  }
  return 0;
}

uint16_t decodeDcdc1(uint8_t raw) {
  const uint8_t code = raw & 0x1F;
  return code <= 19 ? 1500 + code * 100 : 0;
}

uint16_t decodeDcdc4(uint8_t raw) {
  const uint8_t code = raw & 0x7F;
  if (code <= 70) {
    return 500 + code * 10;
  }
  if (code <= 102) {
    return 1220 + (code - 71) * 20;
  }
  return 0;
}

uint16_t decodeDcdc5(uint8_t raw) {
  const uint8_t code = raw & 0x1F;
  if (code == 0) {
    return 1200;
  }
  if (code <= 24) {
    return 1300 + code * 100;
  }
  return 0;
}

uint16_t decodeLdo100mv(uint8_t raw) {
  const uint8_t code = raw & 0x1F;
  return code <= 30 ? 500 + code * 100 : 0;
}

uint16_t decodeLdo50mv(uint8_t raw) {
  const uint8_t code = raw & 0x1F;
  return code <= 19 ? 500 + code * 50 : 0;
}

uint16_t decodeDldo1(uint8_t raw) {
  const uint8_t code = raw & 0x1F;
  return code <= 28 ? 500 + code * 100 : 0;
}
} // namespace

AXP2101::AXP2101(TwoWire &wire, uint8_t address, uint8_t sdaPin,
                 uint8_t sclPin)
    : wire_(wire), address_(address), sdaPin_(sdaPin), sclPin_(sclPin) {}

bool AXP2101::begin(Stream &log) {
  log_ = &log;
  log_->print("AXP2101 I2C init: SDA=GPIO");
  log_->print(sdaPin_);
  log_->print(", SCL=GPIO");
  log_->print(sclPin_);
  log_->print(", address=0x");
  log_->println(address_, HEX);

  if (!wire_.begin(sdaPin_, sclPin_, 100000)) {
    log_->println("AXP2101 I2C bus init failed");
    return false;
  }
  wire_.setClock(100000);

  wire_.beginTransmission(address_);
  const uint8_t error = wire_.endTransmission();
  if (error != 0) {
    log_->print("AXP2101 not found, I2C error=");
    log_->println(error);
    return false;
  }

  log_->println("AXP2101 ACK OK");
  return true;
}

bool AXP2101::setLinearChargerVsysDpm(uint16_t millivolts) {
  if (log_ == nullptr || millivolts < 4100 || millivolts > 4800 ||
      millivolts % 100 != 0) {
    return false;
  }

  uint8_t current = 0;
  if (!readReg(REG_MIN_SYS_VOLTAGE, current)) {
    log_->println("AXP2101 failed to read REG14 before VSYS DPM update");
    return false;
  }

  const uint8_t code = static_cast<uint8_t>((millivolts - 4100) / 100);
  const uint8_t updated =
      static_cast<uint8_t>((current & ~0x70) | ((code & 0x07) << 4));

  log_->print("AXP2101 Linear Charger VSYS DPM: ");
  log_->print(4100 + ((current >> 4) & 0x07) * 100);
  log_->print("mV (REG14=0x");
  printHex8(current);
  log_->print(") -> ");
  log_->print(millivolts);
  log_->print("mV (REG14=0x");
  printHex8(updated);
  log_->println(')');

  if (updated != current && !writeReg(REG_MIN_SYS_VOLTAGE, updated)) {
    log_->println("AXP2101 failed to write Linear Charger VSYS DPM");
    return false;
  }

  uint8_t verify = 0;
  if (!readReg(REG_MIN_SYS_VOLTAGE, verify) || verify != updated) {
    log_->println("AXP2101 Linear Charger VSYS DPM verify failed");
    return false;
  }

  log_->println("AXP2101 Linear Charger VSYS DPM update OK");
  return true;
}

bool AXP2101::setBatteryDetection(bool enabled) {
  if (log_ == nullptr) {
    return false;
  }

  uint8_t current = 0;
  if (!readReg(REG_BATTERY_DETECTION, current)) {
    log_->println("AXP2101 failed to read battery detection control");
    return false;
  }

  const uint8_t updated =
      enabled ? static_cast<uint8_t>(current | bit(0))
              : static_cast<uint8_t>(current & ~bit(0));
  if (updated != current && !writeReg(REG_BATTERY_DETECTION, updated)) {
    log_->println("AXP2101 failed to update battery detection control");
    return false;
  }

  uint8_t verify = 0;
  if (!readReg(REG_BATTERY_DETECTION, verify) ||
      ((verify & bit(0)) != (updated & bit(0)))) {
    log_->println("AXP2101 battery detection verify failed");
    return false;
  }

  log_->print("AXP2101 battery detection: ");
  log_->print(enabled ? "ENABLED" : "DISABLED");
  log_->print(" (REG68=0x");
  printHex8(verify);
  log_->println(')');
  return true;
}

bool AXP2101::setDcdc5Enabled(bool enabled) {
  if (log_ == nullptr) {
    return false;
  }

  uint8_t current = 0;
  if (!readReg(REG_DCDC_ENABLE, current)) {
    log_->println("AXP2101 failed to read DCDC enable control");
    return false;
  }

  const uint8_t updated =
      enabled ? static_cast<uint8_t>(current | bit(4))
              : static_cast<uint8_t>(current & ~bit(4));
  if (updated != current && !writeReg(REG_DCDC_ENABLE, updated)) {
    log_->println("AXP2101 failed to update DCDC5 state");
    return false;
  }

  uint8_t verify = 0;
  if (!readReg(REG_DCDC_ENABLE, verify) ||
      ((verify & bit(4)) != (updated & bit(4)))) {
    log_->println("AXP2101 DCDC5 state verify failed");
    return false;
  }

  log_->print("AXP2101 DCDC5: ");
  log_->print(enabled ? "ENABLED" : "DISABLED");
  log_->print(" (REG80=0x");
  printHex8(verify);
  log_->println(')');
  return true;
}

bool AXP2101::setDcdc3Voltage(uint16_t millivolts) {
  if (log_ == nullptr) {
    return false;
  }

  uint8_t code = 0;
  if (millivolts >= 500 && millivolts <= 1200 &&
      (millivolts - 500) % 10 == 0) {
    code = static_cast<uint8_t>((millivolts - 500) / 10);
  } else if (millivolts >= 1220 && millivolts <= 1540 &&
             (millivolts - 1220) % 20 == 0) {
    code = static_cast<uint8_t>(71 + (millivolts - 1220) / 20);
  } else if (millivolts >= 1600 && millivolts <= 3400 &&
             (millivolts - 1600) % 100 == 0) {
    code = static_cast<uint8_t>(88 + (millivolts - 1600) / 100);
  } else {
    log_->println("AXP2101 invalid DCDC3 voltage");
    return false;
  }

  uint8_t dcdcEnable = 0;
  if (!readReg(REG_DCDC_ENABLE, dcdcEnable) ||
      !writeReg(REG_DCDC_ENABLE,
                static_cast<uint8_t>(dcdcEnable | bit(2)))) {
    log_->println("AXP2101 failed to enable DCDC3");
    return false;
  }

  uint8_t current = 0;
  if (!readReg(static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 2), current)) {
    log_->println("AXP2101 failed to read DCDC3 voltage");
    return false;
  }

  const uint8_t updated = static_cast<uint8_t>((current & bit(7)) | code);
  if (!writeReg(static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 2), updated)) {
    log_->println("AXP2101 failed to set DCDC3 voltage");
    return false;
  }

  uint8_t verify = 0;
  if (!readReg(static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 2), verify) ||
      verify != updated) {
    log_->println("AXP2101 DCDC3 voltage verify failed");
    return false;
  }

  log_->print("AXP2101 DCDC3 voltage: ");
  log_->print(millivolts);
  log_->print("mV (REG84=0x");
  printHex8(verify);
  log_->println(')');
  return true;
}

bool AXP2101::setLongPressPowerOff(uint8_t seconds) {
  if (log_ == nullptr) {
    return false;
  }

  uint8_t timingCode = 0;
  switch (seconds) {
  case 4:
    timingCode = 0;
    break;
  case 6:
    timingCode = 1;
    break;
  case 8:
    timingCode = 2;
    break;
  case 10:
    timingCode = 3;
    break;
  default:
    log_->println("AXP2101 power-off hold time must be 4, 6, 8 or 10s");
    return false;
  }

  uint8_t powerOffEnable = 0;
  uint8_t buttonTiming = 0;
  if (!readReg(REG_POWEROFF_ENABLE, powerOffEnable) ||
      !readReg(REG_BUTTON_TIMING, buttonTiming)) {
    log_->println("AXP2101 failed to read power button configuration");
    return false;
  }

  // Enable long-press shutdown and select power-off instead of restart.
  powerOffEnable =
      static_cast<uint8_t>((powerOffEnable | bit(1)) & ~bit(0));
  buttonTiming =
      static_cast<uint8_t>((buttonTiming & ~0x0C) | (timingCode << 2));

  if (!writeReg(REG_POWEROFF_ENABLE, powerOffEnable) ||
      !writeReg(REG_BUTTON_TIMING, buttonTiming)) {
    log_->println("AXP2101 failed to update power button configuration");
    return false;
  }

  uint8_t verifyEnable = 0;
  uint8_t verifyTiming = 0;
  if (!readReg(REG_POWEROFF_ENABLE, verifyEnable) ||
      !readReg(REG_BUTTON_TIMING, verifyTiming) ||
      !(verifyEnable & bit(1)) || (verifyEnable & bit(0)) ||
      ((verifyTiming >> 2) & 0x03) != timingCode) {
    log_->println("AXP2101 power button configuration verify failed");
    return false;
  }

  log_->print("AXP2101 long-press power-off: ");
  log_->print(seconds);
  log_->println("s");
  return true;
}

bool AXP2101::clearIrqStatus() {
  if (log_ == nullptr) {
    return false;
  }

  // IRQ status registers are write-one-to-clear.
  for (uint8_t reg = REG_IRQ_STATUS0; reg <= REG_IRQ_STATUS0 + 2; reg++) {
    if (!writeReg(reg, 0xFF)) {
      log_->println("AXP2101 failed to clear IRQ status");
      return false;
    }
  }

  powerSnapshotValid_ = false;
  lastIrq0_ = 0;
  lastIrq1_ = 0;
  lastIrq2_ = 0;
  log_->println("AXP2101 historical IRQ status cleared");
  return true;
}

bool AXP2101::ensureRuntimeConfig() {
  uint8_t dcdcEnable = 0;
  uint8_t dcdc3Voltage = 0;
  if (!readReg(REG_DCDC_ENABLE, dcdcEnable) ||
      !readReg(static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 2), dcdc3Voltage)) {
    return false;
  }

  const bool dcdc3Enabled = (dcdcEnable & bit(2)) != 0;
  const bool dcdc5Disabled = (dcdcEnable & bit(4)) == 0;
  const bool dcdc3Is3300Mv = (dcdc3Voltage & 0x7F) == 0x69;
  if (dcdc3Enabled && dcdc5Disabled && dcdc3Is3300Mv) {
    return true;
  }

  log_->println("AXP2101 runtime configuration changed; restoring outputs");
  return setDcdc5Enabled(false) && setDcdc3Voltage(3300);
}

bool AXP2101::printCurrentConfig() {
  if (log_ == nullptr) {
    return false;
  }

  uint8_t r[256] = {};
  const uint8_t registers[] = {
      REG_STATUS1,
      REG_STATUS2,
      REG_COMMON_CONFIG,
      REG_BATFET_CONTROL,
      REG_MIN_SYS_VOLTAGE,
      REG_INPUT_VOLTAGE_LIMIT,
      REG_INPUT_CURRENT_LIMIT,
      REG_MODULE_ENABLE,
      REG_LOW_BATTERY_WARNING,
      REG_POWERON_STATUS,
      REG_POWEROFF_STATUS,
      REG_POWEROFF_ENABLE,
      REG_DCDC_PROTECTION,
      REG_VSYS_POWEROFF,
      REG_PWROK_CONFIG,
      REG_SLEEP_WAKE,
      REG_BUTTON_TIMING,
      REG_ADC_ENABLE,
      REG_IRQ_ENABLE0,
      static_cast<uint8_t>(REG_IRQ_ENABLE0 + 1),
      static_cast<uint8_t>(REG_IRQ_ENABLE0 + 2),
      REG_IRQ_STATUS0,
      static_cast<uint8_t>(REG_IRQ_STATUS0 + 1),
      static_cast<uint8_t>(REG_IRQ_STATUS0 + 2),
      REG_PRECHARGE_CURRENT,
      REG_CHARGE_CURRENT,
      REG_TERMINATION_CURRENT,
      REG_CHARGE_VOLTAGE,
      REG_BATTERY_DETECTION,
      REG_CHARGE_LED,
      REG_DCDC_ENABLE,
      REG_DCDC_MODE,
      REG_DCDC1_VOLTAGE,
      static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 1),
      static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 2),
      static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 3),
      static_cast<uint8_t>(REG_DCDC1_VOLTAGE + 4),
      REG_LDO_ENABLE0,
      REG_LDO_ENABLE1,
      REG_ALDO1_VOLTAGE,
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 1),
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 2),
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 3),
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 4),
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 5),
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 6),
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 7),
      static_cast<uint8_t>(REG_ALDO1_VOLTAGE + 8),
      REG_BATTERY_PERCENT,
  };

  for (const uint8_t reg : registers) {
    if (!readReg(reg, r[reg])) {
      log_->print("AXP2101 read failed at register 0x");
      printHex8(reg);
      log_->println();
      return false;
    }
  }

  log_->println();
  log_->println("========== AXP2101 current configuration (read-only) ==========");

  const uint8_t status1 = r[REG_STATUS1];
  const uint8_t status2 = r[REG_STATUS2];
  log_->print("Power: VBUS=");
  log_->print(status1 & bit(5) ? "good" : "not good");
  log_->print(", battery=");
  log_->print(status1 & bit(3) ? "present" : "absent");
  log_->print(", system=");
  log_->print(status2 & bit(4) ? "on" : "off");
  log_->print(", charge=");
  log_->println(CHARGE_STATES[status2 & 0x07]);

  const uint8_t direction = (status2 >> 5) & 0x03;
  log_->print("Battery direction: ");
  log_->println(direction == 1   ? "charging"
                : direction == 2 ? "discharging"
                                 : "standby");
  log_->print("Limits: current-limit=");
  log_->print(status1 & bit(0) ? "active" : "inactive");
  log_->print(", VINDPM=");
  log_->print(status2 & bit(3) ? "active" : "inactive");
  log_->print(", thermal-regulation=");
  log_->println(status1 & bit(1) ? "active" : "inactive");

  const uint16_t minSystemMv = 3200 + (r[REG_MIN_SYS_VOLTAGE] & 0x07) * 100;
  const uint16_t inputVoltageMv =
      3880 + (r[REG_INPUT_VOLTAGE_LIMIT] & 0x0F) * 80;
  const uint16_t inputCurrentValues[] = {100, 500, 900, 1000, 1500, 2000, 0, 0};
  log_->print("Input: VINDPM=");
  log_->print(inputVoltageMv);
  log_->print("mV, current-limit=");
  log_->print(inputCurrentValues[r[REG_INPUT_CURRENT_LIMIT] & 0x07]);
  log_->print("mA, minimum-system=");
  log_->print(minSystemMv);
  log_->println("mV");

  const uint8_t modules = r[REG_MODULE_ENABLE];
  log_->print("Modules: charger=");
  log_->print(modules & bit(1) ? "on" : "off");
  log_->print(", fuel-gauge=");
  log_->print(modules & bit(3) ? "on" : "off");
  log_->print(", watchdog=");
  log_->println(modules & bit(0) ? "on" : "off");

  const uint8_t prechargeCode = r[REG_PRECHARGE_CURRENT] & 0x0F;
  const uint8_t chargeCode = r[REG_CHARGE_CURRENT] & 0x1F;
  const uint8_t terminationCode = r[REG_TERMINATION_CURRENT] & 0x0F;
  const uint16_t chargeCurrentMa =
      chargeCode <= 8 ? chargeCode * 25 : 200 + (chargeCode - 8) * 100;
  const uint16_t chargeVoltages[] = {0, 4000, 4100, 4200,
                                     4350, 4400, 0,    0};
  log_->print("Charger: precharge=");
  log_->print(prechargeCode <= 8 ? prechargeCode * 25 : 0);
  log_->print("mA, constant-current=");
  log_->print(chargeCode <= 21 ? chargeCurrentMa : 0);
  log_->print("mA, termination=");
  log_->print(terminationCode <= 8 ? terminationCode * 25 : 0);
  log_->print("mA (");
  log_->print(r[REG_TERMINATION_CURRENT] & bit(4) ? "enabled" : "disabled");
  log_->print("), CV=");
  log_->print(chargeVoltages[r[REG_CHARGE_VOLTAGE] & 0x07]);
  log_->println("mV");

  log_->print("Battery warning: level1=");
  log_->print(r[REG_LOW_BATTERY_WARNING] & 0x0F);
  log_->print("%, level2=");
  log_->print(5 + ((r[REG_LOW_BATTERY_WARNING] >> 4) & 0x0F));
  log_->println("%");

  const uint8_t powerOffEnable = r[REG_POWEROFF_ENABLE];
  const uint8_t buttonTiming = r[REG_BUTTON_TIMING];
  const uint8_t offLevelSeconds[] = {4, 6, 8, 10};
  const char *const onLevelTimes[] = {"128ms", "512ms", "1s", "2s"};
  log_->println("Power button / shutdown:");
  log_->print("  Long-press shutdown: ");
  log_->println(powerOffEnable & bit(1) ? "ENABLED" : "DISABLED");
  log_->print("  Long-press action: ");
  log_->println(powerOffEnable & bit(0) ? "restart" : "power off");
  log_->print("  OFFLEVEL hold time: ");
  log_->print(offLevelSeconds[(buttonTiming >> 2) & 0x03]);
  log_->println("s");
  log_->print("  ONLEVEL hold time: ");
  log_->println(onLevelTimes[buttonTiming & 0x03]);
  log_->print("  Emergency 16s shutdown: ");
  log_->println(r[REG_COMMON_CONFIG] & bit(2) ? "ENABLED" : "DISABLED");
  log_->print("  Internal output discharge after power-off: ");
  log_->println(r[REG_COMMON_CONFIG] & bit(5) ? "ENABLED" : "DISABLED");
  log_->print("  BATFET allowed while powered off on battery: ");
  log_->println(r[REG_BATFET_CONTROL] & bit(3) ? "YES" : "NO");
  log_->print("  VSYS under-voltage power-off threshold: ");
  log_->print(2600 + (r[REG_VSYS_POWEROFF] & 0x07) * 100);
  log_->println("mV");

  log_->print("  Last power-on sources (REG 0x20):");
  const uint8_t powerOn = r[REG_POWERON_STATUS];
  if (powerOn == 0) {
    log_->print(" none latched");
  }
  if (powerOn & bit(5)) {
    log_->print(" EN-high");
  }
  if (powerOn & bit(4)) {
    log_->print(" battery-insert");
  }
  if (powerOn & bit(3)) {
    log_->print(" battery-charged-above-3.3V");
  }
  if (powerOn & bit(2)) {
    log_->print(" VBUS-insert");
  }
  if (powerOn & bit(1)) {
    log_->print(" IRQ-low");
  }
  if (powerOn & bit(0)) {
    log_->print(" power-button");
  }
  log_->println();

  log_->print("  Last power-off sources (REG 0x21):");
  const uint8_t powerOff = r[REG_POWEROFF_STATUS];
  if (powerOff == 0) {
    log_->print(" none latched");
  }
  if (powerOff & bit(7)) {
    log_->print(" die-over-temperature");
  }
  if (powerOff & bit(6)) {
    log_->print(" DCDC-over-voltage");
  }
  if (powerOff & bit(5)) {
    log_->print(" DCDC-under-voltage");
  }
  if (powerOff & bit(4)) {
    log_->print(" VBUS-over-voltage");
  }
  if (powerOff & bit(3)) {
    log_->print(" VSYS-under-voltage");
  }
  if (powerOff & bit(2)) {
    log_->print(" EN-low");
  }
  if (powerOff & bit(1)) {
    log_->print(" software");
  }
  if (powerOff & bit(0)) {
    log_->print(" power-button");
  }
  log_->println();
  log_->print("  DCDC OVP/UVP shutdown mask: 0x");
  printHex8(r[REG_DCDC_PROTECTION]);
  log_->println();

  uint16_t adc = 0;
  log_->print("Measurements: SOC=");
  log_->print(r[REG_BATTERY_PERCENT]);
  log_->print("%");
  if ((r[REG_ADC_ENABLE] & bit(0)) && readAdc(REG_VBAT_HIGH, adc)) {
    log_->print(", VBAT=");
    log_->print(adc);
    log_->print("mV");
  }
  if ((r[REG_ADC_ENABLE] & bit(2)) && readAdc(REG_VBUS_HIGH, adc)) {
    log_->print(", VBUS=");
    log_->print(adc);
    log_->print("mV");
  }
  if ((r[REG_ADC_ENABLE] & bit(3)) && readAdc(REG_VSYS_HIGH, adc)) {
    log_->print(", VSYS=");
    log_->print(adc);
    log_->print("mV");
  }
  if ((r[REG_ADC_ENABLE] & bit(4)) && readAdc(REG_TDIE_HIGH, adc)) {
    log_->print(", TDIE_ADC=");
    log_->print(adc / 10.0f, 1);
    log_->print("mV");
  }
  log_->println();

  const uint8_t dcdcEnable = r[REG_DCDC_ENABLE];
  printRail("DCDC1", dcdcEnable & bit(0), decodeDcdc1(r[0x82]), 0x82,
            r[0x82]);
  printRail("DCDC2", dcdcEnable & bit(1), decodeDcdc23(r[0x83]), 0x83,
            r[0x83]);
  printRail("DCDC3", dcdcEnable & bit(2), decodeDcdc3(r[0x84]), 0x84,
            r[0x84]);
  printRail("DCDC4", dcdcEnable & bit(3), decodeDcdc4(r[0x85]), 0x85,
            r[0x85]);
  printRail("DCDC5", dcdcEnable & bit(4), decodeDcdc5(r[0x86]), 0x86,
            r[0x86]);

  const uint8_t ldoEnable = r[REG_LDO_ENABLE0];
  printRail("ALDO1", ldoEnable & bit(0), decodeLdo100mv(r[0x92]), 0x92,
            r[0x92]);
  printRail("ALDO2", ldoEnable & bit(1), decodeLdo100mv(r[0x93]), 0x93,
            r[0x93]);
  printRail("ALDO3", ldoEnable & bit(2), decodeLdo100mv(r[0x94]), 0x94,
            r[0x94]);
  printRail("ALDO4", ldoEnable & bit(3), decodeLdo100mv(r[0x95]), 0x95,
            r[0x95]);
  printRail("BLDO1", ldoEnable & bit(4), decodeLdo100mv(r[0x96]), 0x96,
            r[0x96]);
  printRail("BLDO2", ldoEnable & bit(5), decodeLdo100mv(r[0x97]), 0x97,
            r[0x97]);
  printRail("CPUSLDO", ldoEnable & bit(6), decodeLdo50mv(r[0x98]), 0x98,
            r[0x98]);
  printRail("DLDO1", ldoEnable & bit(7), decodeDldo1(r[0x99]), 0x99,
            r[0x99]);
  printRail("DLDO2", r[REG_LDO_ENABLE1] & bit(0), decodeLdo50mv(r[0x9A]),
            0x9A, r[0x9A]);

  log_->println("Raw configuration registers:");
  for (const uint8_t reg : registers) {
    printRegister("", reg, r[reg]);
  }
  log_->println("===============================================================");
  log_->println();
  return true;
}

bool AXP2101::pollPowerLog(bool force) {
  if (log_ == nullptr) {
    return false;
  }

  uint8_t status1 = 0;
  uint8_t status2 = 0;
  uint8_t powerOn = 0;
  uint8_t powerOff = 0;
  uint8_t irq0 = 0;
  uint8_t irq1 = 0;
  uint8_t irq2 = 0;
  uint8_t adcEnable = 0;
  uint8_t soc = 0;

  if (!readReg(REG_STATUS1, status1) || !readReg(REG_STATUS2, status2) ||
      !readReg(REG_POWERON_STATUS, powerOn) ||
      !readReg(REG_POWEROFF_STATUS, powerOff) ||
      !readReg(REG_IRQ_STATUS0, irq0) ||
      !readReg(static_cast<uint8_t>(REG_IRQ_STATUS0 + 1), irq1) ||
      !readReg(static_cast<uint8_t>(REG_IRQ_STATUS0 + 2), irq2) ||
      !readReg(REG_ADC_ENABLE, adcEnable) ||
      !readReg(REG_BATTERY_PERCENT, soc)) {
    log_->println("[AXP2101] power monitor read failed");
    powerSnapshotValid_ = false;
    return false;
  }

  uint16_t vbatMv = 0;
  const bool hasVbat =
      (adcEnable & bit(0)) != 0 && readAdc(REG_VBAT_HIGH, vbatMv);
  const bool changed =
      !powerSnapshotValid_ || status1 != lastStatus1_ ||
      status2 != lastStatus2_ || powerOn != lastPowerOn_ ||
      powerOff != lastPowerOff_ || irq0 != lastIrq0_ || irq1 != lastIrq1_ ||
      irq2 != lastIrq2_ || soc != lastSoc_ ||
      (hasVbat && (vbatMv > lastVbatMv_ + 10 || vbatMv + 10 < lastVbatMv_));
  const bool heartbeatDue = millis() - lastPowerLogMs_ >= 5000;

  if (force || changed || heartbeatDue) {
    printPowerSnapshot(status1, status2, powerOn, powerOff, irq0, irq1, irq2,
                       soc, hasVbat, vbatMv, powerSnapshotValid_, lastIrq1_);
    lastPowerLogMs_ = millis();
  }

  powerSnapshotValid_ = true;
  lastStatus1_ = status1;
  lastStatus2_ = status2;
  lastPowerOn_ = powerOn;
  lastPowerOff_ = powerOff;
  lastIrq0_ = irq0;
  lastIrq1_ = irq1;
  lastIrq2_ = irq2;
  lastSoc_ = soc;
  if (hasVbat) {
    lastVbatMv_ = vbatMv;
  }
  return true;
}

bool AXP2101::readReg(uint8_t reg, uint8_t &value) {
  wire_.beginTransmission(address_);
  wire_.write(reg);
  if (wire_.endTransmission(false) != 0) {
    return false;
  }
  if (wire_.requestFrom(address_, static_cast<uint8_t>(1)) != 1) {
    return false;
  }
  value = wire_.read();
  return true;
}

bool AXP2101::writeReg(uint8_t reg, uint8_t value) {
  wire_.beginTransmission(address_);
  wire_.write(reg);
  wire_.write(value);
  return wire_.endTransmission() == 0;
}

void AXP2101::printPowerSnapshot(uint8_t status1, uint8_t status2,
                                 uint8_t powerOn, uint8_t powerOff,
                                 uint8_t irq0, uint8_t irq1, uint8_t irq2,
                                 uint8_t soc, bool hasVbat,
                                 uint16_t vbatMv, bool previousValid,
                                 uint8_t previousIrq1) {
  log_->print("[AXP2101 ");
  log_->print(millis());
  log_->print("ms] VBUS=");
  log_->print(status1 & bit(5) ? "GOOD" : "BAD");
  log_->print(" BAT=");
  log_->print(status1 & bit(3) ? "PRESENT" : "ABSENT");
  log_->print(" BATFET=");
  log_->print(status1 & bit(4) ? "OPEN" : "CLOSED");
  log_->print(" SYS=");
  log_->print(status2 & bit(4) ? "ON" : "OFF");
  log_->print(" DIR=");

  const uint8_t direction = (status2 >> 5) & 0x03;
  log_->print(direction == 1   ? "CHARGE"
              : direction == 2 ? "DISCHARGE"
                               : "STANDBY");
  log_->print(" CHG=");
  log_->print(CHARGE_STATES[status2 & 0x07]);
  log_->print(" SOC=");
  log_->print(soc);
  log_->print('%');
  if (hasVbat) {
    log_->print(" VBAT=");
    log_->print(vbatMv);
    log_->print("mV");
  } else {
    log_->print(" VBAT=ADC_OFF");
  }
  log_->print(" PON=0x");
  printHex8(powerOn);
  log_->print(" POFF=0x");
  printHex8(powerOff);
  log_->print(" IRQ=0x");
  printHex8(irq0);
  log_->print('/');
  printHex8(irq1);
  log_->print('/');
  printHex8(irq2);
  log_->println();

  if ((status1 & bit(5)) && !(status1 & bit(3))) {
    log_->println(
        "  Diagnostic: VBUS is valid while battery is absent; monitor whether "
        "the system remains powered.");
  }
  if (!previousValid && irq1 != 0) {
    log_->println(
        "  Note: IRQ bits are historical latched flags and are not cleared.");
  }

  const uint8_t newIrq1 = previousValid ? irq1 & ~previousIrq1 : 0;
  if (newIrq1 & bit(7)) {
    log_->println("  New event: VBUS inserted");
  }
  if (newIrq1 & bit(6)) {
    log_->println("  New event: VBUS removed");
  }
  if (newIrq1 & bit(5)) {
    log_->println("  New event: battery inserted");
  }
  if (newIrq1 & bit(4)) {
    log_->println("  New event: battery removed");
  }
  if (newIrq1 & bit(3)) {
    log_->println("  New event: POWERON short press");
  }
  if (newIrq1 & bit(2)) {
    log_->println("  New event: POWERON long press");
  }
}

bool AXP2101::readAdc(uint8_t highReg, uint16_t &value) {
  uint8_t high = 0;
  uint8_t low = 0;
  if (!readReg(highReg, high) ||
      !readReg(static_cast<uint8_t>(highReg + 1), low)) {
    return false;
  }
  value = (static_cast<uint16_t>(high & 0x3F) << 8) | low;
  return true;
}

void AXP2101::printRegister(const char *name, uint8_t reg, uint8_t value) {
  log_->print("  ");
  if (name[0] != '\0') {
    log_->print(name);
    log_->print(' ');
  }
  log_->print("REG[0x");
  printHex8(reg);
  log_->print("] = 0x");
  printHex8(value);
  log_->println();
}

void AXP2101::printRail(const char *name, bool enabled, uint16_t millivolts,
                        uint8_t reg, uint8_t raw) {
  log_->print("Rail ");
  log_->print(name);
  log_->print(": ");
  log_->print(enabled ? "ON " : "OFF ");
  if (millivolts != 0) {
    log_->print(millivolts);
    log_->print("mV");
  } else {
    log_->print("reserved voltage code");
  }
  log_->print(" (REG 0x");
  printHex8(reg);
  log_->print("=0x");
  printHex8(raw);
  log_->println(')');
}

void AXP2101::printHex8(uint8_t value) {
  if (value < 0x10) {
    log_->print('0');
  }
  log_->print(value, HEX);
}
