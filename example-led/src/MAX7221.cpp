#include "MAX7221.h"
#include <SPI.h>

MAX7221::MAX7221(uint8_t pinDIN, uint8_t pinCLK, uint8_t pinCS)
    : _pinDIN(pinDIN), _pinCLK(pinCLK), _pinCS(pinCS), _decodeMode(0xFF) {}

void MAX7221::begin() {
    pinMode(_pinCS, OUTPUT);
    digitalWrite(_pinCS, HIGH); // CS starts HIGH (inactive)

    // Initialize hardware SPI on the shared pins
    // SCK = _pinCLK (GPIO 12), MISO = -1 (not used), MOSI = _pinDIN (GPIO 11), SS = -1
    SPI.begin(_pinCLK, -1, _pinDIN, -1);

    // Setup MAX7221 initial configuration
    shutdown(true);            // Enter shutdown mode to prevent transient noise
    setScanLimit(6);           // Scan limit 6 displays digits 0 to 6 (we use digits 0, 1, 2, 5, 6)
    setDecodeMode(0xFF);       // Use Code B BCD decode mode (internally handles segment fonts)
    setIntensity(7);           // Start with medium brightness (0 to 15)
    testMode(false);           // Disable test mode
    clearDisplay();            // Clear all digit registers
    shutdown(false);           // Wake up the chip
}

void MAX7221::shutdown(bool bShutdown) {
    writeRegister(0x0C, bShutdown ? 0x00 : 0x01);
}

void MAX7221::setIntensity(uint8_t intensity) {
    if (intensity > 15) intensity = 15;
    writeRegister(0x0A, intensity);
}

void MAX7221::setDecodeMode(uint8_t mode) {
    _decodeMode = mode;
    writeRegister(0x09, mode);
}

void MAX7221::testMode(bool bTest) {
    writeRegister(0x0F, bTest ? 0x01 : 0x00);
}

void MAX7221::setScanLimit(uint8_t limit) {
    if (limit > 7) limit = 7;
    writeRegister(0x0B, limit);
}

void MAX7221::writeRegister(uint8_t reg, uint8_t data) {
    // MAX7221 supports up to 10MHz clock speed, MSBFIRST, SPI Mode 0
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_pinCS, LOW);
    
    // Send register address and then the data byte
    SPI.transfer(reg);
    SPI.transfer(data);
    
    digitalWrite(_pinCS, HIGH);
    SPI.endTransaction();
}

uint8_t MAX7221::getDigitRegister(uint8_t physicalDigit) {
    // Mapping physical 5-digit positions (1=Leftmost, 5=Rightmost) to sequential MAX7221 registers
    // Physical Digit 1 -> DIG 1 (Reg 0x02)
    // Physical Digit 2 -> DIG 2 (Reg 0x03)
    // Physical Digit 3 -> DIG 3 (Reg 0x04)
    // Physical Digit 4 -> DIG 4 (Reg 0x05)
    // Physical Digit 5 -> DIG 5 (Reg 0x06)
    switch (physicalDigit) {
        case 1: return 0x02; // DIG 1
        case 2: return 0x03; // DIG 2
        case 3: return 0x04; // DIG 3
        case 4: return 0x05; // DIG 4
        case 5: return 0x06; // DIG 5
        default: return 0x00; // Invalid
    }
}

void MAX7221::clearDisplay() {
    for (uint8_t i = 1; i <= 5; i++) {
        displayChar(i, ' ', false);
    }
}

void MAX7221::displayRaw(uint8_t physicalDigit, uint8_t segmentPattern) {
    uint8_t reg = getDigitRegister(physicalDigit);
    if (reg != 0x00) {
        writeRegister(reg, segmentPattern);
    }
}

void MAX7221::displayChar(uint8_t physicalDigit, char c, bool dp) {
    uint8_t reg = getDigitRegister(physicalDigit);
    if (reg == 0x00) return;

    if (_decodeMode == 0xFF) {
        // Code B Decoding mode
        uint8_t code = asciiToCodeB(c);
        if (dp) code |= 0x80; // Set DP bit
        writeRegister(reg, code);
    } else {
        // No-decode mode (we send the raw segments)
        uint8_t segments = asciiToSegments(c);
        if (dp) segments |= 0x80; // Turn on DP segment
        writeRegister(reg, segments);
    }
}

