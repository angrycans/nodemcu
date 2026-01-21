#include <Arduino.h>
#include <FastAccelStepper.h>

#define AXIS_NUM 6
#define SERIAL_BAUD 115200

FastAccelStepperEngine engine;
FastAccelStepper *stepper[AXIS_NUM];

// ===== GPIO 定义（可按你实际接线改）=====
const uint8_t STEP_PIN[AXIS_NUM] = {5, 7, 9, 11, 13, 15};
const uint8_t DIR_PIN[AXIS_NUM] = {4, 6, 8, 10, 12, 14};
const uint8_t LIMIT_PIN[AXIS_NUM] = {35, 36, 37, 38, 39, 40};

/*************************************************
 *  驱动类型选择
 *************************************************/
// #define USE_SERVO   // ← 用伺服时打开
#define USE_STEPPER // ← 用步进时打开

/*************************************************
 *  步进电机参数（测试阶段）
 *************************************************/
#ifdef USE_STEPPER

#define LEAD_MM_PER_REV 4.0f // T6 丝杆导程 4mm
#define MOTOR_STEPS_REV 200  // 1.8° 步进
#define MICROSTEPS 16        // 16 细分

// ===== 运动参数 =====
#define SPEED_HZ 80000 // ≈ 100mm/s
#define ACCEL 120000

// ===== Homing 参数 =====
#define HOME_SPEED 3000
#define HOME_ACCEL 8000
#define EXEC_INTERVAL_MS 10

// ===== 自适应 DEADZONE（物理量）=====
#define DEADZONE_MM_MIN 0.01f // 10μm（低速精细）
#define DEADZONE_MM_MAX 0.05f // 50μm（高速稳定）

int32_t calcAdaptiveDeadzoneSteps(
    int32_t newTarget,
    int32_t lastTarget,
    uint32_t dt_ms);
void applyTargets(uint32_t dt);
#endif

/*************************************************
 *  伺服电机参数（工业阶段）
 *************************************************/
#ifdef USE_SERVO

#define LEAD_MM_PER_REV 10.0f // 1610 丝杆
#define MOTOR_STEPS_REV 1000  // 等效电子齿轮
#define MICROSTEPS 1          // 已抽象

// ===== 运动参数 =====
#define SPEED_HZ 150000 // ≈ 100mm/s
#define ACCEL 120000

// ===== Homing 参数 =====
#define HOME_SPEED 3000
#define HOME_ACCEL 8000
#define EXEC_INTERVAL_MS 1
#endif

// ===== 状态变量 =====

int32_t targetStep[AXIS_NUM] = {0};
int32_t lastTargetStep[AXIS_NUM] = {0};
static uint32_t lastExecTime = 0;

// ===== 电机行程参数 =====

const float STEPS_PER_MM =
    (MOTOR_STEPS_REV * MICROSTEPS) / LEAD_MM_PER_REV; // 800

const float AXIS_STROKE_MM[AXIS_NUM] = {
  100.0f, // Axis 1
  100.0f, // Axis 2
  100.0f,  // Axis 3
  100.0f, // Axis 4
  100.0f,  // Axis 5
  100.0f   // Axis 6
};

// ===== Homing 参数 =====

// ① 预退让距离：确保一开始不压在限位上
const int32_t HOME_PREMOVE_MM = 10; // 10mm
const int32_t HOME_BACKOFF_MM = 2;  // 回退 2mm

const int32_t HOME_PREMOVE_STEPS =
    (int32_t)(HOME_PREMOVE_MM * STEPS_PER_MM);

const int32_t HOME_BACKOFF_STEPS =
    (int32_t)(HOME_BACKOFF_MM * STEPS_PER_MM);

bool homed[AXIS_NUM] = {false};
bool homingActive[AXIS_NUM] = {false};

// ===== FlyPT 16bit 全量定义 =====
const uint32_t POS_HOME = 0;
// const uint32_t POS_CNTR = 32000; // 中位（逻辑）
const uint32_t POS_MAXI = 65535; // 旧代码里用 64000，不用 65535

