#include <Battery.h>

Battery battery(3400, 4200, 4);

void initBat();
void BatLoop();

void initBat()
{
  battery.begin(5000, 1.0);
}

void BatLoop()
{
  Serial.print("Battery voltage is ");
  Serial.print(battery.voltage());
  Serial.print(" (");
  Serial.print(battery.level());
  Serial.println("%)");
}