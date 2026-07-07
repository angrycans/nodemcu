#include "MAX7221.h"
#include <Arduino.h>

// GPIO Pin Definitions for Display Drivers
#define PIN_DIN 11
#define PIN_CLK 12
#define PIN_CS1 8
#define PIN_CS2 9
#define PIN_CS3 10

// GPIO Pin Definitions for MAX3485 RS485
#define PIN_RS485_RX 18
#define PIN_RS485_TX 17
#define PIN_RS485_DE 7  // Direction control pin (HIGH = TX, LOW = RX). Set to -1 if using an auto-switching module.

#define MODBUS_DEVICE_ADDR 0x01
#define MODBUS_BAUDRATE 9600
#define MODBUS_TIMEOUT_MS 300
#define MODBUS_DEBUG 1

// Instantiate display drivers
MAX7221 disp1(PIN_DIN, PIN_CLK, PIN_CS1);
MAX7221 disp2(PIN_DIN, PIN_CLK, PIN_CS2);
MAX7221 disp3(PIN_DIN, PIN_CLK, PIN_CS3);

struct BsqStatus {
  int32_t displayCounts;
  uint8_t decimalPlaces;
  uint16_t unitCode;
  uint16_t deviceAddr;
  uint16_t baudCode;
  uint16_t serialFormat;
  uint16_t inputPolarity;
};

