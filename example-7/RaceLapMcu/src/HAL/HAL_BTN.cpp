
#include "HAL/HAL.h"

bool btnStatus = false;
unsigned long lastTime;

void HAL::BTN_Init()
{
    pinMode(BTN_KEY1, INPUT);
    pinMode(BTN_KEY2, OUTPUT);
}

void HAL::BTN_Update()
{

    bool buttonStatus = digitalRead(BTN_KEY1);

    if (digitalRead(BTN_KEY1) == LOW)
    {
        // logger.LogInfo("Button  is LOW");
    }
    else
    {
        // logger.LogInfo("Button  is HIGH");
    }

    // 软件消抖——按钮检测到被按下，且上一次按下和这次间隔超过2000ms
    // if (!buttonStatus && millis() - lastTime > 2000)
    // {
    //     btnStatus = !btnStatus;
    //     logger.LogInfo("Button pressed");
    //     // digitalWrite(BTN_KEY2, HIGH);
    //     // ESP.restart();
    //     lastTime = millis();
    // }
}
