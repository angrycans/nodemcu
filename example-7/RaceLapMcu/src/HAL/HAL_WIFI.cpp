
#include "HAL/HAL.h"
#include "../webserver_helper.hpp"

void HAL::WIFI_Init()
{
    logger.LogInfo("init Wifi");
    // delay(250);
    const char *ssid = "RaceLap";      // Enter SSID here
    const char *password = "88888888"; // Enter Password here

    logger.LogInfo("WiFi.softAP start");

    WiFi.softAP(ssid, password);
    // delay(250);
    logger.LogInfo("Soft-AP IP address = ");
    logger.LogInfo(WiFi.softAPIP().toString().c_str());
    // delay(250);

    // Serial.print("DHCP status:");
    // Serial.println(wifi_softap_dhcps_status());

    // IPAddress local_ip(192, 168, 4, 1);
    // IPAddress gateway(192, 168, 4, 1);
    // IPAddress subnet(255, 255, 255, 0);

    // WiFi.softAPConfig(local_ip, gateway, subnet);
    // delay(50);
    // WiFi.begin("wifi-acans", "85750218");

    // WiFi.begin("yunweizu", "yunweizubangbangda");
    // //   WiFi.begin("qianmi-mobile", "qianmi123");

    // // WiFi.begin("wifi-acans", "85750218");
    // logger.LogInfo("Connecting");

    // while (WiFi.status() != WL_CONNECTED)
    // {
    //   delay(500);

    //   logger.LogInfo(".");
    // }

    // logger.LogInfo("Connected, IP address: ");
    // logger.LogInfo(WiFi.localIP().toString().c_str());

    initWebServer();
}