uint16_t calculateModbusCRC(const uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void setRS485Transmit(bool enabled) {
  if (PIN_RS485_DE != -1) {
    digitalWrite(PIN_RS485_DE, enabled ? HIGH : LOW);
  }
}

void clearRS485Input() {
  while (Serial1.available()) {
    Serial1.read();
  }
}

void printHexBytes(const uint8_t *data, uint16_t length) {
  for (uint16_t i = 0; i < length; i++) {
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
}

uint16_t readBytesWithTimeout(uint8_t *buffer, uint16_t expectedLen, uint32_t timeoutMs) {
  uint16_t received = 0;
  uint32_t startTime = millis();

  while (millis() - startTime < timeoutMs) {
    while (Serial1.available() && received < expectedLen) {
      buffer[received++] = (uint8_t)Serial1.read();
    }
    if (received >= expectedLen) {
      return received;
    }
    delay(1);
  }

  return received;
}

bool sendModbusRequest(const uint8_t *request, uint8_t length) {
  clearRS485Input();
  setRS485Transmit(true);
  delayMicroseconds(100);
  Serial1.write(request, length);
  Serial1.flush(); // Wait for TX to finish
  delayMicroseconds(100);
  setRS485Transmit(false);
  return true;
}

bool readHoldingRegisters(uint8_t deviceAddr, uint16_t startRegister, uint16_t registerCount, uint16_t *registers) {
  if (registerCount == 0 || registerCount > 32) {
    Serial.println("Error: Invalid Modbus register count.");
    return false;
  }

  uint8_t request[8] = {
    deviceAddr,
    0x03,
    (uint8_t)(startRegister >> 8),
    (uint8_t)(startRegister & 0xFF),
    (uint8_t)(registerCount >> 8),
    (uint8_t)(registerCount & 0xFF),
    0,
    0
  };
  uint16_t requestCRC = calculateModbusCRC(request, 6);
  request[6] = (uint8_t)(requestCRC & 0xFF);
  request[7] = (uint8_t)(requestCRC >> 8);

#if MODBUS_DEBUG
  Serial.print("TX: ");
  printHexBytes(request, sizeof(request));
  Serial.println();
#endif

  sendModbusRequest(request, sizeof(request));

  const uint16_t expectedLen = 5 + registerCount * 2;
  uint8_t response[69];
  uint16_t receivedLen = readBytesWithTimeout(response, expectedLen, MODBUS_TIMEOUT_MS);
#if MODBUS_DEBUG
  Serial.print("RX(");
  Serial.print(receivedLen);
  Serial.print("/");
  Serial.print(expectedLen);
  Serial.print("): ");
  printHexBytes(response, receivedLen);
  Serial.println();
#endif

  if (receivedLen < expectedLen) {
    Serial.println("Error: No or incomplete response from RS485 device.");
    Serial.println("Tip: Factory default is Addr 1, 9600 baud, N-8-1. Check A/B wiring if there is no response.");
    return false;
  }

  if (response[0] != deviceAddr || response[1] != 0x03 || response[2] != registerCount * 2) {
    Serial.print("Error: Unexpected Modbus response header: ");
    for (uint16_t i = 0; i < expectedLen; i++) {
      if (response[i] < 0x10) Serial.print("0");
      Serial.print(response[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    return false;
  }

  uint16_t calculatedCRC = calculateModbusCRC(response, expectedLen - 2);
  uint16_t receivedCRC = ((uint16_t)response[expectedLen - 1] << 8) | response[expectedLen - 2];
  if (calculatedCRC != receivedCRC) {
    Serial.print("Error: CRC check failed. Calc=0x");
    Serial.print(calculatedCRC, HEX);
    Serial.print(", Recv=0x");
    Serial.println(receivedCRC, HEX);
    return false;
  }

  for (uint16_t i = 0; i < registerCount; i++) {
    uint16_t dataIndex = 3 + i * 2;
    registers[i] = ((uint16_t)response[dataIndex] << 8) | response[dataIndex + 1];
  }

  return true;
}

const char* unitName(uint16_t unitCode) {
  switch (unitCode) {
    case 1: return "MPa";
    case 2: return "Kg";
    case 3: return "T";
    case 4: return "g";
    case 5: return "N";
    case 6: return "KN";
    default: return "Unknown";
  }
}

long baudRateFromCode(uint16_t baudCode) {
  switch (baudCode) {
    case 0: return 1200;
    case 1: return 2400;
    case 2: return 4800;
    case 3: return 9600;
    case 4: return 19200;
    case 5: return 38400;
    case 6: return 115200;
    default: return 0;
  }
}

const char* serialFormatName(uint16_t formatCode) {
  switch (formatCode) {
    case 0: return "N-8-1";
    case 1: return "N-8-2";
    case 2: return "ODD-8-1";
    case 3: return "EVEN-8-1";
    default: return "Unknown";
  }
}

float scaledDisplayValue(int32_t counts, uint8_t decimalPlaces) {
  float value = (float)counts;
  for (uint8_t i = 0; i < decimalPlaces; i++) {
    value /= 10.0f;
  }
  return value;
}

bool readDeviceStatus(BsqStatus &status) {
  // PDF table: 40001 maps to Modbus start address 0x0000.
  uint16_t registers[3];
  if (!readHoldingRegisters(MODBUS_DEVICE_ADDR, 0x0000, 3, registers)) {
    return false;
  }

  status.decimalPlaces = (uint8_t)min(registers[1], (uint16_t)3);
  status.unitCode = registers[2];
  status.deviceAddr = MODBUS_DEVICE_ADDR;
  status.baudCode = 3;
  status.serialFormat = 0;
  status.inputPolarity = 2;
  status.displayCounts = (int16_t)registers[0];

  return true;
}

// Function to read all status/parameter registers (registers 40001 to 40024)
void updateDeviceStatus() {
  BsqStatus status;
  if (!readDeviceStatus(status)) {
    disp1.displayString("Err");
    disp2.displayString("485");
    disp3.displayString("----");
    return;
  }

  float weight = scaledDisplayValue(status.displayCounts, status.decimalPlaces);
  const char* unitStr = unitName(status.unitCode);
  long baudrate = baudRateFromCode(status.baudCode);

  // Print results to Serial
  Serial.println("==========================================");
  Serial.print("Device Address: "); Serial.println(status.deviceAddr);
  Serial.print("Display Value:  "); Serial.print(weight, status.decimalPlaces); Serial.print(" "); Serial.println(unitStr);
  Serial.print("Raw Counts:     "); Serial.println(status.displayCounts);
  Serial.print("Decimal Point:  "); Serial.println(status.decimalPlaces);
  Serial.print("Baud Rate:      "); Serial.print(baudrate); Serial.println(" bps");
  Serial.print("Data Format:    "); Serial.println(serialFormatName(status.serialFormat));
  Serial.print("Input Polarity: "); Serial.println(status.inputPolarity == 1 ? "Unipolar" : "Bipolar");
  Serial.println("==========================================");

  // Update displays with weight and unit/status
  char displayBuf[16];
  disp1.displayFloat(weight, status.decimalPlaces);

  // Display status on disp2: Address & Baud
  snprintf(displayBuf, sizeof(displayBuf), "A%d.%d", status.deviceAddr, (int)(baudrate / 1000));
  disp2.displayString(displayBuf);

  // Display status on disp3: Unit code and decimal point configuration
  snprintf(displayBuf, sizeof(displayBuf), "U%d.d%d", status.unitCode, status.decimalPlaces);
  disp3.displayString(displayBuf);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("==========================================");
  Serial.println("ESP32-S3 MAX3485 Weighing Reader Booting...");
  Serial.println("==========================================");

  // Initialize DE/RE pin for RS485 direction control
  if (PIN_RS485_DE != -1) {
    pinMode(PIN_RS485_DE, OUTPUT);
    digitalWrite(PIN_RS485_DE, LOW); // Start in receive mode
  }

  // Initialize HardwareSerial for RS485 (Serial1)
  // Default communication settings for BSQ-DG-V2 are 9600 Baud, N-8-1
  Serial1.begin(MODBUS_BAUDRATE, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);

  // Initialize MAX7221 Displays
  disp1.begin();
  disp2.begin();
  disp3.begin();

  // Test Mode: Flash all segments twice to check hardware connectivity
  for (int i = 0; i < 2; i++) {
    disp1.testMode(true);
    disp2.testMode(true);
    disp3.testMode(true);
    delay(500);

    disp1.testMode(false);
    disp2.testMode(false);
    disp3.testMode(false);
    delay(250);
  }

  disp1.displayString("INIT");
  disp2.displayString("485");
  disp3.displayString("CONN");
  delay(1000);
}

void loop() {
  // Read device status periodically
  updateDeviceStatus();
  delay(2000); // Poll every 2 seconds
}
