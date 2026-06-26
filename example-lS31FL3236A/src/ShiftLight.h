#pragma once

#include <Arduino.h>
#include <Wire.h>

class ShiftLight {
public:
  static constexpr uint8_t LedCount = 8;
  static constexpr uint8_t MaxBrightness = 255;

  ShiftLight(TwoWire &wire, uint8_t address, uint8_t sdaPin, uint8_t sclPin);

  bool init(Stream &log = Serial);
  bool setChannel(uint8_t channel, uint8_t brightness, bool autoShow = false);
  bool setLedRgb(uint8_t ledIndex, uint8_t red, uint8_t green, uint8_t blue,
                 bool autoShow = false);
  bool setAllRgb(uint8_t red, uint8_t green, uint8_t blue,
                 bool autoShow = true);
  bool clear(bool autoShow = true);
  bool show();
  void setVerboseLogging(bool enabled);

  bool playSafeFinishBreathing(uint16_t durationMs = 3000,
                               uint16_t periodMs = 1000);
  bool playVictoryBurstRainbow(uint8_t burstCount = 3,
                               uint16_t rainbowDurationMs = 2200);
  bool playSlowLapWarning(uint8_t repeatCount = 3);

private:
  static constexpr uint8_t ChannelCount = 36;

  TwoWire &wire_;
  uint8_t address_;
  uint8_t sdaPin_;
  uint8_t sclPin_;
  Stream *log_ = nullptr;
  bool verboseLogging_ = true;

  bool checkAck();
  bool writeReg(uint8_t reg, uint8_t value);
  bool setWheelColor(uint8_t ledIndex, uint8_t wheelPosition,
                     uint8_t brightness, bool autoShow = false);
  void log(const char *message);
};
