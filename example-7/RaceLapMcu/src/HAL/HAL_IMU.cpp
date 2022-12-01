// #include "SoftWire.h"
// #include "Wire.h"
#include "HAL/HAL.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// SoftWire sw(CONFIG_MCU_SDA, CONFIG_MCU_SCL);

// MPU9250Setting setting;
// static MPU9250_<SoftWire> mpu;
GY521 mpu(0x68);

uint32_t counter = 0;

float ax, ay, az;
float gx, gy, gz;
float t;

void HAL::I2C_Init(bool startScan)
{
    Wire.begin(CONFIG_MCU_SDA, CONFIG_MCU_SCL);

    if (!startScan)
        return;

    uint8_t error, address;
    int nDevices;

    logger.LogInfo("I2C: device scanning...");

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

    logger.LogInfo("I2C:  devices was found\r\n");
}

void HAL::IMU_Init()
{

    // HAL::I2C_Init(true);
    // delay(1000);
    Wire.begin(CONFIG_MCU_SDA, CONFIG_MCU_SCL);

    delay(100);
    while (mpu.wakeup() == false)
    {
        Serial.print(millis());
        Serial.println("\tCould not connect to GY521");
        delay(1000);
    }
    mpu.setAccelSensitivity(0); // 8g
    mpu.setGyroSensitivity(1);  // 500 degrees/s

    // mpu.setThrottle();

    mpu.axe = 0;
    mpu.aye = 0;
    mpu.aze = 0;
    mpu.gxe = 0;
    mpu.gye = 0;
    mpu.gze = 0;

    Serial.println("mpu start...");
}

void HAL::IMU_Calibration()
{
    ax = ay = az = 0;
    gx = gy = gz = 0;
    t = 0;
    for (int i = 0; i < 20; i++)
    {
        mpu.read();
        ax -= mpu.getAccelX();
        ay -= mpu.getAccelY();
        az -= mpu.getAccelZ();
        gx -= mpu.getGyroX();
        gy -= mpu.getGyroY();
        gz -= mpu.getGyroZ();
        t += mpu.getTemperature();
    }

    if (counter % 10 == 0)
    {
        Serial.println("\n\tACCELEROMETER\t\tGYROSCOPE\t\tTEMPERATURE");
        Serial.print('\t');
        Serial.print(mpu.axe, 3);
        Serial.print('\t');
        Serial.print(mpu.aye, 3);
        Serial.print('\t');
        Serial.print(mpu.aze, 3);
        Serial.print('\t');
        Serial.print(mpu.gxe, 3);
        Serial.print('\t');
        Serial.print(mpu.gye, 3);
        Serial.print('\t');
        Serial.print(mpu.gze, 3);
        Serial.print('\n');
        Serial.println("\taxe\taye\taze\tgxe\tgye\tgze\tT");
    }

    Serial.print(counter);
    Serial.print('\t');
    Serial.print(ax * 0.05, 3);
    Serial.print('\t');
    Serial.print(ay * 0.05, 3);
    Serial.print('\t');
    Serial.print(az * 0.05, 3);
    Serial.print('\t');
    Serial.print(gx * 0.05, 3);
    Serial.print('\t');
    Serial.print(gy * 0.05, 3);
    Serial.print('\t');
    Serial.print(gz * 0.05, 3);
    Serial.print('\t');
    Serial.print(t * 0.05, 2);
    Serial.println();

    // adjust calibration errors so table should get all zero's.
    mpu.axe += ax * 0.05;
    mpu.aye += ay * 0.05;
    mpu.aze += az * 0.05;
    mpu.gxe += gx * 0.05;
    mpu.gye += gy * 0.05;
    mpu.gze += gz * 0.05;

    counter++;
    delay(100);
}

void HAL::IMU_Update()
{

    mpu.read();
    // IMU_Info_t imuInfo;
    // accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

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
