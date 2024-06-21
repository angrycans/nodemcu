#pragma once
#include <lvgl.h>
#include "../App_Register.h"
#include "../../acans-bsp/bsp-config.h"
#include "../../acans-bsp/bsp.h"

namespace App
{

    class App_Launcher
    {
    private:
        /* BSP pointer to access hardware functions */
        BSP *_device;

        void updateDeviceStatus();
        void updateAppManage();

        /* UI events */
        lv_timer_t *_time_update_timer;
        static void time_update(lv_timer_t *timer);
        static void scroll_event_cb(lv_event_t *e);
        static void button_event_cb(lv_event_t *e);
        static void panel_control_pad_event_cb(lv_event_t *e);
        static void sleep_mode();

    public:
        inline App_Launcher(BSP *device) { _device = device; }

        void onCreate();
        void onLoop();
        void onDestroy();
    };

}
