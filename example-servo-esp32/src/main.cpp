#include <Arduino.h>
#include <FastAccelStepper.h>
#include <EEPROM.h>

static const char *FW_VERSION = "0.7.1";

#define SERIAL_BAUD 115200

/*************************************************
 *  驱动类型选择
 *************************************************/
#define USE_SERVO // ← 用伺服时打开
// #define USE_STEPPER // ← 用步进时打开

#ifdef USE_STEPPER
#include "tmc2209.h"
#endif

/*************************************************
 *  步进电机参数（测试阶段）
 *************************************************/
#ifdef USE_STEPPER
#define AXIS_NUM 6
// ===== GPIO 定义（可按你实际接线改）=====
const uint8_t STEP_PIN[AXIS_NUM] = {5, 7, 11, 13, 19, 15};
const uint8_t DIR_PIN[AXIS_NUM] = {4, 6, 8, 12, 14, 20};
const uint8_t LIMIT_PIN[AXIS_NUM] = {35, 36, 37, 38, 39, 40};
// false = 默认方向, true = 软件反向
float LEAD_MM_PER_REV[AXIS_NUM] = {
    4,
    4,
    4,
    4,
    4,
    4};

bool Motor_Inverted[AXIS_NUM] = {false, false, false, false, false, false};
bool Motor_Enable[AXIS_NUM] = {true, true, true, true, true, true};
// ===== 电机行程参数 =====
float AXIS_STROKE_MM[AXIS_NUM] = {
    100.0f, // Axis 1
    100.0f, // Axis 2
    100.0f, // Axis 3
    100.0f, // Axis 4
    100.0f, // Axis 5
    100.0f, // Axis 6

};

// ===== Homing 参数 =====

// ① 预退让距离：确保一开始不压在限位上
const float AXIS_HOME_PREMOVE_MM[AXIS_NUM] = {10, 10, 10, 10, 10, 10};
const float AXIS_HOME_BACKOFF_MM[AXIS_NUM] = {2, 2, 2, 2, 2, 2};

#define MOTOR_STEPS_REV 200 // 1.8° 步进
#define MICROSTEPS 32       // 32 细分

// ===== 运动参数 =====
#define SPEED_HZ 200000 // 125 mm/s (~750 RPM)
#define ACCEL 400000    // 250 mm/s²

// ===== Homing 参数 =====
#define HOME_SPEED 3000
#define HOME_ACCEL 6000
#define EXEC_INTERVAL_MS 2

// ===== 自适应 DEADZONE（物理量）=====
#define DEADZONE_MM_MIN 0.01f // 10μm（低速精细）
#define DEADZONE_MM_MAX 0.05f // 50μm（高速稳定）

// 红蓝LED引脚定义
const int redPin = 45;
const int bluePin = 47;

int32_t calcAdaptiveDeadzoneSteps(
    int32_t newTarget,
    int32_t lastTarget,
    uint32_t dt_ms, int axis);
void applyTargets();
#endif

/*************************************************
 *  伺服电机参数（工业阶段）
 *************************************************/
#ifdef USE_SERVO
#define AXIS_NUM 7
// ===== GPIO 定义（可按你实际接线改）=====
const uint8_t STEP_PIN[AXIS_NUM] = {5, 7, 11, 13, 19, 15, 3};
const uint8_t DIR_PIN[AXIS_NUM] = {4, 6, 8, 12, 14, 20, 10};
const uint8_t LIMIT_PIN[AXIS_NUM] = {35, 36, 37, 38, 39, 40, 21};
// 电机反向设置
bool Motor_Inverted[AXIS_NUM] = {false, false, false, false, false, false, false};
bool Motor_Enable[AXIS_NUM] = {true, true, true, true, true, true, true};
// ===== 电机行程参数 =====
float AXIS_STROKE_MM[AXIS_NUM] = {
    100.0f, // Axis 1
    100.0f, // Axis 2
    100.0f, // Axis 3
    100.0f, // Axis 4
    100.0f, // Axis 5
    100.0f, // Axis 6
    100.0f  // Axis 7
};

// ===== Homing 参数 =====

