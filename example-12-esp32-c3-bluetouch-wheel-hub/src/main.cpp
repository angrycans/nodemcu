 /*
 * This example turns the ESP32 into a Bluetooth LE gamepad that presses buttons and moves axis
 * 
 * Possible buttons are:
 * BUTTON_1 through to BUTTON_16 
 * (16 buttons supported by default. Library can be configured to support up to 128)
 * 
 * Possible DPAD/HAT switch position values are: 
 * DPAD_CENTERED, DPAD_UP, DPAD_UP_RIGHT, DPAD_RIGHT, DPAD_DOWN_RIGHT, DPAD_DOWN, DPAD_DOWN_LEFT, DPAD_LEFT, DPAD_UP_LEFT
 * (or HAT_CENTERED, HAT_UP etc)
 *
 * bleGamepad.setAxes takes the following int16_t parameters for the Left/Right Thumb X/Y, Left/Right Triggers plus slider1 and slider2, and hat switch position as above: 
 * (Left Thumb X, Left Thumb Y, Right Thumb X, Right Thumb Y, Left Trigger, Right Trigger, Hat switch position 
 ^ (1 hat switch (dpad) supported by default. Library can be configured to support up to 4)
 *
 * Library can also be configured to support up to 5 simulation controls (can be set with setSimulationControls)
 * (rudder, throttle, accelerator, brake, steering), but they are not enabled by default.
 */
 
/*
 * Custom controller for zhai1987
 */

#include <ESP32Encoder.h> // https://github.com/madhephaestus/ESP32Encoder
#include <Keypad.h>       // https://github.com/Chris--A/Keypad
#include <BleGamepad.h>   // https://github.com/lemmingDev/ESP32-BLE-Gamepad

// Create encoder objects
ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3;
ESP32Encoder encoder4;

// Variables to store the current encoder count
int32_t encoder1Count = 0;
int32_t encoder2Count = 0;
int32_t encoder3Count = 0;
int32_t encoder4Count = 0;

BleGamepad bleGamepad("BLE Racing Controls", "lemmingDev", 100);  //Shows how you can customise the device name, manufacturer name and initial battery level

#define ROWS 5
#define COLS 4
uint8_t rowPins[ROWS] = {13, 14, 15, 16, 33};    //ESP32 pins used for rows      --> adjust to suit --> Pinout on board: R1, R2, R3, R4
uint8_t colPins[COLS] = {17, 18, 19, 21};    //ESP32 pins used for columns   --> adjust to suit --> Pinout on board: Q1, Q2, Q3, Q4
uint8_t keymap[ROWS][COLS] = 
{
  {1, 2, 3, 4},      //Buttons  1,  2,  3,  4      --> Used for calulating the bitmask for sending to the library 
  {5, 6, 7, 8},      //Buttons  5,  6,  7,  8      --> Adjust to suit which buttons you want the library to send
  {9, 10,11,12},     //Buttons  9, 10, 11, 12      --> 
  {13,14,15,16},     //Buttons 13, 14, 15, 16      --> Eg. The value 12 in the array refers to button 12
  {18,17,18,17}
};

Keypad customKeypad = Keypad( makeKeymap(keymap), rowPins, colPins, ROWS, COLS);

//Potentiometers
const int pot1Pin = 34;                // Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 
const int pot2Pin = 35;                // Potentiometer is connected to GPIO 34 (Analog ADC1_CH7) 
const int numberOfPotSamples = 10;     // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 4;    // Delay in milliseconds between pot samples

void KeypadUpdate()
{
  customKeypad.getKeys();
  
  for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.      //LIST_MAX is provided by the Keypad library and gives the number of buttons of the Keypad instance
  {
    if ( customKeypad.key[i].stateChanged )   //Only find keys that have changed state.
    {
      uint8_t keystate = customKeypad.key[i].kstate;

      if (keystate==PRESSED)  {
        Serial.println("key PRESSED---->");
         bleGamepad.press(customKeypad.key[i].kchar); }    //Press or release button based on the current state
      if (keystate==RELEASED) {  Serial.println("key RELEASED---->");
      bleGamepad.release(customKeypad.key[i].kchar); }
      
      bleGamepad.sendReport();    //Send the HID report after values for all button states are updated, and at least one button state had changed
      
    }
  }
}

