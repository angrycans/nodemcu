
#pragma once
#include <lvgl.h>
#include "../../acans-bsp/bsp.h"
#include <mooncake.h>
#include "race.h"

/**
 * @brief Create an App in name space
 *
 */
namespace App
{

    class App_Xlap : public MOONCAKE::APP_BASE
    {
    private:
        struct Data_t
        {
            std::uint32_t time_count = 0;
        };
        Data_t _data;

        Race *race;

    public:
        App_Xlap();
        void onResume() override;
        void onRunning() override;
        void onDestroy() override;

        static void update_label_cb(lv_timer_t *timer);
        static void button_event_cb(lv_event_t *e);
        static void time_update(lv_timer_t *timer);
    };

    class App_Xlap_Packer : public MOONCAKE::APP_PACKER_BASE
    {
        const char *getAppName() override;
        void *getAppIcon() override;
        void *newApp() override
        {
            App_Xlap *_xlap = new App_Xlap();

            return _xlap;
        }
        void deleteApp(void *app) override { delete (App_Xlap *)app; }
    };

}