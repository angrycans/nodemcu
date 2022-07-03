#include <Arduino.h>
#include <SPI.h>

//#define LED LED_BUILTIN;
void setup()
{
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(D3, OUTPUT);    // connect MOSFET to Pin D7
  digitalWrite(D3, HIGH); // set to HIGH to keep arduino on after user releases pushbutton

  delay(5000); // replace this by your stuff

  digitalWrite(D3, LOW); // power-off
}

void loop()
{
  // put your main code here, to run repeatedly:
  // digitalWrite(LED_BUILTIN, (millis() / 1000) % 2);
}