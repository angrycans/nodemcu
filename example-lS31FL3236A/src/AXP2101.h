#pragma once

#include <Arduino.h>
#include <Wire.h>

class AXP2101 {
public:
  static constexpr uint8_t DefaultAddress = 0x34;

  AXP2101(TwoWire &wire, uint8_t address, uint8_t sdaPin, uint8_t sclPin);

  bool begin(Stream &log = Serial);
  bool setLinearChargerVsysDpm(uint16_t millivolts);
  bool setBatteryDetection(bool enabled);
  bool setDcdc5Enabled(bool enabled);
  bool setDcdc3Voltage(uint16_t millivolts);
  bool setLongPressPowerOff(uint8_t seconds);
  bool clearIrqStatus();
  bool ensureRuntimeConfig();
  bool printCurrentConfig();
  bool pollPowerLog(bool force = false);

private:
  TwoWire &wire_;
  uint8_t address_;
  uint8_t sdaPin_;
  uint8_t sclPin_;
  Stream *log_ = nullptr;
  bool powerSnapshotValid_ = false;
  uint8_t lastStatus1_ = 0;
  uint8_t lastStatus2_ = 0;
  uint8_t lastPowerOn_ = 0;
  uint8_t lastPowerOff_ = 0;
  uint8_t lastIrq0_ = 0;
  uint8_t lastIrq1_ = 0;
  uint8_t lastIrq2_ = 0;
  uint8_t lastSoc_ = 0;
  uint16_t lastVbatMv_ = 0;
  uint32_t lastPowerLogMs_ = 0;

  bool readReg(uint8_t reg, uint8_t &value);
  bool writeReg(uint8_t reg, uint8_t value);
  bool readAdc(uint8_t highReg, uint16_t &value);
  void printPowerSnapshot(uint8_t status1, uint8_t status2, uint8_t powerOn,
                          uint8_t powerOff, uint8_t irq0, uint8_t irq1,
                          uint8_t irq2, uint8_t soc, bool hasVbat,
                          uint16_t vbatMv, bool previousValid,
                          uint8_t previousIrq1);
  void printRegister(const char *name, uint8_t reg, uint8_t value);
  void printRail(const char *name, bool enabled, uint16_t millivolts,
                 uint8_t reg, uint8_t raw);
  void printHex8(uint8_t value);
};
