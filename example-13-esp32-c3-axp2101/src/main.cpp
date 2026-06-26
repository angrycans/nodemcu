#include <Arduino.h>
#include "esp_system.h"

#define SDA 4
#define SCL 5

const char* resetReasonToString(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_UNKNOWN: return "UNKNOWN";
    case ESP_RST_POWERON: return "POWERON";
    case ESP_RST_EXT: return "EXTERNAL RESET";
    case ESP_RST_SW: return "SOFTWARE RESET";
    case ESP_RST_PANIC: return "PANIC";
    case ESP_RST_INT_WDT: return "INTERRUPT WDT";
    case ESP_RST_TASK_WDT: return "TASK WDT";
    case ESP_RST_WDT: return "OTHER WDT";
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
    case ESP_RST_BROWNOUT: return "BROWNOUT";
    case ESP_RST_SDIO: return "SDIO";
    default: return "UNKNOWN_ENUM";
  }
}

void setup() {
  Serial.begin(115200);
  delay(4000);

  esp_reset_reason_t reason = esp_reset_reason();
  Serial.print("Reset reason: ");
  Serial.print((int)reason);
  Serial.print(" (");
  Serial.print(resetReasonToString(reason));
  Serial.println(")");

  Serial.print("Free heap: ");
  Serial.println(esp_get_free_heap_size());

  Serial.println("Starting...");

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(750);
}
