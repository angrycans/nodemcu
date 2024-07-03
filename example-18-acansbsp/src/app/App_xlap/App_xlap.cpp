#if 1
#include "App_Xlap.h"
#include "../../acans-bsp/bsp.h"

static std::string app_name = "Xlap";
static BSP *device;



namespace App
{

    std::string App_Xlap_appName()
    {
        return app_name;
    }

    void *App_Xlap_appIcon()
    {
        return NULL;
    }

    void button_event_cb(lv_event_t *e)
    {
        UI_LOG("[%s] button_event_cb _app.isRunning %d %d\n", App_Xlap_appName().c_str(),&_app,_app.isRunning);
         if (_app.isRunning) {
             UI_LOG("[%s] button_event_cb _app.onDestroy = true\n", App_Xlap_appName().c_str());
                _app.onDestroy = true;
            }
    }

    void App_Xlap_onCreate()
    {
        UI_LOG("[%s] onCreate\n", App_Xlap_appName().c_str());

        lv_obj_t *close_btn = lv_btn_create(lv_scr_act());
        lv_obj_set_size(close_btn, 50, 50);

        lv_obj_add_event_cb(close_btn, button_event_cb, LV_EVENT_CLICKED, NULL);
    }

    void App_Xlap_onLoop()
    {
       // UI_LOG("App_Xlap_onLoop() isrunning %d \n" ,_app.isRunning);
    }

    void App_Xlap_onDestroy()
    {
        UI_LOG("[%s] onDestroy\n", App_Xlap_appName().c_str());
    }

    void App_Xlap_getBsp(void *bsp)
    {
        device = (BSP *)bsp;
    }

}

#endif