// ===== 电机安全范围（抽象步数）=====
#define RANGE_DZ_MM 1.0f // 1mm 安全边界
const uint32_t RANGE_DZ =
    (uint32_t)(RANGE_DZ_MM * STEPS_PER_MM);

// ===== 电机每个轴位置变量 =====
uint32_t motor_mins[AXIS_NUM] = {0};

uint32_t motor_maxs[AXIS_NUM] = {0};

// ===== 每轴逻辑中位（步数）=====
uint32_t motor_center[AXIS_NUM];

enum HomingState
{
  HOME_IDLE = 0,
  HOME_PREMOVE, // ① 反向预退让
  HOME_SEEK,    // ② 向限位靠近
  HOME_BACKOFF  // ④ 回退
};

HomingState homingState[AXIS_NUM];
int32_t homingSeekTarget[AXIS_NUM];

// --- 串口协议 ---
// FlyPT 格式建议: P6<M1><M2><M3><M4><M5><M6>!!
const char startMark1 = 'Y';
const char startMark2 = '4'; // 保持你原有的协议头
const char endMark1 = '!';
const char endMark2 = '!';

void setup();
void loop();
void startHoming(uint8_t axis);
void handleSerial();
void handleHoming();
uint16_t cmap_uint16(uint32_t x,
                     uint32_t in_min,
                     uint32_t in_max,
                     uint32_t out_min,
                     uint32_t out_max);
uint32_t mapToRangeX(uint8_t axis, uint16_t pos);
void initAxisRange();
void testMoveMinus100mm(uint8_t axis);

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(2000);
  Serial.println("setup start...");

  engine.init();

  for (int i = 0; i < AXIS_NUM; i++)
  {
    stepper[i] = engine.stepperConnectToPin(STEP_PIN[i]);
    if (!stepper[i])
    {
      Serial.printf("Stepper %d failed\n", i);
      continue;
    }

    stepper[i]->setDirectionPin(DIR_PIN[i]);
    stepper[i]->setAutoEnable(true);

    stepper[i]->setSpeedInHz(SPEED_HZ);
    stepper[i]->setAcceleration(ACCEL);
    stepper[i]->setCurrentPosition(0);

    homed[i] = false;
    homingActive[i] = false;
    homingState[i] = HOME_IDLE;
  }

  for (int i = 0; i < AXIS_NUM; i++)
  {
    pinMode(LIMIT_PIN[i], INPUT_PULLUP);
  }

  initAxisRange();

  Serial.println("ESP32-S3 FlyPT 6DOF READY");

  delay(3000);
  // testMoveMinus100mm(0);
  for (int i = 0; i < AXIS_NUM; i++)
    startHoming(i);
}

void loop()
{
  handleHoming();
  handleSerial();
}
void handleSerial()
{
  while (Serial.available() >= (2 + AXIS_NUM * 2 + 2))
  {
    if (Serial.read() != 'Y')
      continue;
    if (Serial.read() != '4')
      continue;

    for (int i = 0; i < AXIS_NUM; i++)
    {
      uint16_t raw =
          ((uint16_t)Serial.read() << 8) |
          (uint16_t)Serial.read();

      targetStep[i] = mapToRangeX(i, raw);
    }

    uint8_t em1 = Serial.read();
    uint8_t em2 = Serial.read();

    if (em1 != endMark1 || em2 != endMark2)
      continue;

#ifdef USE_STEPPER

    uint32_t now = millis();
    uint32_t dt = now - lastExecTime;
    if (dt < EXEC_INTERVAL_MS)
      return;
    lastExecTime = now;

#endif

#ifdef USE_STEPPER
    applyTargets(dt);
#endif

#ifdef USE_SERVO
    for (int i = 0; i < AXIS_NUM; i++)
    {
      if (!homed[i])
        continue;
      stepper[i]->moveTo(targetStep[i]);
    }
#endif
  }
}

