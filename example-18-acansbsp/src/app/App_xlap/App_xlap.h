#if 1
#pragma once
#include "../App_Typedef.h"
#include "../App_Tools.h"
/**
 * @brief Create an App in name space
 *
 */
namespace App
{

    std::string App_Xlap_appName();
    void *App_Xlap_appIcon();
    void App_Xlap_onCreate();
    void App_Xlap_onLoop();
    void App_Xlap_onDestroy();
    void App_Xlap_getBsp(void *bsp);

    /**
     * @brief Declace your App like this
     *
     */
    App_Declare(Xlap);
}

#endif
