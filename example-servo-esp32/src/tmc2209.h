#pragma once

#include <Arduino.h>
#include <TMCStepper.h>

// UART1 pins (single-wire PDN_UART tied to TX+RX)
static const int TMC_UART_TX = 42;
static const int TMC_UART_RX = 41;
static const int TMC_EN_PIN = 9;
static const int TMC_UART2_TX = 10;
static const int TMC_UART2_RX = 46;

static const uint32_t TMC_UART_BAUD = 115200;
static const float TMC_R_SENSE = 0.10f;

static const uint16_t TMC_RMS_CURRENT_MA = 1000;
static const uint16_t TMC_MICROSTEPS = 32; // 配合插值实现 256

static HardwareSerial &TMCSerial = Serial1;
static HardwareSerial &TMCSerial2 = Serial2;
static TMC2209Stepper tmc2209_0(&TMCSerial, TMC_R_SENSE, 0b00);
static TMC2209Stepper tmc2209_1(&TMCSerial, TMC_R_SENSE, 0b01);
static TMC2209Stepper tmc2209_2(&TMCSerial, TMC_R_SENSE, 0b10);
static TMC2209Stepper tmc2209_3(&TMCSerial2, TMC_R_SENSE, 0b00);
static TMC2209Stepper tmc2209_4(&TMCSerial2, TMC_R_SENSE, 0b01);
static TMC2209Stepper tmc2209_5(&TMCSerial2, TMC_R_SENSE, 0b10);

inline void initTmc2209() {
    Serial.println("Tmc2209 init start");

  pinMode(TMC_EN_PIN, OUTPUT);
  digitalWrite(TMC_EN_PIN, LOW); // 低电平使能

  TMCSerial.begin(TMC_UART_BAUD, SERIAL_8N1, TMC_UART_RX, TMC_UART_TX);
  TMCSerial2.begin(TMC_UART_BAUD, SERIAL_8N1, TMC_UART2_RX, TMC_UART2_TX);

  TMC2209Stepper *drivers[] = {
      &tmc2209_0, &tmc2209_1, &tmc2209_2,
      &tmc2209_3, &tmc2209_4, &tmc2209_5};
  for (auto *drv : drivers) {
    drv->begin();
    drv->toff(5);
    drv->blank_time(24);
    drv->rms_current(TMC_RMS_CURRENT_MA);
    drv->microsteps(TMC_MICROSTEPS);
    drv->intpol(true); // 256 细分插值

    // UART control
    drv->I_scale_analog(false);
    drv->pdn_disable(true);

    // Quiet default
    drv->en_spreadCycle(false); // StealthChop
    drv->pwm_autoscale(true);
    drv->pwm_autograd(true);
  }

    Serial.println("Tmc2209 init ok");

}
