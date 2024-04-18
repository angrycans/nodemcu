
#include "HAL/HAL.h"
#define CM_CONSTRAIN(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

#define CM_VALUE_LIMIT(x, min, max) ((x) = CM_CONSTRAIN((x), (min), (max)))
#define CM_EXECUTE_INTERVAL(func, time)    \
    do                                     \
    {                                      \
        static unsigned long lasttime = 0; \
        if (millis() - lasttime >= (time)) \
            func, lasttime = millis();     \
    } while (0)
#define BATT_MIN_VOLTAGE 3300
#define BATT_MAX_VOLTAGE 4200
#define BATT_FULL_CHARGE_VOLTAGE 4100

Battery battery(3300, 4200, CONFIG_BATTERY_PIN);

int voltage, usage;

static uint16_t ADCValue;

static void Power_ADC_Update()
{

    ADCValue = analogRead(CONFIG_BATTERY_PIN);
    HAL::Power_GetInfo();
}

void HAL::POWER_Init()
{
    pinMode(CONFIG_BATDET_PIN, INPUT_PULLUP);
    pinMode(CONFIG_BATTERY_PIN, INPUT);
    battery.begin(1200, 1.47);
}

void HAL::Power_GetInfo()
{
    voltage = map(
        ADCValue,
        0, 4095,
        0, 3300);

    voltage *= 2;

    CM_VALUE_LIMIT(voltage, BATT_MIN_VOLTAGE, BATT_MAX_VOLTAGE);

    int usage = map(
        voltage,
        BATT_MIN_VOLTAGE, BATT_FULL_CHARGE_VOLTAGE,
        0, 100);

    CM_VALUE_LIMIT(usage, 0, 100);
}

void HAL::POWER_Update()
{
    CM_EXECUTE_INTERVAL(Power_ADC_Update(), 1000);
}
