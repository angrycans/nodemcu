#include <Arduino.h>
#include <Wire.h>
#include <lv_conf.h>
#include <lvgl.h>
#include "gui.h"
#include <OneButton.h>

#include "../ui/ui.h"
#include "../gfx/gfx.h"

static const char *TAG = "gui";

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * 10];

LGFX gfx;

#define LV_KEY_PREV_PIN 4
#define LV_KEY_NEXT_PIN 5
#define LV_KEY_ENTER_PIN 6

OneButton btn_next = OneButton(
    LV_KEY_PREV_PIN, // Input pin for the button
    true,            // Button is active LOW
    true             // Enable internal pull-up resistor
);

OneButton btn_prev = OneButton(
    LV_KEY_NEXT_PIN, // Input pin for the button
    true,            // Button is active LOW
    true             // Enable internal pull-up resistor
);

OneButton btn_enter = OneButton(
    LV_KEY_ENTER_PIN, // Input pin for the button
    true,             // Button is active LOW
    true              // Enable internal pull-up resistor
);

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  if (gfx.getStartCount() == 0)
  { // Processing if not yet started
    gfx.startWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  data->state = LV_INDEV_STATE_REL;

  if (gfx.getTouch(&touchX, &touchY))
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

uint32_t get_key()
{
  if (digitalRead(LV_KEY_PREV_PIN) == 0)
  {
    Serial.println("LV_KEY_PREV_PIN press");
    return 1;

    // data->enc_diff = LV_KEY_PREV;
  }
  else if (digitalRead(LV_KEY_NEXT_PIN) == 0)
  {

    Serial.println("LV_KEY_NEXT press");
    return 2;
  }
  else if (digitalRead(LV_KEY_ENTER_PIN) == 0)
  {
    Serial.println("LV_KEY_ENTER press");

    return 3;
  }
}

void keyboard_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  static uint32_t last_key = 0;
  uint32_t act_key = get_key();
  if (act_key != 0)
  {
    data->state = LV_INDEV_STATE_PR;

    switch (act_key)
    {
    case 1:
      act_key = LV_KEY_PREV;
      break;
    case 2:
      act_key = LV_KEY_NEXT;
      break;
    case 3:
      act_key = LV_KEY_ENTER;
      break;
    }

    last_key = act_key;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }

  data->key = last_key;
}

void my_keyboard_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (digitalRead(LV_KEY_PREV_PIN) == 0)
  {
    Serial.println("LV_KEY_PREV_PIN press");
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = LV_KEY_PREV;

    // data->enc_diff = LV_KEY_PREV;
  }
  else if (digitalRead(LV_KEY_NEXT_PIN) == 0)
  {

    Serial.println("LV_KEY_NEXT press");
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = LV_KEY_NEXT;
    // data->enc_diff = LV_KEY_NEXT;
  }
  else if (digitalRead(LV_KEY_ENTER_PIN) == 1)
  {
    Serial.println("LV_KEY_ENTER press");
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = LV_KEY_ENTER;
    // data->enc_diff = LV_KEY_ENTER;
  }
}

static void event_cb(lv_event_t *e)
{
  LV_LOG_USER("Clicked");

  static uint32_t cnt = 1;
  lv_obj_t *btn = lv_event_get_target(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  lv_label_set_text_fmt(label, "%" LV_PRIu32, cnt);
  cnt++;
}

/**
 * @brief
 *
 */
void gui_start()
{

  pinMode(LV_KEY_PREV_PIN, INPUT);
  pinMode(LV_KEY_NEXT_PIN, INPUT);
  pinMode(LV_KEY_ENTER_PIN, INPUT);

  // ----------- GFX -------------
  gfx.begin();
  gfx.setRotation(3);
  //   gfx.setColorDepth(16);
  gfx.setBrightness(127);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // /*Initialize the input device driver*/
  // static lv_indev_drv_t indev_drv;
  // lv_indev_drv_init(&indev_drv);
  // indev_drv.type = LV_INDEV_TYPE_POINTER;
  // indev_drv.read_cb = my_touchpad_read;
  // lv_indev_drv_register(&indev_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_KEYPAD;
  // indev_drv.read_cb = my_keyboard_read;
  indev_drv.read_cb = keyboard_read;

  lv_indev_t *key_indev_drv = lv_indev_drv_register(&indev_drv);

  lv_group_t *g = lv_group_create();
  lv_group_set_default(g);
  lv_indev_set_group(key_indev_drv, g);

  btn_next.attachClick([]()
                       {
                         Serial.println("btn_next Pressed!");
                         // lv_event_send(lv_scr_act(), LV_EVENT_CLICKED, NULL);
                       });

  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn, 100, 50);
  lv_obj_center(btn);
  lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Click me!");
  lv_obj_center(label);

  lv_group_add_obj(g, btn);
  // ui_init();
}

void gui_loop()
{
  btn_next.tick();
  btn_prev.tick();
  btn_enter.tick();
}