// ① 预退让距离：确保一开始不压在限位上
const float AXIS_HOME_PREMOVE_MM[AXIS_NUM] = {10, 10, 10, 10, 10, 10, 10};
const float AXIS_HOME_BACKOFF_MM[AXIS_NUM] = {2, 2, 2, 2, 2, 2, 2};

// false = 默认方向, true = 软件反向
#define MOTOR_STEPS_REV 1000 // 等效电子齿轮
#define MICROSTEPS 1         // 已抽象

// ===== 运动参数 =====
#define SPEED_HZ 150000 // ≈ 100mm/s
#define ACCEL 120000

// ===== Homing 参数 =====
#define HOME_SPEED 3000
#define HOME_ACCEL 8000
#define EXEC_INTERVAL_MS 1

float LEAD_MM_PER_REV[AXIS_NUM] = {
    5.0f,
    5.0f,
    5.0f,
    5.0f,
    5.0f,
    5.0f,
    5.0f,
};
// 红蓝LED引脚定义
const int redPin = 45;
const int bluePin = 47;

#endif

FastAccelStepperEngine engine;
FastAccelStepper *stepper[AXIS_NUM];

// ===== 状态变量 =====

int32_t targetStep[AXIS_NUM] = {0};
int32_t lastTargetStep[AXIS_NUM] = {0};
uint32_t lastExecTime[AXIS_NUM] = {0};

bool homed[AXIS_NUM] = {false};
bool homingActive[AXIS_NUM] = {false};

// ===== FlyPT 16bit 全量定义 =====
const uint32_t POS_HOME = 0;
// const uint32_t POS_CNTR = 32000; // 中位（逻辑）
const uint32_t POS_MAXI = 65535; // 旧代码里用 64000，不用 65535

// ===== 电机安全范围（抽象步数）=====
#define RANGE_DZ_MM 1.0f // 1mm 安全边界

// ===== 电机每个轴位置变量 =====
int32_t motor_mins[AXIS_NUM] = {0};

int32_t motor_maxs[AXIS_NUM] = {0};

// ===== 每轴逻辑中位（步数）=====
uint32_t motor_center[AXIS_NUM];

enum HomingState
{
  HOME_IDLE = 0,
  HOME_PREMOVE, // ① 反向预退让
  HOME_SEEK,    // ② 向限位靠近
  HOME_BACKOFF, // ④ 回退
  HOME_HOMED,   // 完成且成功
  HOME_FAILED   // 完成但失败
};

HomingState homingState[AXIS_NUM];
int32_t homingSeekTarget[AXIS_NUM];

int HomingActiveNum = 0;

// --- 串口协议 ---
// FlyPT 格式建议: Y7<M1><M2><M3><M4><M5><M6><M7>!!
const char startMark1 = 'Y';
const char startMark2 = '7';
const char endMark1 = '!';
const char endMark2 = '!';

// ===== EEPROM结构体定义 =====
struct AxisConfig
{
  float lead;
  float stroke;
  bool invert;
  bool enable;
};

struct EepromConfig
{
  uint32_t magic;
  uint16_t version;
  AxisConfig axis[AXIS_NUM];
  bool auto_home;
};

static const uint32_t EEPROM_MAGIC = 0x46504C54; // "FPLT"
static const uint16_t EEPROM_VERSION = 3;
EepromConfig cfg;
bool autoHomeOnBoot = true;

// ===== 函数声明 =====
void setup();
void loop();
void startHoming(uint8_t axis);
void handleSerial();
void handleHoming();
uint32_t cmap_uint32(uint32_t x,
                     uint32_t in_min,
                     uint32_t in_max,
                     uint32_t out_min,
                     uint32_t out_max);
uint32_t mapToRangeX(uint8_t axis, uint16_t pos);
void initAxisRange();
float STEPS_PER_MM(int i);
int32_t home_premove_steps(int i);
int32_t home_backoff_steps(int i);
uint32_t RANGE_DZ(int i);
void testMoveMinus100mm(uint8_t axis);
void syncToCfg();
void syncFromCfg();
void saveParams();
void loadParams();
void handleCmd();
String getBoardName();
void sendBoardInfo();
void sendFirmwareInfo();
void sendConfigJson();
void sendHomeStatus();
bool anyHomingActive();

