#include "Arduino.h"
#include <lvgl.h>
#include "./app/app.h"

// APP app;
#include <mooncake.h>

#include "app/App_Launcher/App_Launcher.h"
#include "app/App_Xlap/App_Xlap.h"
using namespace MOONCAKE;

Mooncake mooncake;
APP app;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("main setup...");
    delay(1000);
    delay(3000);
    mooncake.init();
    app.init();

    // 安装 App (工厂)
    // Install your app by app packer
    // so that mooncake konws how to create and destroy your app

    // App::App_Launcher_Packer *myApp_Launcher_Packer = new App::App_Launcher_Packer;
    // myApp_Launcher_Packer->setFramwork(&mooncake);
    // mooncake.installApp(myApp_Launcher_Packer);

    App::App_Xlap_Packer *xlap_packer = new App::App_Xlap_Packer;
    xlap_packer->setFramwork(&mooncake);
    mooncake.installApp(xlap_packer);

    // 安装 App 后, 可以在已安装列表里获取 App (工厂)
    // After app installed, we can get the packer pointer back from the installed list
    auto my_app_packer = mooncake.getInstalledAppList()[0];

    // 创建 App 实例, 并开始 App
    // Tell mooncake to create an instance of your app, and start it
    mooncake.createAndStartApp(my_app_packer);
}

void loop()
{
    mooncake.update();
    app.loop();
    delay(5);
}
