#include <Arduino.h>
#include <lv_conf.h>
#include <lvgl.h>

#include "gui/gui.h"

#define LED_RUN 1

void setup()
{
    Serial.begin(115200);
    pinMode(LED_RUN, OUTPUT);
    delay(1000);
    Serial.println("setup ...");
    gui_start();
}

void loop()
{
    // digitalWrite(LED_RUN, (millis() / 1000) % 2);

    lv_timer_handler();
    gui_loop();

    delay(5);
}