bool testMode = false;

void setup()
{

  Serial.begin(SERIAL_BAUD);
  delay(1000);

  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  digitalWrite(redPin, LOW);
  digitalWrite(bluePin, HIGH);

  Serial.println("setup start...");

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  EEPROM.begin(sizeof(cfg));
#endif

  loadParams();

#ifdef USE_STEPPER
  initTmc2209();
#endif

  engine.init();
  for (int i = 0; i < AXIS_NUM; i++)
  {
    pinMode(LIMIT_PIN[i], INPUT_PULLUP);
  }

  for (int i = 0; i < AXIS_NUM; i++)
  {
    stepper[i] = engine.stepperConnectToPin(STEP_PIN[i]);
    if (!stepper[i])
    {
      Serial.printf("Stepper %d failed\n", i);
      continue;
    }

    stepper[i]->setDirectionPin(DIR_PIN[i], Motor_Inverted[i]);
    stepper[i]->setAutoEnable(true);

    stepper[i]->setSpeedInHz(SPEED_HZ);
    stepper[i]->setAcceleration(ACCEL);
    stepper[i]->setCurrentPosition(0);

    homed[i] = false;
    homingActive[i] = false;
    homingState[i] = HOME_IDLE;
  }

  initAxisRange();

  Serial.println("ESP32-S3 FlyPT 6DOF READY");

  uint32_t waitStart = millis();
  while (millis() - waitStart < 2000)
  {
    if (!Serial.available())
      continue;
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line == "HELLO")
    {
      sendBoardInfo();
      sendConfigJson();
      testMode = true;
      break;
    }
  }

  HomingActiveNum = 0;
  if (autoHomeOnBoot)
  {
    for (int i = 0; i < AXIS_NUM; i++)
    {
      startHoming(i);
    }
  }
  else
  {
    for (int i = 0; i < AXIS_NUM; i++)
    {
     

        stepper[i]->setSpeedInHz(SPEED_HZ);
        stepper[i]->setAcceleration(ACCEL);
        stepper[i]->setCurrentPosition(0);

        homingActive[i] = false;
        homed[i] = true;
        homingState[i] = HOME_HOMED;
    }
  }

  digitalWrite(redPin, HIGH);
  digitalWrite(bluePin, LOW);
}

void loop()
{

  if (anyHomingActive())
  {
    handleHoming();
  }
  handleSerial();
}

void handleSerial()
{
  while (Serial.available() >= (2 + AXIS_NUM * 2 + 2))
  {
    if (Serial.peek() != startMark1)
      break;
    if (Serial.read() != startMark1)
      continue;
    if (Serial.read() != startMark2)
      continue;

    for (int i = 0; i < AXIS_NUM; i++)
    {
      uint16_t raw =
          ((uint16_t)Serial.read() << 8) |
          (uint16_t)Serial.read();

      if (Motor_Enable[i])
      {
        targetStep[i] = mapToRangeX(i, raw);
      }
    }

    uint8_t em1 = Serial.read();
    uint8_t em2 = Serial.read();

    if (em1 != endMark1 || em2 != endMark2)
      continue;

#ifdef USE_STEPPER
    applyTargets();
#endif

#ifdef USE_SERVO
    for (int i = 0; i < AXIS_NUM; i++)
    {
      if (!Motor_Enable[i])
        continue;
      if (!homed[i])
        continue;
      if (homingState[i] != HOME_HOMED)
        continue;
      stepper[i]->moveTo(targetStep[i]);
    }
#endif
  }

  // Only parse text commands when the buffer isn't a binary frame head
  if (Serial.available() > 0 && Serial.peek() != startMark1)
  {
    handleCmd();
  }
}

