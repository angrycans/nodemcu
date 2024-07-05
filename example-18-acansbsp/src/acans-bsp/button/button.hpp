
#pragma once
#include "../bsp-config.h"
#include "Button/Button.h"
#if BSP_MODULE_BUTTON

/* Add your button with this */
#define Button_Add(name, pin) __Button name = __Button(pin)

class Button
{
private:
public:
#if BSP_BTN_A_PIN >= 0
    Button_Add(A, BSP_BTN_A_PIN);
#endif

#if BSP_BTN_B_PIN >= 0
    Button_Add(B, BSP_BTN_B_PIN);
#endif

#if BSP_BTN_C_PIN >= 0
    Button_Add(C, BSP_BTN_C_PIN);
#endif
};

#endif
