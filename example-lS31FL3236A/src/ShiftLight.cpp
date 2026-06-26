#include "ShiftLight.h"

#include <math.h>

namespace {
constexpr uint8_t REG_SHUTDOWN = 0x00;
constexpr uint8_t REG_PWM_BASE = 0x01;
constexpr uint8_t REG_PWM_UPDATE = 0x25;
constexpr uint8_t REG_LED_CTRL_BASE = 0x26;
constexpr uint8_t REG_GLOBAL_CTRL = 0x4A;
constexpr uint8_t REG_RESET = 0x4F;

constexpr float BREATH_TWO_PI = 6.28318530718f;
constexpr uint8_t EFFECT_PEAK_BRIGHTNESS = 255;
} // namespace

ShiftLight::ShiftLight(TwoWire &wire, uint8_t address, uint8_t sdaPin,
                       uint8_t sclPin)
    : wire_(wire), address_(address), sdaPin_(sdaPin), sclPin_(sclPin) {}

bool ShiftLight::init(Stream &log) {
  log_ = &log;

  wire_.begin(sdaPin_, sclPin_);
  wire_.setClock(100000);

  if (!checkAck()) {
    log_->println("ShiftLight init failed: no I2C ACK");
    return false;
  }
  writeReg(REG_RESET, 0x00);
  delay(10);
  writeReg(REG_SHUTDOWN, 0x01);
  writeReg(REG_GLOBAL_CTRL, 0x00);

  clear(false);
  show();

  return true;
}

bool ShiftLight::setChannel(uint8_t channel, uint8_t brightness,
                            bool autoShow) {
  if (channel < 1 || channel > ChannelCount) {
    log("Invalid channel");
    return false;
  }

  const bool pwmOk = writeReg(REG_PWM_BASE + channel - 1, brightness);
  const bool enableOk =
      writeReg(REG_LED_CTRL_BASE + channel - 1, brightness == 0 ? 0x00 : 0x01);

  if (autoShow) {
    show();
  }

  return pwmOk && enableOk;
}

bool ShiftLight::setLedRgb(uint8_t ledIndex, uint8_t red, uint8_t green,
                           uint8_t blue, bool autoShow) {
  if (ledIndex < 1 || ledIndex > LedCount) {
    log("Invalid LED index");
    return false;
  }

  const uint8_t baseChannel = (ledIndex - 1) * 3 + 1;

  // Board order is OUTn=R, OUTn+1=G, OUTn+2=B.
  const bool redOk = setChannel(baseChannel, red, false);
  const bool greenOk = setChannel(baseChannel + 1, green, false);
  const bool blueOk = setChannel(baseChannel + 2, blue, false);

  if (autoShow) {
    show();
  }

  return redOk && greenOk && blueOk;
}

bool ShiftLight::setAllRgb(uint8_t red, uint8_t green, uint8_t blue,
                           bool autoShow) {
  bool ok = true;
  for (uint8_t led = 1; led <= LedCount; led++) {
    ok = setLedRgb(led, red, green, blue, false) && ok;
  }

  if (autoShow) {
    show();
  }

  return ok;
}

bool ShiftLight::clear(bool autoShow) {
  bool ok = true;
  for (uint8_t channel = 1; channel <= ChannelCount; channel++) {
    ok = setChannel(channel, 0, false) && ok;
  }

  if (autoShow) {
    show();
  }

  return ok;
}

bool ShiftLight::show() {
  return writeReg(REG_PWM_UPDATE, 0x00);
}

void ShiftLight::setVerboseLogging(bool enabled) {
  verboseLogging_ = enabled;
}

bool ShiftLight::playSafeFinishBreathing(uint16_t durationMs,
                                         uint16_t periodMs) {
  if (periodMs == 0) {
    return false;
  }

  log("Effect: safe finish green breathing");

  const bool previousVerboseLogging = verboseLogging_;
  verboseLogging_ = false;

  constexpr uint8_t MIN_GREEN = 18;
  constexpr uint8_t MAX_GREEN = EFFECT_PEAK_BRIGHTNESS;
  constexpr uint8_t SETTLE_GREEN = 80;
  constexpr uint16_t FRAME_MS = 20;

  bool ok = true;
  const uint32_t startedAt = millis();

  while (millis() - startedAt < durationMs) {
    const uint32_t elapsed = millis() - startedAt;
    const float phase = static_cast<float>(elapsed % periodMs) / periodMs;
    const float wave = (1.0f - cosf(BREATH_TWO_PI * phase)) * 0.5f;
    const uint8_t green =
        MIN_GREEN + static_cast<uint8_t>((MAX_GREEN - MIN_GREEN) * wave);

    ok = setAllRgb(0, green, 0, true) && ok;
    delay(FRAME_MS);
  }

  ok = setAllRgb(0, SETTLE_GREEN, 0, true) && ok;
  verboseLogging_ = previousVerboseLogging;

  return ok;
}

