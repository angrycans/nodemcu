//调用库来自：https://github.com/bogde/HX711
#include <HX711.h>
//调用库来自：https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Joystick.h>

/*
定义称重传感器（刹车）相关参数
calibration_factor 称重传感器标定系数
DOUT 数据引脚
CLK 时钟引脚
SCALE_MIN HX711最小输入量
SCALE_MAX HX711最大输入量
B_MIN 刹车踏板下限值
B_MAX 刹车踏板上限值
*/
#define calibration_factor 100
#define DOUT  1
#define CLK  0
#define SCALE_MIN 100
#define SCALE_MAX 12000
#define B_MIN 0
#define B_MAX 4096

/*
定义称重传感器（手刹）相关参数
calibration_factor 称重传感器标定系数
DOUT 数据引脚
CLK 时钟引脚
SCALE_MIN HX711最小输入量
SCALE_MAX HX711最大输入量
B_MIN 刹车踏板下限值
B_MAX 刹车踏板上限值
*/
#define calibration_factor2 100
#define DOUT2  6
#define CLK2  7
#define SCALE_MIN2 0
#define SCALE_MAX2 10000
#define H_MIN 0
#define H_MAX 4096

/*
定义油门（Throttle）参数
T_PIN 模拟电压量输入引脚
T_INPUT_MIN 油门传感器最小模拟量（完全释放时的原始值）
T_INPUT_MAX 油门传感器最大模拟量（完全踩下时的原始值）
T_MIN 油门踏板下限值
T_MAX 油门踏板上限值
*/
#define T_PIN A3
#define T_INPUT_MIN 205
#define T_INPUT_MAX 470
#define T_MIN 0
#define T_MAX 255

/*
定义离合（XAxis）参数
C_PIN 模拟电压量输入引脚
C_INPUT_MIN 离合传感器最小模拟量（完全释放时的原始值）
C_INPUT_MAX 离合传感器最大模拟量（完全踩下时的原始值）
C_MIN 离合踏板下限值
C_MAX 离合踏板上限值
*/
#define C_PIN A2
#define C_INPUT_MIN 200
#define C_INPUT_MAX 460
#define C_MIN 0
#define C_MAX 255

//创建Loadcell类，以阻塞模式启动
HX711 Loadcell;
HX711 Loadcell2;

//创建Joystick类，定义外设类型和各轴启用状态
Joystick_ Joystick(0x04, JOYSTICK_TYPE_JOYSTICK, //HID设备ID设置为0x04 | 外设类型设置为Joystick（摇杆）
                   2, 0,                   // 0个按钮 | 0个切换开关
                   true, false, false,     // X轴扳机启用 | Y轴扳机禁用 | Z轴扳机禁用
                   false, false, false,    // X轴旋转禁用 | Y轴旋转禁用 | Z轴旋转禁用
                   false, true,            // 脚舵禁用 | 油门启用
                   true, true, false);    // 加速器禁用 | 刹车启用 | 方向盘禁用

//声明各踏板所需的变量
int Brake_ = 0;
int handBrake_ = 0;
int Throttle_ = 0;
int Clutch_ = 0;

int BtnA = 0;
int BtnB = 0;

void setup() {
    delay(5000);
    Serial.begin(115200);

    Serial.println("arduino micro setup");
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    //初始化Joystick库
    Joystick.begin(false);



    //设置各轴区间
    Joystick.setThrottleRange(T_MIN, T_MAX);
    Joystick.setBrakeRange(B_MIN, B_MAX);
    Joystick.setXAxisRange(C_MIN, C_MAX);
    Joystick.setAcceleratorRange(H_MIN, H_MAX);

    //设置HX711使用的引脚
    Loadcell.begin(DOUT, CLK);

    //载入称重传感器标定数据
    Loadcell.set_scale(calibration_factor);
    Loadcell.tare();

    //设置HX711使用的引脚
    Loadcell2.begin(DOUT2, CLK2);
    //载入称重传感器标定数据
    Loadcell2.set_scale(calibration_factor2);
    Loadcell2.tare();
}

void loop() {
     
    int currBtnStateA = digitalRead(A0);
    Serial.println("BtnA old=" + String(BtnA) + " new=" + String(currBtnStateA));
    if (BtnA != currBtnStateA)
    {
        Joystick.setButton(0, currBtnStateA);
        BtnA = currBtnStateA;
    }
    int currBtnStateB = digitalRead(A1);
    Serial.println("BtnB lod=" + String(BtnB) + " new=" + String(currBtnStateB));
    if (BtnB != currBtnStateB)
    {
        Joystick.setButton(1, currBtnStateB);
        BtnB = currBtnStateB;
    }

    //读取称重传感器数据作为刹车值，若原始变量为负值，则需要使用负号： -Loadcell.get
    Brake_ = Loadcell.get_units();
   

    Brake_ = map(Brake_, SCALE_MIN, SCALE_MAX, B_MIN, B_MAX); //值映射
    Joystick.setBrake(Brake_);

    handBrake_ = Loadcell2.get_units();
    handBrake_ = map(handBrake_, SCALE_MIN2, SCALE_MAX2, B_MIN, B_MAX); //值映射
    Joystick.setAccelerator(handBrake_);

    //将离合值传入离合摇杆（XAxis）
    Clutch_ = analogRead(C_PIN);
     // Serial.println(Clutch_);
      //  Serial.println("---------");
    Clutch_ = map(Clutch_, C_INPUT_MIN, C_INPUT_MAX, C_MIN, C_MAX); //值映射
    //Serial.println(Clutch_);
    Clutch_ = C_MAX - Clutch_; //霍尔电压值反转，视磁铁布置方式而定
    Joystick.setXAxis(Clutch_);

    //将油门值传入油门摇杆（Throttle）
    Throttle_ = analogRead(T_PIN);
    Throttle_ = map(Throttle_, T_INPUT_MIN, T_INPUT_MAX, T_MIN, T_MAX); //值映射
    Throttle_ = T_MAX - Throttle_; //霍尔电压值反转，视磁铁布置方式而定
    Joystick.setThrottle(Throttle_);




    //更新状态，设置循环延迟10ms
    Joystick.sendState();
    delay(10);
}