void startHoming(uint8_t axis)
{

  if (axis >= AXIS_NUM)
    return;

  if (homed[axis] && HomingActiveNum > 0)
  {
    HomingActiveNum--;
  }
  homed[axis] = false;
  homingActive[axis] = true;
  homingState[axis] = HOME_PREMOVE;

  stepper[axis]->forceStop();
  stepper[axis]->setSpeedInHz(HOME_SPEED);
  stepper[axis]->setAcceleration(HOME_ACCEL);
  stepper[axis]->setCurrentPosition(0);

  // ⭐ ① 预退让：远离限位
  stepper[axis]->move(home_premove_steps(axis));

  Serial.printf("Axis %d homing: PREMOVE\n", axis + 1);
}

bool anyHomingActive()
{
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (homingActive[i])
      return true;
  }
  return false;
}

void handleHoming()
{
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (!homingActive[i])
      continue;

    switch (homingState[i])
    {
      // ===== ① 预退让完成 → 开始找限位 =====
    case HOME_PREMOVE:
      if (!stepper[i]->isRunning())
      {
        homingState[i] = HOME_SEEK;

        stepper[i]->setSpeedInHz(HOME_SPEED);
        stepper[i]->setAcceleration(HOME_ACCEL);
        stepper[i]->setCurrentPosition(0);

        // ⭐ 绝对 SEEK 目标（一次性）
        homingSeekTarget[i] =
            (motor_mins[i] - (motor_maxs[i] - motor_mins[i] + home_backoff_steps(i))) / 2;

        stepper[i]->moveTo(homingSeekTarget[i]);

        Serial.printf(
            "Axis %d homing: SEEK to %ld\n",
            i + 1,
            homingSeekTarget[i]);
      }
      break;

      // ===== ② 找到限位 =====
    case HOME_SEEK:
    {
      // ===== 命中限位 =====
      if (digitalRead(LIMIT_PIN[i]) == LOW)
      {
        stepper[i]->forceStop();

      
        stepper[i]->setCurrentPosition(motor_mins[i]);

        homingState[i] = HOME_BACKOFF;

        stepper[i]->moveTo(motor_mins[i] + home_backoff_steps(i));

        Serial.printf("Axis %d homing: HIT LIMIT\n", i + 1);

        break; // ⭐ 必须立即退出
      }

      // ===== SEEK 走到目标但没命中限位 =====
      if (!stepper[i]->isRunning())
      {

        // homingState[i] = HOME_BACKOFF;
        homingState[i] = HOME_FAILED;
        homingActive[i] = false;
        homed[i] = false;
        HomingActiveNum++;

        Serial.printf(
            "ERROR: Axis %d homing FAILED (limit not found) HomingActiveNum=%d\n",
            i + 1, HomingActiveNum);
        sendHomeStatus();
        break;
        ;
      }
    }
    break;

    // ===== ③ 回退完成 → Homing 完成 =====
    case HOME_BACKOFF:
      if (!stepper[i]->isRunning())
      {

        // 恢复正常速度
        stepper[i]->setSpeedInHz(SPEED_HZ);
        stepper[i]->setAcceleration(ACCEL);
        stepper[i]->setCurrentPosition(0);

        // ⭐ 自动到中位
        // stepper[i]->moveTo(motor_center[i]);


        homingActive[i] = false;
        homed[i] = true;
        homingState[i] = HOME_HOMED;
        HomingActiveNum++;
        Serial.printf("Axis %d homing: DONE HomingActiveNum=%d\n", i + 1, HomingActiveNum);

        sendHomeStatus();
      }
      break;

    default:
      break;
    }
  }
}

uint32_t cmap_uint32(uint32_t x,
                     uint32_t in_min,
                     uint32_t in_max,
                     uint32_t out_min,
                     uint32_t out_max)
{
  uint64_t rv =
      (uint64_t)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

  if (rv > out_max)
    rv = out_max;
  if (rv < out_min)
    rv = out_min;

  return (uint32_t)rv;
}

uint32_t mapToRangeX(uint8_t axis, uint16_t pos)
{
  return cmap_uint32(pos, POS_HOME, POS_MAXI, motor_mins[axis] + RANGE_DZ(axis), motor_maxs[axis] - RANGE_DZ(axis));
}

