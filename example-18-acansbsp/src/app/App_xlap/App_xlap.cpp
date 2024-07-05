#if 1
#include "App_Xlap.h"
#include "UI/ui.h"

extern void ui_init();

namespace App
{

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

    void App_Xlap::onResume()
    {
        // _device->lvgl.init();

        Serial.println("[APP] App_Xlap::onResume()");

        xlap_ui_init();
        lv_obj_t *close_btn = lv_btn_create(lv_scr_act());
        lv_obj_set_size(close_btn, 36, 36);

        lv_obj_add_event_cb(close_btn, button_event_cb, LV_EVENT_CLICKED, NULL);
    }

    void App_Xlap::onRunning()
    {

        // _device->lvgl.disable();

        // updateAppManage();

        // _device->lvgl.enable();
    }

    void App_Xlap::onDestroy()
    {
    }

    void App_Xlap::button_event_cb(lv_event_t *e)
    {
    }

}


#endif
