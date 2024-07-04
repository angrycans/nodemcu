
#pragma once
#include <Arduino.h>
#include <iostream>
#include <string>
#include "bsp-config.h"
#include "littlefs/lfs.hpp"
#include "sd/sd.hpp"
#include "button/button.hpp"
#include "led/led.hpp"
#include "lvgl/bsp_lvgl.hpp"

/**
 * @brief Acans BSP
 *
 */
class BSP
{
private:
public:
    const std::string Logo = R"(
  ___                      ______  ___________ 
 / _ \                     | ___ \/  ___| ___ \
/ /_\ \ ___ __ _ _ __  ___ | |_/ /\ `--.| |_/ /
|  _  |/ __/ _` | '_ \/ __|| ___ \ `--. \  __/ 
| | | | (_| (_| | | | \__ \| |_/ //\__/ / |    
\_| |_/\___\__,_|_| |_|___/\____/ \____/\_|    
                                               
)";

/* Module Button */
#if BSP_MODULE_BUTTON
    Button btn;
#endif

/* Module RGBLED */
#if BSP_MODULE_LED
    LED led = LED();
#endif

/* Module  */
#if BSP_MODULE_LVGL
    BspLvgl lvgl = BspLvgl();
#endif

    BSPSD sd;

    LFS lfs;

    BSP();

    /**
     * @brief BSP init
     *
     */
    void init();

    /**
     * @brief Print out board infos
     *
     */
    void printBspInfos();
};
