
#pragma once

#include "../acans-bsp/bsp.h"
#include "App_Launcher/App_Launcher.h"

#define UI_LOG(format, args...) printf(format, ##args)

class APP
{
private:
    bool _inited;
    BSP *_device;
    App::App_Launcher *_launcher;

public:
    APP() : _inited(false) {}
    ~APP() {}

    /**
     * @brief UI begin
     *
     */
    int init();

    /**
     * @brief Put it into loop
     *
     */
    void loop();

    /* Get device bsp pointor */
    inline BSP *device() { return _device; }
};