bool ShiftLight::playVictoryBurstRainbow(uint8_t burstCount,
                                         uint16_t rainbowDurationMs) {
  log("Effect: victory purple burst rainbow");

  const bool previousVerboseLogging = verboseLogging_;
  verboseLogging_ = false;

  constexpr uint8_t PURPLE_R = EFFECT_PEAK_BRIGHTNESS;
  constexpr uint8_t PURPLE_G = 0;
  constexpr uint8_t PURPLE_B = 210;
  constexpr uint8_t RAINBOW_BRIGHTNESS = EFFECT_PEAK_BRIGHTNESS;
  constexpr uint16_t BURST_STEP_MS = 34;
  constexpr uint16_t BURST_GAP_MS = 70;
  constexpr uint16_t FRAME_MS = 24;

  bool ok = true;

  for (uint8_t burst = 0; burst < burstCount; burst++) {
    clear(true);
    delay(BURST_STEP_MS);

    ok = setLedRgb(4, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    ok = setLedRgb(5, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    show();
    delay(BURST_STEP_MS);

    ok = setLedRgb(3, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    ok = setLedRgb(6, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    show();
    delay(BURST_STEP_MS);

    ok = setLedRgb(2, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    ok = setLedRgb(7, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    show();
    delay(BURST_STEP_MS);

    ok = setLedRgb(1, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    ok = setLedRgb(8, PURPLE_R, PURPLE_G, PURPLE_B, false) && ok;
    show();
    delay(BURST_STEP_MS);

    clear(true);
    delay(BURST_GAP_MS);
  }

  const uint32_t startedAt = millis();
  while (millis() - startedAt < rainbowDurationMs) {
    const uint32_t elapsed = millis() - startedAt;
    const uint8_t flow = static_cast<uint8_t>((elapsed * 256UL) / 650UL);

    for (uint8_t led = 1; led <= LedCount; led++) {
      const uint8_t wheel =
          static_cast<uint8_t>(flow + (led - 1) * (256 / LedCount));
      ok = setWheelColor(led, wheel, RAINBOW_BRIGHTNESS, false) && ok;
    }

    show();
    delay(FRAME_MS);
  }

  verboseLogging_ = previousVerboseLogging;
  return ok;
}

bool ShiftLight::playSlowLapWarning(uint8_t repeatCount) {
  log("Effect: slow lap dark red double flash");

  const bool previousVerboseLogging = verboseLogging_;
  verboseLogging_ = false;

  constexpr uint8_t DARK_RED = 144;
  constexpr uint16_t FLASH_ON_MS = 90;
  constexpr uint16_t FLASH_OFF_MS = 120;
  constexpr uint16_t GROUP_GAP_MS = 280;
  constexpr uint16_t FADE_FRAME_MS = 35;

  bool ok = true;

  for (uint8_t repeat = 0; repeat < repeatCount; repeat++) {
    for (uint8_t flash = 0; flash < 2; flash++) {
      ok = setAllRgb(DARK_RED, 0, 0, true) && ok;
      delay(FLASH_ON_MS);
      ok = clear(true) && ok;
      delay(FLASH_OFF_MS);
    }

    delay(GROUP_GAP_MS);
  }

  for (int16_t red = DARK_RED; red >= 0; red -= 6) {
    ok = setAllRgb(static_cast<uint8_t>(red), 0, 0, true) && ok;
    delay(FADE_FRAME_MS);
  }

  ok = clear(true) && ok;
  verboseLogging_ = previousVerboseLogging;
  return ok;
}

bool ShiftLight::checkAck() {
  wire_.beginTransmission(address_);
  return wire_.endTransmission() == 0;
}

bool ShiftLight::writeReg(uint8_t reg, uint8_t value) {
  wire_.beginTransmission(address_);
  wire_.write(reg);
  wire_.write(value);
  const uint8_t error = wire_.endTransmission();

  if (log_ != nullptr && verboseLogging_) {
    log_->print("I2C write reg 0x");
    if (reg < 16) {
      log_->print('0');
    }
    log_->print(reg, HEX);
    log_->print(" = 0x");
    if (value < 16) {
      log_->print('0');
    }
    log_->print(value, HEX);
    log_->print(error == 0 ? " OK" : " FAIL, error=");
    if (error != 0) {
      log_->print(error);
    }
    log_->println();
  }

  return error == 0;
}

bool ShiftLight::setWheelColor(uint8_t ledIndex, uint8_t wheelPosition,
                               uint8_t brightness, bool autoShow) {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  if (wheelPosition < 85) {
    red = 255 - wheelPosition * 3;
    green = wheelPosition * 3;
  } else if (wheelPosition < 170) {
    wheelPosition -= 85;
    green = 255 - wheelPosition * 3;
    blue = wheelPosition * 3;
  } else {
    wheelPosition -= 170;
    blue = 255 - wheelPosition * 3;
    red = wheelPosition * 3;
  }

  red = static_cast<uint16_t>(red) * brightness / MaxBrightness;
  green = static_cast<uint16_t>(green) * brightness / MaxBrightness;
  blue = static_cast<uint16_t>(blue) * brightness / MaxBrightness;

  return setLedRgb(ledIndex, red, green, blue, autoShow);
}

void ShiftLight::log(const char *message) {
  if (log_ != nullptr && verboseLogging_) {
    log_->println(message);
  }
}
