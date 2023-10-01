
#include <Arduino.h>

#define T_PIN 2
#define BUTTONPIN 19 // Pin button is attached to

// BleGamepad bleGamepad;

int previousButton1State = HIGH;
int Throttle_ = 0;
String All_OutPut = "0";
void setup()
{
    // put your setup code here, to run once:
    delay(2000);
    Serial.begin(115200);

    Serial.println("start...");
}

void loop()
{
    // put your main code here, to run repeatedly:
    Throttle_ = analogRead(T_PIN);
    delay(100);
    Serial.println("===>");
    // 将三个踏板的原始值输出到串口
    All_OutPut = String("Throttle ") + String(Throttle_);
    Serial.println(All_OutPut);
}