void startHoming(uint8_t axis)
{
  if (axis >= AXIS_NUM)
    return;

  homed[axis] = false;
  homingActive[axis] = true;
  homingState[axis] = HOME_PREMOVE;

  stepper[axis]->setSpeedInHz(HOME_SPEED);
  stepper[axis]->setAcceleration(HOME_ACCEL);

  // ⭐ ① 预退让：远离限位
  stepper[axis]->move(HOME_PREMOVE_STEPS);

  Serial.printf("Axis %d homing: PREMOVE\n", axis + 1);
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

        // ⭐ 绝对 SEEK 目标（一次性）
        homingSeekTarget[i] =
            motor_mins[i] - (motor_maxs[i] - motor_mins[i] + HOME_BACKOFF_STEPS);

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

        // ⭐ 机械零点
        stepper[i]->setCurrentPosition(motor_mins[i]);

        homingState[i] = HOME_BACKOFF;

        stepper[i]->moveTo(motor_mins[i] + HOME_BACKOFF_STEPS);

        Serial.printf("Axis %d homing: HIT LIMIT\n", i + 1);

        break; // ⭐ 必须立即退出
      }

      // ===== SEEK 走到目标但没命中限位 =====
      if (!stepper[i]->isRunning())
      {
        stepper[i]->forceStop();

        // ⚠️ 不设机械零，仅给逻辑起点
        stepper[i]->setCurrentPosition(motor_mins[i]);

        homingState[i] = HOME_BACKOFF;

        stepper[i]->moveTo(motor_mins[i] + HOME_BACKOFF_STEPS);

        Serial.printf(
            "ERROR: Axis %d homing FAILED (limit not found)\n",
            i + 1);

        break;
        ;
      }
    }
    break;

    // ===== ③ 回退完成 → Homing 完成 =====
    case HOME_BACKOFF:
      if (!stepper[i]->isRunning())
      {
        homingActive[i] = false;
        homed[i] = true;
        homingState[i] = HOME_IDLE;

        // 恢复正常速度
        stepper[i]->setSpeedInHz(SPEED_HZ);
        stepper[i]->setAcceleration(ACCEL);

        // ⭐ 自动到中位
        // stepper[i]->moveTo(motor_center[i]);

        Serial.printf("Axis %d homing: DONE\n", i + 1);
      }
      break;

    default:
      break;
    }
  }
}

uint16_t cmap_uint16(uint32_t x,
                     uint32_t in_min,
                     uint32_t in_max,
                     uint32_t out_min,
                     uint32_t out_max)
{
  uint32_t rv =
      (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

  if (rv > out_max)
    rv = out_max;
  if (rv < out_min)
    rv = out_min;
  return (uint16_t)rv;
}

uint32_t mapToRangeX(uint8_t axis, uint16_t pos)
{
  return cmap_uint16(pos, POS_HOME, POS_MAXI, motor_mins[axis] + RANGE_DZ, motor_maxs[axis] - RANGE_DZ);
}

void testMoveMinus100mm(uint8_t axis)
{
  if (axis >= AXIS_NUM)
    return;
  if (!stepper[axis])
    return;

  // ===== 计算步数 =====
  const float STEPS_PER_MM =
      (MOTOR_STEPS_REV * MICROSTEPS) / LEAD_MM_PER_REV;

  const int32_t move_steps = (int32_t)(100.0f * STEPS_PER_MM); // 100mm

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
    uint32_t dt_ms)
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

  return (int32_t)(dz_mm * STEPS_PER_MM);
}

void applyTargets(uint32_t dt)
{
  for (int i = 0; i < AXIS_NUM; i++)
  {
    if (!homed[i])
      continue;
    int32_t newTarget = targetStep[i];
    int32_t lastTarget = lastTargetStep[i];

    int32_t dz = calcAdaptiveDeadzoneSteps(
        newTarget,
        lastTarget,
        dt);

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
        (int32_t)(AXIS_STROKE_MM[i] * STEPS_PER_MM);

    motor_center[i] =
        (motor_mins[i] + motor_maxs[i]) / 2;
  }
}