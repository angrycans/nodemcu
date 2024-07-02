// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.1
// LVGL version: 8.3.11
// Project name: ebike_demo

#include "../ui.h"


// COMPONENT Roller

lv_obj_t *ui_Roller_create(lv_obj_t *comp_parent) {

lv_obj_t *cui_Roller;
cui_Roller = lv_obj_create(comp_parent);
lv_obj_set_width( cui_Roller, 150);
lv_obj_set_height( cui_Roller, 103);
lv_obj_set_align( cui_Roller, LV_ALIGN_CENTER );
lv_obj_clear_flag( cui_Roller, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
lv_obj_set_style_bg_color(cui_Roller, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_Roller, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_border_color(cui_Roller, lv_color_hex(0x293031), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_border_opa(cui_Roller, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_border_width(cui_Roller, 1, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_border_side(cui_Roller, LV_BORDER_SIDE_RIGHT, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_left(cui_Roller, 10, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_right(cui_Roller, 10, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_top(cui_Roller, 2, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_pad_bottom(cui_Roller, 2, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t *cui_Label_Roller;
cui_Label_Roller = lv_label_create(cui_Roller);
lv_obj_set_width( cui_Label_Roller, LV_SIZE_CONTENT);  /// 1
lv_obj_set_height( cui_Label_Roller, LV_SIZE_CONTENT);   /// 1
lv_label_set_text(cui_Label_Roller,"Bike mode");
lv_obj_set_style_text_color(cui_Label_Roller, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(cui_Label_Roller, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(cui_Label_Roller, &ui_font_Big, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_t *cui_Roller2;
cui_Roller2 = lv_roller_create(cui_Roller);
lv_roller_set_options( cui_Roller2, "Eco\nCity\nMontain\nRace", LV_ROLLER_MODE_NORMAL );
lv_obj_set_height( cui_Roller2, 68);
lv_obj_set_width( cui_Roller2, lv_pct(100));
lv_obj_set_align( cui_Roller2, LV_ALIGN_BOTTOM_MID );
lv_obj_set_style_text_color(cui_Roller2, lv_color_hex(0x839DA4), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(cui_Roller2, 170, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_align(cui_Roller2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(cui_Roller2, &ui_font_Medium, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_radius(cui_Roller2, 6, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(cui_Roller2, lv_color_hex(0x252525), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_Roller2, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_bg_grad_color(cui_Roller2, lv_color_hex(0x080C10), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_bg_grad_dir(cui_Roller2, LV_GRAD_DIR_VER, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_outline_color(cui_Roller2, lv_color_hex(0x7BAAC5), LV_PART_MAIN | LV_STATE_DEFAULT );
lv_obj_set_style_outline_opa(cui_Roller2, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_outline_width(cui_Roller2, 1, LV_PART_MAIN| LV_STATE_DEFAULT);
lv_obj_set_style_outline_pad(cui_Roller2, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

lv_obj_set_style_text_color(cui_Roller2, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT );
lv_obj_set_style_text_opa(cui_Roller2, 255, LV_PART_SELECTED| LV_STATE_DEFAULT);
lv_obj_set_style_text_font(cui_Roller2, &ui_font_Medium, LV_PART_SELECTED| LV_STATE_DEFAULT);
lv_obj_set_style_bg_color(cui_Roller2, lv_color_hex(0xD56920), LV_PART_SELECTED | LV_STATE_DEFAULT );
lv_obj_set_style_bg_opa(cui_Roller2, 255, LV_PART_SELECTED| LV_STATE_DEFAULT);

lv_obj_t ** children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_ROLLER_NUM);
children[UI_COMP_ROLLER_ROLLER] = cui_Roller;
children[UI_COMP_ROLLER_LABEL_ROLLER] = cui_Label_Roller;
children[UI_COMP_ROLLER_ROLLER2] = cui_Roller2;
lv_obj_add_event_cb(cui_Roller, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
lv_obj_add_event_cb(cui_Roller, del_component_child_event_cb, LV_EVENT_DELETE, children);
ui_comp_Roller_create_hook(cui_Roller);
return cui_Roller; 
}

