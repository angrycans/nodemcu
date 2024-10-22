#pragma once
#include <Arduino.h>
#include <LittleFS.h>

class LFS
{

public:
    inline LFS() {}
    inline ~LFS() {}

    inline void init()
    {
        Serial.println("LittleFS start...");
        if (!LittleFS.begin())
        {
            Serial.println("LittleFS mount error");
        }
        else
        {
            Serial.println("LittleFS mount ok");
        }

        // // 创建和写入文件
        // File file = LittleFS.open("/test1.txt", "r");

        // if (!file)
        // {
        //     Serial.println("打开文件失败");
        // }
        // while (file.available())
        // {
        //     Serial.write(file.read());
        // }
        // file.close();
    }
};
