#include "Arduino.h"
#include <lvgl.h>
#include "./app/app.h"

APP app;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    app.init();
}

void loop()
{
    app.loop();

    delay(5);
}