void testMoveMinus100mm(uint8_t axis)
{
  if (axis >= AXIS_NUM)
    return;
  if (!stepper[axis])
    return;

  // ===== 计算步数 =====

  const int32_t move_steps = (int32_t)(100.0f * STEPS_PER_MM(axis)); // 100mm

  Serial.printf(
      "Axis %d test move: -100mm => -%ld steps\n",
      axis + 1,
      move_steps);

  // ===== 设置测试速度（不要太快，防丢步）=====
  stepper[axis]->setSpeedInHz(2000);    // 2000 steps/s
  stepper[axis]->setAcceleration(4000); // 温和加速

  // ===== 从“已知零点”开始 =====
  stepper[axis]->forceStop();
  stepper[axis]->setCurrentPosition(0);

  // ===== 反向运动 100mm =====
  stepper[axis]->moveTo(move_steps);
}

#ifdef USE_STEPPER
int32_t calcAdaptiveDeadzoneSteps(
    int32_t newTarget,
    int32_t lastTarget,
    uint32_t dt_ms, int axis)
{

  if (dt_ms == 0)
    dt_ms = 1;
  // 目标速度（steps/ms）
  int32_t delta = newTarget - lastTarget;
  float v = fabsf((float)delta) / (float)dt_ms;

  // 经验上：5~50 steps/ms 是“慢→快”区间
  float alpha = constrain(v / 50.0f, 0.0f, 1.0f);

  float dz_mm =
      DEADZONE_MM_MIN +
      alpha * (DEADZONE_MM_MAX - DEADZONE_MM_MIN);

  return (int32_t)(dz_mm * STEPS_PER_MM(axis));
}

void applyTargets()
{
  uint32_t now = millis();
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (!Motor_Enable[i])
      continue;
    if (!homed[i])
      continue;

    // Serial.println("applyTargets");

    uint32_t dt = now - lastExecTime[i];
    if (dt < EXEC_INTERVAL_MS)
      continue;
    lastExecTime[i] = now;

    int32_t newTarget = targetStep[i];
    int32_t lastTarget = lastTargetStep[i];

    int32_t dz = calcAdaptiveDeadzoneSteps(
        newTarget,
        lastTarget,
        dt, i);

    int32_t diff = (int32_t)targetStep[i] - (int32_t)lastTargetStep[i];
    if (diff > dz || diff < -dz)

    {
      stepper[i]->moveTo(newTarget);
      lastTargetStep[i] = newTarget;
    }
  }
}

#endif

void initAxisRange()
{
  for (int i = 0; i < AXIS_NUM; i++)
  {
    motor_maxs[i] =
        (int32_t)(AXIS_STROKE_MM[i] * STEPS_PER_MM(i));

    motor_center[i] =
        (motor_mins[i] + motor_maxs[i]) / 2;
  }
}

int32_t home_premove_steps(int i)
{
  return (int32_t)(AXIS_HOME_PREMOVE_MM[i] * STEPS_PER_MM(i));
}

int32_t home_backoff_steps(int i)
{
  return (int32_t)(AXIS_HOME_BACKOFF_MM[i] * STEPS_PER_MM(i));
}

float STEPS_PER_MM(int i)
{
  return (MOTOR_STEPS_REV * MICROSTEPS) / LEAD_MM_PER_REV[i]; // 800
}

uint32_t RANGE_DZ(int i)
{
  return (uint32_t)(RANGE_DZ_MM * STEPS_PER_MM(i));
}

void syncToCfg()
{
  for (int i = 0; i < AXIS_NUM; i++)
  {
    cfg.axis[i].lead = LEAD_MM_PER_REV[i];
    cfg.axis[i].stroke = AXIS_STROKE_MM[i];
    cfg.axis[i].invert = Motor_Inverted[i];
    cfg.axis[i].enable = Motor_Enable[i];
  }
  cfg.auto_home = autoHomeOnBoot;
}

void syncFromCfg()
{
  for (int i = 0; i < AXIS_NUM; i++)
  {
    LEAD_MM_PER_REV[i] = cfg.axis[i].lead;
    AXIS_STROKE_MM[i] = cfg.axis[i].stroke;
    Motor_Inverted[i] = cfg.axis[i].invert;
    Motor_Enable[i] = cfg.axis[i].enable;
  }
  autoHomeOnBoot = cfg.auto_home;
}
void saveParams()
{
  cfg.magic = EEPROM_MAGIC;
  cfg.version = EEPROM_VERSION;
  syncToCfg();
  EEPROM.put(0, cfg);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  EEPROM.commit();
#endif
}

