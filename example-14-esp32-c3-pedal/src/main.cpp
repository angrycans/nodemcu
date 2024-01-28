
#include <Arduino.h>
#include <BleGamepad.h>

#define T_PIN 2
#define T_INPUT_MIN 2400
#define T_INPUT_MAX 3500
#define T_MIN -32767
#define T_MAX 32767
#define BUTTONPIN 19 // Pin button is attached to

BleGamepad bleGamepad("BLE Driving Controller", "lemmingDev", 100);

int previousButton1State = HIGH;
int Throttle_ = 0;
String All_OutPut = "0";

#define numOfButtons 10
#define numOfHatSwitches 0
#define enableX false
#define enableY false
#define enableZ false
#define enableRX false
#define enableRY false
#define enableRZ false
#define enableSlider1 false
#define enableSlider2 false
#define enableRudder false
#define enableThrottle false
#define enableAccelerator true
#define enableBrake true
#define enableSteering true
void setup()
{
    // put your setup code here, to run once:
    delay(2000);
    Serial.begin(115200);

    // Setup controller with 10 buttons, accelerator, brake and steering
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setAutoReport(false);
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setButtonCount(numOfButtons);
    bleGamepadConfig.setWhichAxes(enableX, enableY, enableZ, enableRX, enableRY, enableRZ, enableSlider1, enableSlider2);      // Can also be done per-axis individually. All are true by default
    bleGamepadConfig.setWhichSimulationControls(enableRudder, enableThrottle, enableAccelerator, enableBrake, enableSteering); // Can also be done per-control individually. All are false by default
    bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);                                                                      // 1 by default
    // Some non-Windows operating systems and web based gamepad testers don't like min axis set below 0, so 0 is set by default
    bleGamepadConfig.setAxesMin(0x8001); // -32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
    bleGamepadConfig.setAxesMax(0x7FFF); // 32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal

    bleGamepad.begin(&bleGamepadConfig);
    // changing bleGamepadConfig after the begin function has no effect, unless you call the begin function again

    // Set accelerator and brake to min
    bleGamepad.setAccelerator(-32767);
    bleGamepad.setBrake(-32767);

    // Set steering to center
    bleGamepad.setSteering(0);

    Serial.println("start...");
}

void loop()
{
    // put your main code here, to run repeatedly:
    Throttle_ = analogRead(T_PIN);
    All_OutPut = String("Throttle ") + String(Throttle_);

    Throttle_ = map(Throttle_, T_INPUT_MIN, T_INPUT_MAX, T_MIN, T_MAX); // 值映射
    Throttle_ = T_MAX - Throttle_;                                      // 霍尔电压值反转，视磁铁布置方式而定
    // delay(1000);

    // 将三个踏板的原始值输出到串口
    // All_OutPut = String("Throttle ") + String(Throttle_);
    Serial.println(All_OutPut);

    if (bleGamepad.isConnected())
    {
        // Serial.println("Press all buttons one by one");
        // for (int i = 1; i <= numOfButtons; i += 1)
        // {
        //     bleGamepad.press(i);
        //     bleGamepad.sendReport();
        //     delay(100);
        //     bleGamepad.release(i);
        //     bleGamepad.sendReport();
        //     delay(25);
        // }

        // Serial.println("Move steering from center to max");
        // for (int i = 0; i > -32767; i -= 256)
        // {
        //     bleGamepad.setSteering(i);
        //     bleGamepad.sendReport();
        //     delay(10);
        // }

        // Serial.println("Move steering from min to max");
        // for (int i = -32767; i < 32767; i += 256)
        // {
        //     bleGamepad.setSteering(i);
        //     bleGamepad.sendReport();
        //     delay(10);
        // }

        // Serial.println("Move steering from max to center");
        // for (int i = 32767; i > 0; i -= 256)
        // {
        //     bleGamepad.setSteering(i);
        //     bleGamepad.sendReport();
        //     delay(10);
        // }
        // bleGamepad.setSteering(0);
        // bleGamepad.sendReport();

        // Serial.println("Move accelerator from min to max");
        // // for(int i = 32767 ; i > -32767 ; i -= 256)    //Use this for loop setup instead if accelerator is reversed
        // for (int i = -32767; i < 32767; i += 256)
        // {
        //     bleGamepad.setAccelerator(i);
        //     bleGamepad.sendReport();
        //     delay(10);
        // }
        // bleGamepad.setAccelerator(-32767);
        // bleGamepad.sendReport();

        // Serial.println("Move brake from min to max");
        // for (int i = -32767; i < 32767; i += 256)
        // {
        //     bleGamepad.setBrake(i);
        //     bleGamepad.sendReport();
        //     delay(10);
        // }
        delay(10);
        bleGamepad.setBrake(Throttle_);
        bleGamepad.sendReport();
    }
}