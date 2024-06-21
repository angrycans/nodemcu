
#pragma once
#include <Arduino.h>
#include <lv_conf.h>
#include <lvgl.h>
#include "lgfx.h"
#include "lv_port_disp.h"

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * 10];

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
        gfx.setRotation(3);
        //   gfx.setColorDepth(16);
        gfx.setBrightness(127);

        lv_init();

        lv_port_disp_init(&gfx);

        // lv_obj_t *btn = lv_btn_create(lv_scr_act());
        // lv_obj_set_size(btn, 100, 50);
        // lv_obj_center(btn);
        // // lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, NULL);

        // lv_obj_t *label = lv_label_create(btn);
        // lv_label_set_text(label, "Click me!");
        // lv_obj_center(label);

        Serial.println("BspLvgl init ok ");
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
