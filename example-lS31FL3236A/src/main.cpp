#include <Arduino.h>
#include <Wire.h>

#include "AXP2101.h"
#include "ShiftLight.h"

constexpr uint8_t I2C_SDA_PIN = 5;
constexpr uint8_t I2C_SCL_PIN = 6;
constexpr uint8_t IS31_ADDR = 0x3C;
constexpr uint8_t AXP2101_SDA_PIN = 8;
constexpr uint8_t AXP2101_SCL_PIN = 9;

ShiftLight shiftLight(Wire, IS31_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
TwoWire axp2101Wire(1);
AXP2101 axp2101(axp2101Wire, AXP2101::DefaultAddress, AXP2101_SDA_PIN,
                AXP2101_SCL_PIN);
bool shiftLightReady = false;
bool axp2101Ready = false;
uint32_t lastAxpConfigCheckMs = 0;

void maintainAxp2101() {
  if (!axp2101Ready) {
    return;
  }

  axp2101.pollPowerLog();
  if (millis() - lastAxpConfigCheckMs >= 1000) {
    axp2101.ensureRuntimeConfig();
    lastAxpConfigCheckMs = millis();
  }
}

void monitoredDelay(uint32_t durationMs) {
  const uint32_t startedAt = millis();
  while (millis() - startedAt < durationMs) {
    maintainAxp2101();
    delay(250);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  if (axp2101.begin(Serial)) {
    axp2101Ready = true;
    axp2101.setLinearChargerVsysDpm(4700);
    axp2101.setBatteryDetection(true);
    axp2101.setDcdc5Enabled(false);
    axp2101.setDcdc3Voltage(3300);
    axp2101.setLongPressPowerOff(4);
    axp2101.printCurrentConfig();
    axp2101.clearIrqStatus();
    axp2101.pollPowerLog(true);
  }

  shiftLight.setVerboseLogging(false);
  if (!shiftLight.init(Serial)) {
    Serial.println("ShiftLight init failed");
    return;
  }

  shiftLightReady = true;
}

void loop() {
  if (!shiftLightReady) {
    monitoredDelay(1000);
    return;
  }

  maintainAxp2101();
  shiftLight.playSafeFinishBreathing();
  maintainAxp2101();

  monitoredDelay(3000);

  shiftLight.playVictoryBurstRainbow();
  maintainAxp2101();

  monitoredDelay(3000);

  shiftLight.playSlowLapWarning();
  maintainAxp2101();

  monitoredDelay(3000);
}
