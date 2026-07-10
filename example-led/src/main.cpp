#include "MAX7221.h"
#include <Arduino.h>

#define PIN_DIN 11
#define PIN_CLK 12
#define PIN_CS1 8
#define PIN_CS2 9
#define PIN_CS3 10

#define PIN_RS485_RX 18
#define PIN_RS485_TX 17
#define PIN_RS485_DE 7
#define PIN_ZERO_BUTTON 2
#define PIN_COUNT_BUTTON 42

#define MODBUS_DEVICE_ADDR 0x01
#define MODBUS_BAUDRATE 9600
#define MODBUS_TIMEOUT_MS 300
#define MODBUS_DEBUG 0
#define POLL_INTERVAL_MS 100
#define RETRY_INTERVAL_MS 1000
#define BUTTON_DEBOUNCE_MS 50

constexpr uint16_t REG_WEIGHT = 0x0000;
constexpr uint16_t REG_DECIMALS = 0x0001;
constexpr uint16_t REG_UNIT = 0x0002;
constexpr uint16_t REG_CLEAR_ZERO = 0x0016;
constexpr uint16_t REG_INPUT_POLARITY = 0x0017;
constexpr uint16_t CLEAR_ZERO_VALUE = 0x0011;

MAX7221 disp1(PIN_DIN, PIN_CLK, PIN_CS1);
MAX7221 disp2(PIN_DIN, PIN_CLK, PIN_CS2);
MAX7221 disp3(PIN_DIN, PIN_CLK, PIN_CS3);

struct Button {
  uint8_t pin;
  bool stable;
  bool raw;
  uint32_t changedAt;

  Button(uint8_t buttonPin) : pin(buttonPin), stable(HIGH), raw(HIGH), changedAt(0) {}
};

Button zeroButton{PIN_ZERO_BUTTON};
Button countButton{PIN_COUNT_BUTTON};
int32_t totalWeightTenthsKg = 0;
uint32_t count = 0;
uint8_t consecutiveReadFailures = 0;
uint32_t lastPollAt = 0;
uint32_t lastInitAttemptAt = 0;
bool running = false;
bool communicationErrorShown = false;

uint16_t calculateModbusCRC(const uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
  }
  return crc;
}

