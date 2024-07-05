
#include "App_Launcher.h"
#include "UI/ui.h"

extern void ui_init();

namespace App
{

    const char *App_Launcher_Packer::getAppName() { return "App_Launcher"; }
    // Theme color
    constexpr static uint32_t _theme_color = 0x3D7AF5;
    void *App_Launcher_Packer::getCustomData() { return (void *)(&_theme_color); }

    // Icon
    void *App_Launcher_Packer::getAppIcon() { return nullptr; }

    void App_Launcher::update_label_cb(lv_timer_t *timer)
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

    void App_Launcher::onResume()
    {
        // _device->lvgl.init();

        Serial.println("[APP] App_Launcher::onResume()");

        // lv_timer_create(update_label_cb, 100, ui_TimeMs);

        // _device->lvgl.disable();

        // /* Init launcher UI */
        ui_init();
        lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x4D5B74), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_scr_load_anim(ui_ScreenLauncher, LV_SCR_LOAD_ANIM_FADE_IN, 250, 0, true);

        /* Desktop init */
        lv_obj_set_scroll_snap_x(ui_PanelDesktop, LV_SCROLL_SNAP_CENTER);
        lv_obj_update_snap(ui_PanelDesktop, LV_ANIM_ON);

        /* Place App on the desktop panel */
        for (int i = 0; i < static_cast<MOONCAKE::Mooncake *>(getAppPacker()->getFramwork())->getInstalledAppNum(); i++)
        {
            /* Apps' Icon (button) */
            lv_obj_t *app_btn = lv_btn_create(ui_PanelDesktop);
            lv_obj_set_size(app_btn, 150, 150);
            lv_obj_align(app_btn, LV_ALIGN_CENTER, i * 180 + 560, 0);
            lv_obj_add_flag(app_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
            lv_obj_set_style_radius(app_btn, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(app_btn, lv_color_hex(0xA6B4CD), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(app_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_spread(app_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

            /* Setup event call back */
            lv_obj_add_event_cb(app_btn, button_event_cb, LV_EVENT_CLICKED, NULL);

            /* Get App Icon image */
            const lv_img_dsc_t *app_icon = NULL; //(const lv_img_dsc_t *)App::Register[i].appIcon();
            if (app_icon == NULL)
            {
                app_icon = &ui_img_img_icon_defalut_png;
            }
            lv_obj_set_style_bg_img_src(app_btn, app_icon, LV_PART_MAIN | LV_STATE_DEFAULT);

            /* Apps' name */
            lv_obj_t *app_name = lv_label_create(ui_PanelDesktop);
            lv_obj_align(app_name, LV_ALIGN_CENTER, i * 180 + 560, 95);
            std::string str(static_cast<MOONCAKE::Mooncake *>(getAppPacker()->getFramwork())->getInstalledAppList()[i]->getAppName());

            lv_label_set_text(app_name, str.c_str());
            lv_obj_set_style_text_color(app_name, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(app_name, &ui_font_FontUbuntuBold18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        // /* Add ui event call back */
        // lv_obj_add_event_cb(ui_PanelDesktop, scroll_event_cb, LV_EVENT_SCROLL_END, NULL);
        // lv_obj_add_event_cb(ui_ArcBrightness, panel_control_pad_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
        // lv_obj_add_event_cb(ui_ButtonAutoScreenOff, panel_control_pad_event_cb, LV_EVENT_CLICKED, NULL);
        // lv_obj_add_event_cb(ui_ButtonInfos, panel_control_pad_event_cb, LV_EVENT_CLICKED, NULL);
        // lv_obj_add_event_cb(ui_ButtonWifi, panel_control_pad_event_cb, LV_EVENT_CLICKED, NULL);
        // lv_obj_add_event_cb(ui_ButtonBle, panel_control_pad_event_cb, LV_EVENT_CLICKED, NULL);

        /* Go to the previous place */
        // if (_app.selected != -1)
        // {
        //     /* Pull down state bar manully */
        //     lv_obj_set_y(ui_ImgStateBar, -105);
        //     lv_obj_scroll_to_view(lv_obj_get_child(ui_PanelDesktop, (_app.selected * 2 + 2)), LV_ANIM_OFF);
        // }
        // else
        {
            lv_obj_scroll_to_view(lv_obj_get_child(ui_PanelDesktop, 1), LV_ANIM_OFF);
        }

        /* Create a timer to update time */
        // _time_update_timer = lv_timer_create(time_update, 1000, (void *)_device);
        // time_update(_time_update_timer);

        /* Reset auto screen off time counting */
        lv_disp_trig_activity(NULL);

        // /* Update control panel */
        // if (_device_status.autoScreenOff)
        // {
        //     lv_obj_add_state(ui_ButtonAutoScreenOff, (LV_STATE_CHECKED | LV_STATE_FOCUSED));
        // }

        // _device->lvgl.enable();
    }

    void App_Launcher::onRunning()
    {

        // _device->lvgl.disable();

        // updateAppManage();

        // _device->lvgl.enable();
    }

    void App_Launcher::onDestroy()
    {
        /* Delete timer */
        //   lv_timer_del(_time_update_timer);

        /* Delete Launcher screen and reaplace with an empty one */
        lv_disp_load_scr(lv_obj_create(NULL));
        lv_obj_del(ui_ScreenLauncher);
    }

    void App_Launcher::button_event_cb(lv_event_t *e)
    {
        lv_obj_t *obj = lv_event_get_target(e);
        lv_event_code_t code = lv_event_get_code(e);

        /* Into that App */
        if (code == LV_EVENT_CLICKED)
        {
            int idx = (lv_obj_get_index(obj) - 2) / 2;
            // MOONCAKE::APP_BASE *app = static_cast<MOONCAKE::Mooncake *>(getAppPacker()->getFramwork())->getInstalledAppList()[idx]->getAddr();
            // static_cast<MOONCAKE::Mooncake *>(getAppPacker()->getFramwork())->startApp();
            // _app.onCreate = true;
        }
    }

}