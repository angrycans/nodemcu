#pragma once
#include <lvgl.h>
#include "../../acans-bsp/bsp.h"
#include <mooncake.h>

namespace App
{

    class App_Launcher : public MOONCAKE::APP_BASE
    {
    private:
        struct Data_t
        {
            std::uint32_t time_count = 0;
        };
        Data_t _data;

        BSP *_device;

    public:
        void onResume() override;
        void onRunning() override;
        void onDestroy() override;

        inline void setBsp(BSP *device) { _device = device; }

        static void update_label_cb(lv_timer_t *timer);
        static void button_event_cb(lv_event_t *e);
    };

    class App_Launcher_Packer : public MOONCAKE::APP_PACKER_BASE
    {
        const char *getAppName() override;
        void *getAppIcon() override;
        void *newApp() override { return new App_Launcher; }
        void deleteApp(void *app) override { delete (App_Launcher *)app; }
        void *getCustomData() override;
    };

}