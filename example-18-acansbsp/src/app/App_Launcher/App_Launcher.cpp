
#include "App_Launcher.h"
#include "UI/ui.h"

extern void ui_init();

/* Structure to hold device status */
struct DeviceStatus_t
{
    bool updated = false;
    bool autoScreenOff = false;
    uint8_t brightness = 127;
    uint32_t autoScreenOffTime = 20000;
};
static DeviceStatus_t _device_status;

AppManager_t _app;

namespace App
{
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

    void App_Launcher::onCreate()
    {
        // _device->lvgl.init();

        Serial.println("[APP] App_Launcher::onCreate()");

        // lv_timer_create(update_label_cb, 100, ui_TimeMs);

        _device->lvgl.disable();

        // /* Init launcher UI */
        ui_init();
        lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x4D5B74), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_scr_load_anim(ui_ScreenLauncher, LV_SCR_LOAD_ANIM_FADE_IN, 250, 0, true);

        /* Desktop init */
        lv_obj_set_scroll_snap_x(ui_PanelDesktop, LV_SCROLL_SNAP_CENTER);
        lv_obj_update_snap(ui_PanelDesktop, LV_ANIM_ON);

        /* Read App register */
        _app.totalNum = sizeof(App::Register) / sizeof(App::AppRegister_t);

        UI_LOG("[APP] appnum=%d", _app.totalNum);

        /* Place App on the desktop panel */
        for (int i = 0; i < _app.totalNum; i++)
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
            const lv_img_dsc_t *app_icon = (const lv_img_dsc_t *)App::Register[i].appIcon();
            if (app_icon == NULL)
            {
                app_icon = &ui_img_img_icon_defalut_png;
            }
            lv_obj_set_style_bg_img_src(app_btn, app_icon, LV_PART_MAIN | LV_STATE_DEFAULT);

            /* Apps' name */
            lv_obj_t *app_name = lv_label_create(ui_PanelDesktop);
            lv_obj_align(app_name, LV_ALIGN_CENTER, i * 180 + 560, 95);
            lv_label_set_text(app_name, App::Register[i].appName().c_str());
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
        if (_app.selected != -1)
        {
            /* Pull down state bar manully */
            lv_obj_set_y(ui_ImgStateBar, -105);
            lv_obj_scroll_to_view(lv_obj_get_child(ui_PanelDesktop, (_app.selected * 2 + 2)), LV_ANIM_OFF);
        }
        else
        {
            lv_obj_scroll_to_view(lv_obj_get_child(ui_PanelDesktop, 1), LV_ANIM_OFF);
        }

        /* Create a timer to update time */
        // _time_update_timer = lv_timer_create(time_update, 1000, (void *)_device);
        // time_update(_time_update_timer);

        /* Reset auto screen off time counting */
        lv_disp_trig_activity(NULL);

        /* Update control panel */
        if (_device_status.autoScreenOff)
        {
            lv_obj_add_state(ui_ButtonAutoScreenOff, (LV_STATE_CHECKED | LV_STATE_FOCUSED));
        }

        _device->lvgl.enable();
    }

    void App_Launcher::onLoop()
    {

        _device->lvgl.disable();

        updateDeviceStatus();

        updateAppManage();

        _device->lvgl.enable();
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
            _app.selected = (lv_obj_get_index(obj) - 2) / 2;
            _app.onCreate = true;
        }
    }

    void App_Launcher::updateAppManage()
    {
        // UI_LOG("updateAppManage");
        /* App state manage, kind of like a FSM */
        if (_app.onCreate)
        {
            _app.onCreate = false;
            _app.isRunning = true;

            /* Destroy launcher */
            onDestroy();
            /* Open App */
            App::Register[_app.selected].getBsp(_device);
            App::Register[_app.selected].onCreate();

            UI_LOG("app isrunning %d %d \n", &_app, _app.isRunning);
        }
        if (_app.isRunning)
        {
            App::Register[_app.selected].onLoop();
        }
        if (_app.onDestroy)
        {

            UI_LOG("_app.onDestroy");
            _app.onDestroy = false;
            _app.isRunning = false;

            /* Quit App */
            App::Register[_app.selected].onDestroy();
            /* Create launcher */
            onCreate();
        }
    }

    void App_Launcher::updateDeviceStatus()
    {
    }

    void App_Launcher::sleep_mode()
    {
    }
}