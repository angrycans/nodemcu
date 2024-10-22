#if 1
#include "App_Xlap.h"
#include "UI/ui.h"
#include "../App_Tools.hpp"

namespace App
{

    App_Xlap::App_Xlap()
    {

#if BSP_MODULE_GPS

        race = new Race();

#endif
    }

    const char *App_Xlap_Packer::getAppName() { return "App_Xlap"; }

    // Icon
    void *App_Xlap_Packer::getAppIcon() { return nullptr; }

    void App_Xlap::update_label_cb(lv_timer_t *timer)
    {
        // 获取标签对象
        lv_obj_t *label = (lv_obj_t *)timer->user_data;

        // 获取当前的毫秒时间
        uint32_t ms_time = lv_tick_get();

        // 创建一个字符串来存储时间
        char buf[16];
        snprintf(buf, sizeof(buf), "%u ms", ms_time);

        // 更新标签的文本
        lv_label_set_text(label, buf);
    }

    void App_Xlap::time_update(lv_timer_t *timer)
    {

        // 更新 LVGL 对象的值
        lv_label_set_text_fmt(ui_TimeMinSec, "%s", formatTimeMs(millis()));
    }

    void App_Xlap::onResume()
    {
        // _device->lvgl.init();

        Serial.println("[APP] App_Xlap::onResume()");

        xlap_ui_init();
        lv_obj_t *close_btn = lv_btn_create(lv_scr_act());
        lv_obj_set_size(close_btn, 36, 36);

        lv_obj_add_event_cb(close_btn, button_event_cb, LV_EVENT_CLICKED, NULL);

        lv_timer_t *_time_update_timer = lv_timer_create(time_update, 100, this);

        time_update(_time_update_timer);
    }

    void App_Xlap::onRunning()
    {
    }

    void App_Xlap::onDestroy()
    {

        delete race;
    }

    void App_Xlap::button_event_cb(lv_event_t *e)
    {
        BSP &bsp = BSP::getInstance();
        bsp.gps.setRunning(!bsp.gps.getRunning());
    }

}

#endif
