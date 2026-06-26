#include "MAX7221.h"
#include <Arduino.h>

// GPIO Pin Definitions based on user settings
#define PIN_DIN 11
#define PIN_CLK 12
#define PIN_CS1 8
#define PIN_CS2 9
#define PIN_CS3 10

// Instantiate display drivers
MAX7221 disp1(PIN_DIN, PIN_CLK, PIN_CS1);
MAX7221 disp2(PIN_DIN, PIN_CLK, PIN_CS2);
MAX7221 disp3(PIN_DIN, PIN_CLK, PIN_CS3);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("==========================================");
  Serial.println("MAX7221 Final Calibration Booting...");
  Serial.println("==========================================");

  // Initialize all three displays
  disp1.begin();
  disp2.begin();
  disp3.begin();

  // Test Mode: Turn on all segments for 1 second
  disp1.testMode(true);
  disp2.testMode(true);
  disp3.testMode(true);
  delay(1000);

  // Turn off test mode
  disp1.testMode(false);
  disp2.testMode(false);
  disp3.testMode(false);
  delay(500);

  // Show final values
  Serial.println("Displaying values:");
  Serial.println("Disp 1 -> 12345");
  Serial.println("Disp 2 -> 34567");
  Serial.println("Disp 3 -> 5.6.7.8.9");

  disp1.displayString("12345");
  disp2.displayString("34567");
  disp3.displayString("5.6.7.8.9");
}

void loop() {
  // Static display, no action needed in loop
  delay(1000);
}
