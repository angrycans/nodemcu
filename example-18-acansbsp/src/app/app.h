
#pragma once

#include "../acans-bsp/bsp.h"
#include <mooncake.h>

class APP
{
private:
    bool _inited;
    BSP *_device;

public:
    APP() : _inited(false) {}
    ~APP() {}
    int init();
    void loop();
    inline BSP *device() { return _device; }
};