void loadParams()
{
  EEPROM.get(0, cfg);
  if (cfg.magic != EEPROM_MAGIC || cfg.version != EEPROM_VERSION)
  {
    cfg.magic = EEPROM_MAGIC;
    cfg.version = EEPROM_VERSION;
    syncToCfg();
    EEPROM.put(0, cfg);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    EEPROM.commit();
#endif
    return;
  }
  syncFromCfg();
}

String getBoardName()
{
#if defined(ARDUINO_AVR_MEGA2560)
  return "MEGA2560";
#elif defined(ARDUINO_ARCH_ESP32)
  String name = String(ESP.getChipModel());
  return name.length() ? name : "ESP32";
#elif defined(ARDUINO_ARCH_ESP8266)
  return "ESP8266";
#else
  return "Unknown";
#endif
}

void sendBoardInfo()
{
  Serial.print("BOARD:");
  Serial.print(getBoardName());
#ifdef USE_SERVO
  Serial.println(" Servo");
#elif defined(USE_STEPPER)
  Serial.println(" Stepper");
#else
  Serial.println();
#endif
}

void sendFirmwareInfo()
{
  Serial.print("FW:");
  Serial.println(FW_VERSION);
}

void sendConfigJson()
{
  Serial.print("CFG ");
  Serial.print("{\"stroke\":[");
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (i)
      Serial.print(",");
    Serial.print(AXIS_STROKE_MM[i]);
  }
  Serial.print("],\"lead\":[");
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (i)
      Serial.print(",");
    Serial.print(LEAD_MM_PER_REV[i], 3);
  }
  Serial.print("],\"invert\":[");
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (i)
      Serial.print(",");
    Serial.print(Motor_Inverted[i] ? 1 : 0);
  }
  Serial.print("],\"enable\":[");
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (i)
      Serial.print(",");
    Serial.print(Motor_Enable[i] ? 1 : 0);
  }
  Serial.print("],\"auto_home\":");
  Serial.print(autoHomeOnBoot ? 1 : 0);
  Serial.println("}");
}

void sendHomeStatus()
{
  Serial.print("HOME ");
  Serial.print("{\"state\":[");
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (i)
      Serial.print(",");
    int state = 0;
    switch (homingState[i])
    {
    case HOME_PREMOVE:
    case HOME_SEEK:
    case HOME_BACKOFF:
      state = 1; // HOMING
      break;
    case HOME_HOMED:
      state = 2; // HOMED
      break;
    case HOME_FAILED:
      state = 3; // FAILED
      break;
    case HOME_IDLE:
    default:
      state = 0; // IDLE
      break;
    }
    Serial.print(state);
  }
  Serial.println("]}");
}

