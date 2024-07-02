
#pragma once
#include <Arduino.h>
#include <lv_conf.h>
#include <lvgl.h>
#include "lgfx.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_fs.h"

class BspLvgl
{
private:
    LGFX gfx;
    bool _enable;

public:
    /**
     * @brief Init lvgl and create a task to handle lv timer
     *
     * @param lcd
     * @param tp
     */
    void init()
    {
        gfx.begin();
        gfx.setRotation(1);
        //   gfx.setColorDepth(16);
        gfx.setBrightness(127);

        lv_init();

        lv_port_disp_init(&gfx);
        lv_port_indev_init(&gfx);
        lv_fs_fatfs_init();

        Serial.println("Bsp Lvgl init ok ");
    }

    /**
     * @brief Enable lvgl refreshing
     *
     * @param xTicksToWait
     */
    inline void enable()
    {
        if (_enable)
            return;
        _enable = true;
    }

    inline void disable()
    {
        if (!_enable)
            return;

        _enable = false;
    }

    inline bool isEnable()
    {

        return _enable;
    }
};
