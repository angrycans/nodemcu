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
        if (!LittleFS.begin())
        {
            Serial.println("LittleFS 挂载失败");
        }
        Serial.println("LittleFS 挂载成功");

        // 创建和写入文件
        File file = LittleFS.open("/test1.txt", "r");

        if (!file)
        {
            Serial.println("打开文件失败");
        }
        while (file.available())
        {
            Serial.write(file.read());
        }
        file.close();
    }
};
