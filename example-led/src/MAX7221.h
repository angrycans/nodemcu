#ifndef MAX7221_H
#define MAX7221_H

#include <Arduino.h>

class MAX7221 {
public:
    // Constructor. CS pin is specific to each chip, DIN and CLK are shared.
    MAX7221(uint8_t pinDIN, uint8_t pinCLK, uint8_t pinCS);

    // Initialize the chip
    void begin();

    // Enable/disable shutdown mode (power saving)
    void shutdown(bool bShutdown);

    // Set intensity/brightness (0 to 15)
    void setIntensity(uint8_t intensity);

    // Set decode mode:
    // 0x00: No decode (raw segment control) - DEFAULT & RECOMMENDED
    // 0xFF: Code B BCD decode (displays 0-9, -, E, H, L, P, Blank)
    void setDecodeMode(uint8_t mode);

    // Clear display
    void clearDisplay();

    // Turn all segments on or off (for hardware testing)
    void testMode(bool bTest);

    // Set the scan limit (0 to 7). For our 5-digit layout, it must be at least 6.
    void setScanLimit(uint8_t limit);

    // Display a raw 8-bit segment pattern on a physical digit (1 to 5)
    // Segment bits: Bit 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, 7=DP
    void displayRaw(uint8_t physicalDigit, uint8_t segmentPattern);

    // Display a single character on a physical digit (1 to 5)
    // Works in both No-decode and Code B decode mode (uses BCD conversion when in decode mode).
    void displayChar(uint8_t physicalDigit, char c, bool dp = false);

    // Display an integer (e.g. -9999 to 99999)
    void displayNumber(int32_t number);

    // Display a floating point number (e.g. -9.99 to 99.99)
    void displayFloat(float number, uint8_t decimalPlaces = 2);

    // Display a string (up to 5 characters, automatically handling decimal points)
    void displayString(const char* str);

    // Directly write to a MAX7221 register
    void writeRegister(uint8_t reg, uint8_t data);

private:
    uint8_t _pinDIN;
    uint8_t _pinCLK;
    uint8_t _pinCS;
    uint8_t _decodeMode;

    // Helper to map 1-indexed physical digits (1-5) to MAX7221 register addresses (0x01-0x08)
    uint8_t getDigitRegister(uint8_t physicalDigit);

    // Helper to convert ASCII character to MAX7221 segment pattern (No-decode mode)
    uint8_t asciiToSegments(char c);

    // Helper to convert ASCII character to Code B BCD code (Code B decode mode)
    uint8_t asciiToCodeB(char c);
};

#endif // MAX7221_H
