
#pragma once

#include "../acans-bsp/bsp.h"
#include <mooncake.h>
#include "./App_tools.h"

class APP
{
private:
    bool _inited;
    BSP *_device;
    // App::App_Launcher *_launcher;

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