void setup()
{
  analogReadResolution(9);
  
  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors=UP;

  // Attach pins to encoders
  encoder1.attachHalfQuad(22, 23);
  encoder2.attachHalfQuad(26, 25);
  encoder3.attachHalfQuad(32, 27);
  encoder4.attachHalfQuad(5, 4);

  // Clear encoder counts
  encoder1.clearCount();
  encoder2.clearCount();
  encoder3.clearCount();
  encoder4.clearCount();

  // Initialise encoder counts
  encoder1Count = encoder1.getCount();
  encoder2Count = encoder2.getCount();
  encoder3Count = encoder3.getCount();
  encoder4Count = encoder4.getCount();
  
  bleGamepad.begin(26, 0, false, false, false, false, false, false, false, false, false, true, false, true, false );        //Begin library with default buttons/hats/axes
  bleGamepad.setAutoReport(false);  //Disable auto reports --> You then need to force HID updates with bleGamepad.sendReport()
}

void EncodersUpdate()
{
  int32_t tempEncoderCount1 = encoder1.getCount();
  int32_t tempEncoderCount2 = encoder2.getCount();
  int32_t tempEncoderCount3 = encoder3.getCount();
  int32_t tempEncoderCount4 = encoder4.getCount();
  
  bool sendReport = false;
  
  if(tempEncoderCount1 != encoder1Count) 
  {
    sendReport = true;
    
    if(tempEncoderCount1 > encoder1Count)
    {
      bleGamepad.press(BUTTON_19);
    }
    else
    {
      bleGamepad.press(BUTTON_20);
    } 
  }
  
  if(tempEncoderCount2 != encoder2Count) 
  {
    sendReport = true;
    
    if(tempEncoderCount2 > encoder2Count)
    {
      bleGamepad.press(BUTTON_21);
    }
    else
    {
      bleGamepad.press(BUTTON_22);
    }
  }
  
  if(tempEncoderCount3 != encoder3Count) 
  {
    sendReport = true;
    
    if(tempEncoderCount3 > encoder3Count)
    {
      bleGamepad.press(BUTTON_23);
    }
    else
    {
      bleGamepad.press(BUTTON_24);
    }
  }
  
  if(tempEncoderCount4 != encoder4Count) 
  {
    sendReport = true;
    
    if(tempEncoderCount4 > encoder4Count)
    {
      bleGamepad.press(BUTTON_25);
    }
    else
    {
      bleGamepad.press(BUTTON_26);
    }
  }
  
  if (sendReport)
  {
    bleGamepad.sendReport();
    delay(10);

    bleGamepad.release(BUTTON_19);
    bleGamepad.release(BUTTON_20);
    bleGamepad.release(BUTTON_21);
    bleGamepad.release(BUTTON_22);
    bleGamepad.release(BUTTON_23);
    bleGamepad.release(BUTTON_24);
    bleGamepad.release(BUTTON_25);
    bleGamepad.release(BUTTON_26);
    bleGamepad.sendReport();
    delay(10);

    encoder1Count = encoder1.getCount();
    encoder2Count = encoder2.getCount();
    encoder3Count = encoder3.getCount();
    encoder4Count = encoder4.getCount();
  } 
}

void PotentiometersUpdate()
{
    int pot1Values[numberOfPotSamples];  // Array to store pot readings
    int pot1Value = 0;   // Variable to store calculated pot reading average

    int pot2Values[numberOfPotSamples];  // Array to store pot readings
    int pot2Value = 0;   // Variable to store calculated pot reading average

    // Populate readings
    for(int i = 0 ; i < numberOfPotSamples ; i++)
    {
      pot1Values[i] = analogRead(pot1Pin);
      pot1Value += pot1Values[i];
      pot2Values[i] = analogRead(pot2Pin);
      pot2Value += pot2Values[i];
      
      delay(delayBetweenSamples);
    }

    // Calculate the average
    pot1Value = pot1Value / numberOfPotSamples;
    pot2Value = pot2Value / numberOfPotSamples;

    // Map analog reading from 0 ~ 411 to 32737 ~ -32737 for use as an axis reading
    int adjustedPot1Value = map(pot1Value, 0, 511, 32737, -32737);
    int adjustedPot2Value = map(pot2Value, 0, 511, 32737, -32737);

    // Update X axis and send report
    bleGamepad.setThrottle(adjustedPot1Value);
    bleGamepad.setBrake(adjustedPot2Value);
    bleGamepad.sendReport();
}

void loop() 
{
   if(bleGamepad.isConnected()) 
    {
      KeypadUpdate();
      EncodersUpdate();
      PotentiometersUpdate();
      delay(10);
    }
}
