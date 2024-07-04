// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.1
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "../ui.h"

void ui_ScreenTimer_screen_init(void)
{
ui_ScreenTimer = lv_obj_create(NULL);
lv_obj_clear_flag( ui_ScreenTimer, LV_OBJ_FLAG_SCROLLABLE );    /// Flags

ui_Panel1 = lv_obj_create(ui_ScreenTimer);
lv_obj_set_width( ui_Panel1, 320);
lv_obj_set_height( ui_Panel1, 80);
lv_obj_set_x( ui_Panel1, 0 );
lv_obj_set_y( ui_Panel1, -5 );
lv_obj_set_align( ui_Panel1, LV_ALIGN_CENTER );
lv_obj_clear_flag( ui_Panel1, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_bg_color(ui_Panel1, lv_color_hex(0xBCB9B9), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(ui_Panel1, 255, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_TimeMs = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_TimeMs, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_TimeMs, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_TimeMs, 102 );
lv_obj_set_y( ui_TimeMs, 7 );
lv_obj_set_align( ui_TimeMs, LV_ALIGN_CENTER );
lv_label_set_text(ui_TimeMs,"378");
lv_obj_set_style_text_font(ui_TimeMs, &ui_font_DSDIGIT64, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label1 = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_Label1, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_Label1, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_Label1, 84 );
lv_obj_set_y( ui_Label1, -77 );
lv_obj_set_align( ui_Label1, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label1,"114");
lv_obj_set_style_text_align(ui_Label1, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_Label1, &lv_font_montserrat_48, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label2 = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_Label2, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_Label2, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_Label2, -135 );
lv_obj_set_y( ui_Label2, -65 );
lv_obj_set_align( ui_Label2, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label2,"LAP");

ui_TimeMinSec = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_TimeMinSec, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_TimeMinSec, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_TimeMinSec, -42 );
lv_obj_set_y( ui_TimeMinSec, -1 );
lv_obj_set_align( ui_TimeMinSec, LV_ALIGN_CENTER );
lv_label_set_text(ui_TimeMinSec,"01:21.");
lv_obj_set_style_text_align(ui_TimeMinSec, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(ui_TimeMinSec, &ui_font_DSDIGIT90, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label4 = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_Label4, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_Label4, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_Label4, 33 );
lv_obj_set_y( ui_Label4, 80 );
lv_obj_set_align( ui_Label4, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label4,"01:32.000");
lv_obj_set_style_text_font(ui_Label4, &ui_font_DSDIGIT64, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label3 = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_Label3, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_Label3, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_Label3, 136 );
lv_obj_set_y( ui_Label3, -66 );
lv_obj_set_align( ui_Label3, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label3,"KMH");

ui_Label5 = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_Label5, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_Label5, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_Label5, -104 );
lv_obj_set_y( ui_Label5, -73 );
lv_obj_set_align( ui_Label5, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label5,"1");
lv_obj_set_style_text_font(ui_Label5, &lv_font_montserrat_48, LV_PART_MAIN| LV_STATE_DEFAULT);

ui_Label6 = lv_label_create(ui_ScreenTimer);
lv_obj_set_width( ui_Label6, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( ui_Label6, LV_SIZE_CONTENT);   /// 1
lv_obj_set_x( ui_Label6, -33 );
lv_obj_set_y( ui_Label6, -75 );
lv_obj_set_align( ui_Label6, LV_ALIGN_CENTER );
lv_label_set_text(ui_Label6,"+1.023");

}
