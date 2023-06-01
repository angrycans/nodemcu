#include <Arduino.h>
#include "AXP173.h"
#include "log.h"

#include "Wire.h"

#define LED_BUILTIN 5
AXP173 pmu;
I2C_PORT i2c;

#define CONFIG_SDA 8
#define CONFIG_SCL 5

void setPmuPower()
{ //电源通道电压输出设置，交换位置可以设置上电时序，中间加delay可以延迟上电
  /* Enable and set LDO2 voltage */
  pmu.setOutputEnable(AXP173::OP_LDO2, true);  // LDO2设置为输出
  pmu.setOutputVoltage(AXP173::OP_LDO2, 3000); // LDO2电压设置为3.000V

  /* Enable and set LDO3 voltage */
  pmu.setOutputEnable(AXP173::OP_LDO3, true);  // LDO3设置为输出
  pmu.setOutputVoltage(AXP173::OP_LDO3, 3300); // LDO3电压设置为3.300V

  /* Enable and set LDO4 voltage */
  pmu.setOutputEnable(AXP173::OP_LDO4, true);  // LDO4设置为输出
  pmu.setOutputVoltage(AXP173::OP_LDO4, 3300); // LDO4电压设置为3.300V

  /* Enable and set DCDC1 voltage */
  pmu.setOutputEnable(AXP173::OP_DCDC1, true);  // DCDC1设置为输出
  pmu.setOutputVoltage(AXP173::OP_DCDC1, 3300); // DCDC1电压设置为3.300V

  /* Enable and set DCDC2 voltage */
  pmu.setOutputEnable(AXP173::OP_DCDC2, true);  // DCDC2设置为输出
  pmu.setOutputVoltage(AXP173::OP_DCDC2, 2275); // DCDC2电压设置为2.275V

  /* Enable Battery Charging */
  pmu.setChargeEnable(true);               //充电功能使能
  pmu.setChargeCurrent(AXP173::CHG_360mA); //设置充电电流为450mA
}

void setPmuConfig()
{ //电源芯片ADC，库仑计等功能设置
  /* Set off time */
  pmu.setPowerOffTime(AXP173::POWEROFF_4S); //设置关机后所有电源通道关闭时长为4S

  /* Set on time */
  pmu.setPowerOnTime(AXP173::POWERON_128mS); //设置开机后电源通道启动输出时长为128mS

  /* Enable VBUS ADC */
  pmu.setADCEnable(AXP173::ADC_VBUS_V, true); // VBUS ADC 电压使能
  pmu.setADCEnable(AXP173::ADC_VBUS_C, true); // VBUS ADC 电流使能

  /* Enable Battery ADC */
  pmu.setADCEnable(AXP173::ADC_BAT_V, true); // Battery ADC 电压使能
  pmu.setADCEnable(AXP173::ADC_BAT_C, true); // Battery ADC 电流使能

  /* Enable Coulometer and set COULOMETER_ENABLE*/
  pmu.setCoulometer(AXP173::COULOMETER_ENABLE, true); //库仑计使能

  Serial.println("AXP173 Set OK");
}

void printPmuInfo()
{ //需要打印在屏幕上的芯片信息

  /* Get PMU temp info */
  lprintf(LOG_INFO, "CoreTemp :%.2f 'C\n", pmu.getAXP173Temp()); //芯片温度

  /* Get VBUS info */
  // lprintf(LOG_INFO,"VBUS_voltage :%.2f V\n", pmu.getVBUSVoltage());         //VBUS输入电压
  // lprintf(LOG_INFO,"VBUS_current :%.2f mA\n", pmu.getVBUSCurrent());        //VBUS输入电流

  if (pmu.isBatExist())
  {
    // lprintf(LOG_INFO,"Battery :Battery Exist\n");                         //电池接入状态
    /* Get Battery info */
    pmu.isCharging() ? lprintf(LOG_INFO, "Charging :Is Charging s\n") : lprintf(LOG_INFO, "Charging :NO or END Charging\n"); //充电状态
    lprintf(LOG_INFO, "Bat_voltage :%.2f V\n", pmu.getBatVoltage());                                                         //电池电压
    lprintf(LOG_INFO, "Bat_Current :%.2f mA\n", pmu.getBatCurrent());                                                        //电池电流  正为充电，负为放电

    // lprintf(LOG_INFO,"Bat_Level :%.2f %%\n", pmu.getBatLevel());            //电池电量百分比显示
    // lprintf(LOG_INFO,"Bat_BatPower :%.2f W", pmu.getBatPower());            //电池瞬时功率

    lprintf(LOG_INFO, "GetBatCoulombInput :%.2f C\n", pmu.GetBatCoulombInput());   // Get Coulomb charge Data
    lprintf(LOG_INFO, "GetBatCoulombOutput :%.2f C\n", pmu.GetBatCoulombOutput()); // Get Coulomb Discharge Data
    lprintf(LOG_INFO, "CoulometerData :%.2f C\n", pmu.getCoulometerData());        // get coulomb val affter calculation
  }
  else
  {
    lprintf(LOG_INFO, "Battery :NO Battery\n"); //没电池就输出这
  }
}
void setup()
{

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200); // 设置波特率为 115200
  Serial.println("esp32-c3 setup....");

  Wire.begin(CONFIG_SDA, CONFIG_SCL);

  pmu.begin(&Wire);
  setPmuPower();

  i2c.I2C_dev_scan();

  setPmuConfig();

  delay(3000);
  printPmuInfo();
}

void loop()
{

  digitalWrite(LED_BUILTIN, (millis() / 1000) % 2);
}