void MAX7221::displayString(const char* str) {
    uint8_t physicalDigit = 1;
    int idx = 0;

    // Clear display first to ensure blanking of unused digits
    clearDisplay();

    while (str[idx] != '\0' && physicalDigit <= 5) {
        char c = str[idx];
        bool dp = false;

        // Check if next character is a dot/decimal point
        if (str[idx + 1] == '.') {
            dp = true;
            idx++; // Consume the dot
        }

        displayChar(physicalDigit, c, dp);
        physicalDigit++;
        idx++;
    }
}

void MAX7221::displayNumber(int32_t number) {
    char buf[12];
    if (number > 99999 || number < -9999) {
        displayString("Err");
        return;
    }
    snprintf(buf, sizeof(buf), "%5d", number);
    displayString(buf);
}

void MAX7221::displayFloat(float number, uint8_t decimalPlaces) {
    char buf[12];
    if (number > 99999.0f || number < -9999.0f) {
        displayString("Err");
        return;
    }

    // Format with the specified decimal places
    char format[10];
    snprintf(format, sizeof(format), "%%.%df", decimalPlaces);
    snprintf(buf, sizeof(buf), format, number);

    // Calculate actual digit count (excluding decimal points)
    int digitCount = 0;
    for (int i = 0; buf[i] != '\0'; i++) {
        if (buf[i] != '.') digitCount++;
    }

    // If it exceeds the 5 physical digits, fall back to integer representation
    if (digitCount > 5) {
        snprintf(buf, sizeof(buf), "%.0f", number);
        digitCount = 0;
        for (int i = 0; buf[i] != '\0'; i++) {
            if (buf[i] != '.') digitCount++;
        }
        if (digitCount > 5) {
            displayString("Err");
            return;
        }
    }

    // Right-align the formatted buffer to 5 digits
    char finalBuf[12];
    int leadingSpaces = 5 - digitCount;
    int writeIdx = 0;
    for (int i = 0; i < leadingSpaces; i++) {
        finalBuf[writeIdx++] = ' ';
    }
    strcpy(finalBuf + writeIdx, buf);

    displayString(finalBuf);
}

uint8_t MAX7221::asciiToCodeB(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    switch (c) {
        case '-': return 10;
        case 'E': case 'e': return 11;
        case 'H': case 'h': return 12;
        case 'L': case 'l': return 13;
        case 'P': case 'p': return 14;
        case ' ': default: return 15;
    }
}

uint8_t MAX7221::asciiToSegments(char c) {
    switch (c) {
        case '0': return 0b00111111; // A B C D E F
        case '1': return 0b00000110; // B C
        case '2': return 0b01011011; // A B D E G
        case '3': return 0b01001111; // A B C D G
        case '4': return 0b01100110; // B C F G
        case '5': return 0b01101101; // A C D F G
        case '6': return 0b01111101; // A C D E F G
        case '7': return 0b00000111; // A B C
        case '8': return 0b01111111; // A B C D E F G
        case '9': return 0b01101111; // A B C D F G
        case 'A': case 'a': return 0b01110111;
        case 'B': case 'b': return 0b01111100;
        case 'C': case 'c': return 0b00111001;
        case 'D': case 'd': return 0b01011110;
        case 'E': case 'e': return 0b01111001;
        case 'F': case 'f': return 0b01110001;
        case 'G': case 'g': return 0b00111101;
        case 'H': case 'h': return 0b01110110;
        case 'I': case 'i': return 0b00000110;
        case 'J': case 'j': return 0b00001110;
        case 'L': case 'l': return 0b00111000;
        case 'N': case 'n': return 0b01010100;
        case 'O': case 'o': return 0b00111111;
        case 'P': case 'p': return 0b01110011;
        case 'R': case 'r': return 0b01010000;
        case 'S': case 's': return 0b01101101;
        case 'T': case 't': return 0b01111000;
        case 'U': case 'u': return 0b00111110;
        case 'Y': case 'y': return 0b01101110;
        case '-': return 0b01000000;
        case '_': return 0b00001000;
        case '=': return 0b01001000; // D and G
        case '.': return 0b10000000; // DP only
        case ' ': default: return 0b00000000;
    }
}
