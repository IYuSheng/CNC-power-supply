/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
  
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_label_voltage;
	lv_obj_t *screen_label_Current;
	lv_obj_t *screen_label_power;
	lv_obj_t *screen_btn_Voltage;
	lv_obj_t *screen_btn_Voltage_label;
	lv_obj_t *screen_btn_Current;
	lv_obj_t *screen_btn_Current_label;
	lv_obj_t *screen_btn_Power;
	lv_obj_t *screen_btn_Power_label;
	lv_obj_t *screen_label_Voltage_now;
	lv_obj_t *screen_label_Current_now;
	lv_obj_t *screen_label_Voltage_Set;
	lv_obj_t *screen_label_Current_Set;
	lv_obj_t *screen_label_Timeout;
	lv_obj_t *screen_label_Temperature;
	lv_obj_t *screen_label_Start;
	lv_obj_t *screen_label_mode;
	lv_obj_t *screen_label_main;
	lv_obj_t *screen_label_select_unit;
	lv_obj_t *screen_label_in1;
	lv_obj_t *screen_label_in2;
	lv_obj_t *screen_label_in_v;
	lv_obj_t *screen_label_in_a;
	lv_obj_t *screen_label_set_v;
	lv_obj_t *screen_label_set_a;
	lv_obj_t *screen_label_energy;
	lv_obj_t *screen_btn_energy;
	lv_obj_t *screen_btn_energy_label;
	lv_obj_t *screen_line_V;
	lv_obj_t *screen_line_A;
	lv_obj_t *screen_1;
	bool screen_1_del;
	lv_obj_t *screen_1_cont_backgrn;
	lv_obj_t *screen_1_btn_slider_y;
	lv_obj_t *screen_1_btn_slider_y_label;
	lv_obj_t *screen_1_label_symbol;
	lv_obj_t *screen_1_img_haibara;
	lv_obj_t *screen_1_label_version;
	lv_obj_t *screen_1_btn_slider_x;
	lv_obj_t *screen_1_btn_slider_x_label;
	lv_obj_t *screen_1_label_power;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, int32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                       uint16_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                       lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_ready_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_ui(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_screen(lv_ui *ui);
void setup_scr_screen_1(lv_ui *ui);

LV_IMG_DECLARE(_background_320x240);
LV_IMG_DECLARE(_start_haibara_alpha_133x150);

LV_FONT_DECLARE(lv_font_montserratMedium_18)
LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_montserratMedium_17)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_18)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_montserratMedium_26)
LV_FONT_DECLARE(lv_font_montserratMedium_14)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_14)
LV_FONT_DECLARE(lv_font_montserratMedium_13)
LV_FONT_DECLARE(lv_font_montserratMedium_73)
LV_FONT_DECLARE(lv_font_montserratMedium_28)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_12)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_16)
LV_FONT_DECLARE(lv_font_montserratMedium_30)
LV_FONT_DECLARE(lv_font_montserratMedium_22)


#ifdef __cplusplus
}
#endif
#endif
