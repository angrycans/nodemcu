// 定义 CNC Shield X 轴的引脚
#include <Arduino.h>

const int enPin = 8;     // 使能引脚，低电平有效
const int stepPin = 2;   // 步进引脚
const int dirPin = 5;    // 方向引脚

void setup() {
  pinMode(enPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  digitalWrite(enPin, LOW); // 开启驱动
}

void loop() {
  digitalWrite(dirPin, HIGH); // 设置旋转方向
  for(int x = 0; x < 200; x++) { // 假设旋转200步
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000); // 改变这个值可以改变转速
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
  
  delay(1000); // 停顿一秒
  
  digitalWrite(dirPin, LOW); // 改变方向
  for(int x = 0; x < 200; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
  
  delay(1000);
}