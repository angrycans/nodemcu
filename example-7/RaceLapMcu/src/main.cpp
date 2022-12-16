#include "Arduino.h"
#include <WiFi.h>

#include "config.h"
#include "reset_reason.hpp"

#include "HAL/HAL.h"

void setup()
{
  Serial.begin(BUAD);

  delay(2000);
  boot_print_reset_reason();

  HAL::Init();
}

void loop()
{

  HAL::Update();
}
