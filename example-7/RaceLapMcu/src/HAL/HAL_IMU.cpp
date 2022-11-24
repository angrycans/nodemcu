// #include "SoftWire.h"
// #include "Wire.h"
#include "HAL/HAL.h"

#include "I2Cdev.h"
#include "MPU6050.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// SoftWire sw(CONFIG_MCU_SDA, CONFIG_MCU_SCL);

// MPU9250Setting setting;
// static MPU9250_<SoftWire> mpu;
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
void HAL::I2C_Init(bool startScan)
{
    Wire.begin(CONFIG_MCU_SDA, CONFIG_MCU_SCL);

    if (!startScan)
        return;

    uint8_t error, address;
    int nDevices;

    Serial.println("I2C: device scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C: device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println(" !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("I2C: unknow error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    Serial.printf("I2C: %d devices was found\r\n", nDevices);
}

void HAL::IMU_Init()
{

    // HAL::I2C_Init(true);
    delay(1000);
    Wire.begin(CONFIG_MCU_SDA, CONFIG_MCU_SCL);

    // if (!mpu.setup(0x68, setting, sw))
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
    Serial.println("PID tuning Each Dot = 100 readings");
}

void HAL::IMU_Update()
{
    // IMU_Info_t imuInfo;
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // display tab-separated accel/gyro x/y/z values
    // Serial.print("a/g:\t");
    // Serial.print(ax);
    // Serial.print("\t");
    // Serial.print(ay);
    // Serial.print("\t");
    // Serial.print(az);
    // Serial.print("\t");
    // Serial.println(round((float)gx / 32768 * 180));
    // Serial.print("\t");
    //  Serial.print(gy);
    //  Serial.print("\t");
    //  Serial.println(gz);

    // mpu.update();

    // imuInfo.ax = mpu.getAccX();
    // imuInfo.ay = mpu.getAccY();
    // imuInfo.az = mpu.getAccZ();
    // imuInfo.gx = mpu.getGyroX();
    // imuInfo.gy = mpu.getGyroY();
    // imuInfo.gz = mpu.getGyroZ();
    // imuInfo.mx = mpu.getMagX();
    // imuInfo.my = mpu.getMagY();
    // imuInfo.mz = mpu.getMagZ();
    // imuInfo.roll = mpu.getRoll();
    // imuInfo.yaw = mpu.getYaw();
    // imuInfo.pitch = mpu.getPitch();

    // Serial.println(imuInfo.ay);
}