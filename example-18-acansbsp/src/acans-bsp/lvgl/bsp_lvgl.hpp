
#pragma once
#include <Arduino.h>
#include <lv_conf.h>
#include <lvgl.h>
#include "lgfx.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

// static const uint16_t screenWidth = 320;
// static const uint16_t screenHeight = 240;

// static lv_disp_draw_buf_t draw_buf;
// static lv_color_t buf[2][screenWidth * 10];

// static LGFX _gfx;

// static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
// {
//     if (_gfx.getStartCount() == 0)
//     { // Processing if not yet started
//         _gfx.startWrite();
//     }
//     _gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
//     lv_disp_flush_ready(disp);
// }

// static void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
// {
//     uint16_t touchX, touchY;

//     data->state = LV_INDEV_STATE_REL;

//     if (_gfx.getTouch(&touchX, &touchY))
//     {
//         data->state = LV_INDEV_STATE_PR;

//         /*Set the coordinates*/
//         data->point.x = touchX;
//         data->point.y = touchY;

//         Serial.print("Touch X: ");
//         Serial.print(touchX);
//         Serial.print(", Touch Y: ");
//         Serial.println(touchY);
//     }
// }

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

        // _gfx = gfx;
        // lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);

        // /*Initialize the display*/
        // static lv_disp_drv_t disp_drv;
        // lv_disp_drv_init(&disp_drv);
        // disp_drv.hor_res = screenWidth;
        // disp_drv.ver_res = screenHeight;
        // disp_drv.flush_cb = my_disp_flush;
        // disp_drv.draw_buf = &draw_buf;
        // lv_disp_drv_register(&disp_drv);

        // static lv_indev_drv_t indev_drv;
        // lv_indev_drv_init(&indev_drv);
        // indev_drv.type = LV_INDEV_TYPE_POINTER;
        // indev_drv.read_cb = my_touchpad_read;
        // lv_indev_drv_register(&indev_drv);

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