void printHexBytes(const uint8_t *data, uint16_t length) {
  for (uint16_t i = 0; i < length; ++i) {
    if (data[i] < 0x10) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
}

void setRS485Transmit(bool enabled) { digitalWrite(PIN_RS485_DE, enabled ? HIGH : LOW); }

void clearRS485Input() {
  while (Serial1.available()) Serial1.read();
}

uint16_t readBytesWithTimeout(uint8_t *buffer, uint16_t expectedLen) {
  uint16_t received = 0;
  uint32_t startedAt = millis();
  while (millis() - startedAt < MODBUS_TIMEOUT_MS) {
    while (Serial1.available() && received < expectedLen) buffer[received++] = Serial1.read();
    if (received == expectedLen) break;
    delay(1);
  }
  return received;
}

void sendModbusRequest(const uint8_t *request, uint8_t length) {
  clearRS485Input();
  setRS485Transmit(true);
  delayMicroseconds(100);
  Serial1.write(request, length);
  Serial1.flush();
  delayMicroseconds(100);
  setRS485Transmit(false);
}

bool readHoldingRegisters(uint16_t startRegister, uint16_t registerCount, uint16_t *registers) {
  if (registerCount == 0 || registerCount > 32) return false;
  uint8_t request[8] = {MODBUS_DEVICE_ADDR, 0x03, (uint8_t)(startRegister >> 8), (uint8_t)startRegister,
                        (uint8_t)(registerCount >> 8), (uint8_t)registerCount, 0, 0};
  uint16_t crc = calculateModbusCRC(request, 6);
  request[6] = crc;
  request[7] = crc >> 8;
#if MODBUS_DEBUG
  Serial.print("TX: "); printHexBytes(request, sizeof(request)); Serial.println();
#endif
  sendModbusRequest(request, sizeof(request));

  const uint16_t expectedLen = 5 + registerCount * 2;
  uint8_t response[69];
  uint16_t received = readBytesWithTimeout(response, expectedLen);
  if (received != expectedLen || response[0] != MODBUS_DEVICE_ADDR || response[1] != 0x03 ||
      response[2] != registerCount * 2 || calculateModbusCRC(response, expectedLen - 2) !=
      ((uint16_t)response[expectedLen - 1] << 8 | response[expectedLen - 2])) {
    Serial.print("03 failed, RX: "); printHexBytes(response, received); Serial.println();
    return false;
  }
  for (uint16_t i = 0; i < registerCount; ++i) registers[i] = (uint16_t)response[3 + i * 2] << 8 | response[4 + i * 2];
  return true;
}

bool writeHoldingRegister(uint16_t registerAddress, uint16_t value) {
  uint8_t request[8] = {MODBUS_DEVICE_ADDR, 0x06, (uint8_t)(registerAddress >> 8), (uint8_t)registerAddress,
                        (uint8_t)(value >> 8), (uint8_t)value, 0, 0};
  uint16_t crc = calculateModbusCRC(request, 6);
  request[6] = crc;
  request[7] = crc >> 8;
#if MODBUS_DEBUG
  Serial.print("TX: "); printHexBytes(request, sizeof(request)); Serial.println();
#endif
  sendModbusRequest(request, sizeof(request));

  uint8_t response[8];
  uint16_t received = readBytesWithTimeout(response, sizeof(response));
  if (received != sizeof(response) || memcmp(request, response, sizeof(request)) != 0) {
    Serial.print("06 failed, RX: "); printHexBytes(response, received); Serial.println();
    return false;
  }
  return true;
}

void showCommunicationError() {
  communicationErrorShown = true;
  disp1.displayString("Err");
  disp2.displayString("485");
  disp3.displayString("Err");
}

void formatWeight(int32_t tenthsKg, char *buffer, size_t size) {
  int32_t magnitude = tenthsKg < 0 ? -tenthsKg : tenthsKg;
  if (tenthsKg < 0) snprintf(buffer, size, "-%03ld.%ld", (long)(magnitude / 10), (long)(magnitude % 10));
  else snprintf(buffer, size, "%04ld.%ld", (long)(magnitude / 10), (long)(magnitude % 10));
}

void showWeight(MAX7221 &display, int32_t tenthsKg) {
  if (tenthsKg < -9999 || tenthsKg > 99999) {
    display.displayString("Err");
    Serial.println("Display weight overflow");
    return;
  }
  char text[8];
  formatWeight(tenthsKg, text, sizeof(text));
  display.displayString(text);
}

void refreshTotals() {
  showWeight(disp2, totalWeightTenthsKg);
  if (count > 999) {
    disp3.displayString("Err");
    Serial.println("Display count overflow");
  } else {
    char text[6];
    snprintf(text, sizeof(text), "%03lu00", (unsigned long)count);
    disp3.displayString(text);
  }
}

void flashPass() {
  disp2.clearDisplay();
  disp3.clearDisplay();
  disp1.setDecodeMode(0x00);
  for (uint8_t i = 0; i < 2; ++i) {
    disp1.displayString("PASS");
    delay(500);
    disp1.clearDisplay();
    delay(250);
  }
  disp1.setDecodeMode(0xFF);
}

const char *const PARAMETER_NAMES[24] = {
  "显示值", "小数点", "单位", "屏蔽值", "采样频率", "RC滤波系数",
  "485设备地址", "波特率", "串口校验", "模拟输出低位显示值", "模拟输出高位显示值",
  "校准点1期望值", "校准点2期望值", "校准点3期望值", "校准点4期望值", "校准点5期望值",
  "校准点数", "去皮", "模拟输出低位校正值", "模拟输出高位校正值", "零点跟踪范围",
  "零点跟踪时间", "清零", "输入极性"
};

void printParameters(const uint16_t *parameters) {
  uint16_t scale = parameters[1] == 0 ? 1 : parameters[1] == 1 ? 10 : parameters[1] == 2 ? 100 : 1000;
  Serial.println("========== BSQ-DG-V2 参数 ==========");
  for (uint8_t i = 0; i < 24; ++i) {
    Serial.print("400"); if (i + 1 < 10) Serial.print('0'); Serial.print(i + 1);
    Serial.print(" "); Serial.print(PARAMETER_NAMES[i]);
    Serial.print(" | raw=0x"); if (parameters[i] < 0x1000) Serial.print('0');
    if (parameters[i] < 0x0100) Serial.print('0');
    if (parameters[i] < 0x0010) Serial.print('0');
    Serial.print(parameters[i], HEX); Serial.print(" | ");
    switch (i) {
      case 0: Serial.print("显示="); Serial.println((int16_t)parameters[0] / (float)scale, parameters[1]); break;
      case 1: Serial.print(parameters[i]); Serial.println(" 位小数"); break;
      case 2: Serial.println(parameters[i] == 1 ? "MPa" : parameters[i] == 2 ? "kg" : parameters[i] == 3 ? "T" : parameters[i] == 4 ? "g" : parameters[i] == 5 ? "N" : parameters[i] == 6 ? "kN" : "未知单位"); break;
      case 4: Serial.println(parameters[i] == 1 ? "600 Hz" : parameters[i] == 2 ? "300 Hz" : parameters[i] == 3 ? "150 Hz" : parameters[i] == 4 ? "75 Hz" : parameters[i] == 5 ? "37.5 Hz" : parameters[i] == 6 ? "18.75 Hz" : parameters[i] == 7 ? "10 Hz" : "未知"); break;
      case 5: Serial.println(parameters[i] == 1 ? "无滤波" : parameters[i] == 2 ? "0.8" : parameters[i] == 3 ? "0.6" : parameters[i] == 4 ? "0.4" : parameters[i] == 5 ? "0.2" : parameters[i] == 6 ? "0.1" : "未知"); break;
      case 7: Serial.println(parameters[i] == 0 ? "1200" : parameters[i] == 1 ? "2400" : parameters[i] == 2 ? "4800" : parameters[i] == 3 ? "9600" : parameters[i] == 4 ? "19200" : parameters[i] == 5 ? "38400" : parameters[i] == 6 ? "115200" : "未知"); break;
      case 8: Serial.println(parameters[i] == 0 ? "N-8-1" : parameters[i] == 1 ? "N-8-2" : parameters[i] == 2 ? "ODD-8-1" : parameters[i] == 3 ? "EVEN-8-1" : "未知"); break;
      case 17: Serial.println(parameters[i] == 1 ? "去皮" : parameters[i] == 2 ? "清除去皮" : "无操作"); break;
      case 22: Serial.println(parameters[i] == CLEAR_ZERO_VALUE ? "清零命令" : "无操作"); break;
      case 23: Serial.println(parameters[i] == 1 ? "单极性" : parameters[i] == 2 ? "双极性" : "未知"); break;
      default: Serial.println(parameters[i]); break;
    }
  }
  Serial.println("=====================================");
}

bool initializeScale() {
  uint16_t parameters[24];
  Serial.println("Reading all scale parameters");
  if (!readHoldingRegisters(REG_WEIGHT, 24, parameters)) return false;
  flashPass();

  if (!writeHoldingRegister(REG_DECIMALS, 1) || !writeHoldingRegister(REG_UNIT, 2) ||
      !writeHoldingRegister(REG_INPUT_POLARITY, 2) ||
      !writeHoldingRegister(REG_CLEAR_ZERO, CLEAR_ZERO_VALUE)) return false;

  uint16_t verification[24];
  if (!readHoldingRegisters(REG_WEIGHT, 24, verification) || verification[1] != 1 || verification[2] != 2 ||
      verification[23] != 2) {
    Serial.println("Scale configuration verification failed");
    return false;
  }
  printParameters(verification);
  totalWeightTenthsKg = 0;
  count = 0;
  consecutiveReadFailures = 0;
  communicationErrorShown = false;
  showWeight(disp1, (int16_t)verification[0]);
  refreshTotals();
  Serial.println("Scale ready");
  return true;
}

bool readCurrentWeight(int32_t &tenthsKg) {
  uint16_t raw;
  if (!readHoldingRegisters(REG_WEIGHT, 1, &raw)) return false;
  tenthsKg = (int16_t)raw;
  return true;
}

void clearAll() {
  if (!writeHoldingRegister(REG_CLEAR_ZERO, CLEAR_ZERO_VALUE)) {
    showCommunicationError();
    return;
  }
  totalWeightTenthsKg = 0;
  count = 0;
  showWeight(disp1, 0);
  refreshTotals();
  Serial.println("Weight, total and count cleared");
}

void addCount() {
  int32_t weight;
  if (!readCurrentWeight(weight)) {
    showCommunicationError();
    return;
  }
  totalWeightTenthsKg += weight;
  ++count;
  refreshTotals();
  if (!writeHoldingRegister(REG_CLEAR_ZERO, CLEAR_ZERO_VALUE)) {
    Serial.println("Counted successfully, but scale clear failed");
    showCommunicationError();
    return;
  }
  showWeight(disp1, 0);
  Serial.println("Weight added and scale cleared");
}

bool pressed(Button &button) {
  bool raw = digitalRead(button.pin);
  uint32_t now = millis();
  if (raw != button.raw) {
    button.raw = raw;
    button.changedAt = now;
  }
  if (raw != button.stable && now - button.changedAt >= BUTTON_DEBOUNCE_MS) {
    button.stable = raw;
    return button.stable == LOW;
  }
  return false;
}

void pollWeight() {
  int32_t weight;
  if (readCurrentWeight(weight)) {
    consecutiveReadFailures = 0;
    if (communicationErrorShown) {
      refreshTotals();
      communicationErrorShown = false;
    }
    showWeight(disp1, weight);
  } else if (++consecutiveReadFailures >= 3) {
    showCommunicationError();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RS485_DE, OUTPUT);
  setRS485Transmit(false);
  Serial1.begin(MODBUS_BAUDRATE, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
  pinMode(PIN_ZERO_BUTTON, INPUT_PULLUP);
  pinMode(PIN_COUNT_BUTTON, INPUT_PULLUP);

  disp1.begin();
  disp2.begin();
  disp3.begin();
  for (uint8_t i = 0; i < 2; ++i) {
    disp1.testMode(true); disp2.testMode(true); disp3.testMode(true);
    delay(500);
    disp1.testMode(false); disp2.testMode(false); disp3.testMode(false);
    delay(250);
  }
  disp1.clearDisplay(); disp2.clearDisplay(); disp3.clearDisplay();
  lastInitAttemptAt = millis() - RETRY_INTERVAL_MS;
}

void loop() {
  uint32_t now = millis();
  if (!running) {
    if (now - lastInitAttemptAt >= RETRY_INTERVAL_MS) {
      lastInitAttemptAt = now;
      running = initializeScale();
      if (!running) showCommunicationError();
    }
    return;
  }
  if (pressed(zeroButton)) clearAll();
  if (pressed(countButton)) addCount();
  now = millis();
  if (now - lastPollAt >= POLL_INTERVAL_MS) {
    lastPollAt = now;
    pollWeight();
  }
}
