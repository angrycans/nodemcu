
#include "app.h"

int APP::init()
{

    if (_inited)
    {
        spdlog::info("[APP] already inited\n");
        return -1;
    }

    /* Create bsp */
    _device = new BSP();
    spdlog::info("[APP]  init...\n");
    // _launcher = new App::App_Launcher(_device);
    if (_device == NULL)
    {
        spdlog::info("[APP] Launcher create failed\n");
        return -1;
    }

    _inited = true;

    /* Start launcher */
    //   _launcher->onCreate();

    return 0;
}

void APP::loop()
{
    _device->loop();
    // _launcher->onLoop();
}
