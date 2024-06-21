
#pragma once
#include "../App_Register.h"

/**
 * @brief Create an App in name space
 *
 */
namespace App
{

    std::string App_xlap_appName();
    void *App_xlap_appIcon();
    void App_xlap_onCreate();
    void App_xlap_onLoop();
    void App_xlap_onDestroy();
    void App_xlap_getBsp(void *bsp);

    /**
     * @brief Declace your App like this
     *
     */
    App_Declare(xlap);
}
