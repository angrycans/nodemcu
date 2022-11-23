#include "Arduino.h"
#include <WiFi.h>

#include "config.h"
#include "HAL/HAL.h"

void setup()
{
  Serial.begin(BUAD);
  delay(3000);
  HAL::Init();
}

void loop()
{

  HAL::Update();
}