void handleCmd()
{
  // 只处理完整一行文本指令
  if (!Serial.available())
    return;

  String line = Serial.readStringUntil('\n');
  line.trim(); // 去掉回车换行

  if (line.length() == 0)
    return;

  // HELLO 自报板名
  if (line == "HELLO")
  {

    printf("Received HELLO command\n");
    sendBoardInfo();
    sendFirmwareInfo();
    sendConfigJson();
    sendHomeStatus();
    Serial.println("HELLO OK");
    return;
  }

  if (line.startsWith("SET STROKE "))
  {
    String rest = line.substring(11);
    int sp = rest.indexOf(' ');
    if (sp > 0)
    {
      int axis = rest.substring(0, sp).toInt();
      float stroke = rest.substring(sp + 1).toFloat();
      if (axis >= 0 && axis < AXIS_NUM)
      {
        AXIS_STROKE_MM[axis] = stroke;
        motor_maxs[axis] = (int32_t)(AXIS_STROKE_MM[axis] * STEPS_PER_MM(axis));
        motor_center[axis] = (motor_mins[axis] + motor_maxs[axis]) / 2;
        saveParams();
        Serial.printf("AXIS %d STROKE %.3f\n", axis, stroke);
      }
    }
    Serial.println("SET STROKE OK");
    return;
  }

  if (line.startsWith("SET LEAD "))
  {
    String rest = line.substring(9);
    int sp = rest.indexOf(' ');
    if (sp > 0)
    {
      int axis = rest.substring(0, sp).toInt();
      float lead = rest.substring(sp + 1).toFloat();
      if (axis >= 0 && axis < AXIS_NUM)
      {
        LEAD_MM_PER_REV[axis] = lead;
        saveParams();
        Serial.printf("AXIS %d LEAD %.3f\n", axis, lead);
      }
    }
    Serial.println("SET LEAD OK");
    return;
  }

  if (line.startsWith("SET INVERT "))
  {
    String rest = line.substring(11);
    int sp = rest.indexOf(' ');
    if (sp > 0)
    {
      int axis = rest.substring(0, sp).toInt();
      int inv = rest.substring(sp + 1).toInt();
      if (axis >= 0 && axis < AXIS_NUM)
      {
        Motor_Inverted[axis] = (inv != 0);
        if (stepper[axis])
        {
          stepper[axis]->setDirectionPin(DIR_PIN[axis], Motor_Inverted[axis]);
        }
        saveParams();
        Serial.printf("AXIS %d INVERT %d\n", axis, Motor_Inverted[axis] ? 1 : 0);
      }
    }
    Serial.println("SET INVERT OK");
    return;
  }

  if (line.startsWith("SET AUTO_HOME "))
  {
    int val = line.substring(14).toInt();
    autoHomeOnBoot = (val != 0);
    saveParams();
    Serial.printf("AUTO_HOME %d\n", autoHomeOnBoot ? 1 : 0);
    Serial.println("SET AUTO_HOME OK");
    return;
  }

  if (line.startsWith("HOME AXIS "))
  {
    int axis = line.substring(10).toInt();
    if (axis >= 0 && axis < AXIS_NUM)
    {
      if (Motor_Enable[axis])
      {
        startHoming(axis);
        Serial.printf("Axis %d homing: START\n", axis + 1);
        sendHomeStatus();
      }
    }
    Serial.println("HOME AXIS OK");
    return;
  }

  if (line.startsWith("HOME OK "))
  {
    int axis = line.substring(8).toInt();
    if (axis >= 0 && axis < AXIS_NUM)
    {
      if (Motor_Enable[axis])
      {
        homingActive[axis] = false;
        homed[axis] = true;
        homingState[axis] = HOME_HOMED;
        sendHomeStatus();
      }
    }
    Serial.println("HOME OK OK");
    return;
  }

  if (line == "HOME ALL")
  {
    for (int i = 0; i < AXIS_NUM; i++)
    {
      if (Motor_Enable[i])
      {
        startHoming(i);
      }
    }
    Serial.println("Axis ALL homing: START");
    sendHomeStatus();
    Serial.println("HOME ALL OK");
    return;
  }

  if (line == "REBOOT")
  {
    Serial.println("REBOOT OK");
    Serial.flush();
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    delay(50);
    ESP.restart();
#else
    Serial.println("REBOOT UNSUPPORTED");
#endif
    return;
  }

  // // ENABLE/DISABLE AXIS
  if (line.startsWith("ENABLE AXIS "))
  {
    int axis = line.substring(12).toInt();
    if (axis >= 0 && axis < AXIS_NUM)
    {
      Motor_Enable[axis] = true;
      saveParams();
      Serial.printf("AXIS %d ENABLED\n", axis);
    }
    Serial.println("ENABLE AXIS OK");
    return;
  }

  if (line.startsWith("DISABLE AXIS "))
  {
    int axis = line.substring(13).toInt();
    if (axis >= 0 && axis < AXIS_NUM)
    {
      Motor_Enable[axis] = false;
      Serial.printf("AXIS %d DISABLED\n", axis);
      stepper[axis]->forceStop();
      saveParams();
    }
    Serial.println("DISABLE AXIS OK");
    return;
  }
}
