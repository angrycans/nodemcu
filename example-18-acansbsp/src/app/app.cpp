
#include "app.h"

int APP::init()
{

    if (_inited)
    {
        UI_LOG("[APP] already inited\n");
        return -1;
    }

    /* Create bsp */
    _device = new BSP();
    UI_LOG("[APP]  init...\n");
    _launcher = new App::App_Launcher(_device);
    if (_device == NULL)
    {
        UI_LOG("[APP] Launcher create failed\n");
        return -1;
    }

    _inited = true;

    /* Start launcher */
    _launcher->onCreate();

    return 0;
}

void APP::loop()
{
    _launcher->onLoop();
}
