/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"
#include <stdlib.h>

// 文件顶部添加全局变量
static lv_coord_t x_axis_max = 20; // 初始X轴范围
static lv_coord_t current_x = 0;   // 当前X值

void setup_scr_screen(lv_ui *ui)
{
    //Write codes screen
    ui->screen = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_cont_1
    ui->screen_cont_1 = lv_obj_create(ui->screen);
    lv_obj_set_pos(ui->screen_cont_1, 0, 0);
    lv_obj_set_size(ui->screen_cont_1, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_chart_1
    ui->screen_chart_1 = lv_chart_create(ui->screen_cont_1);
    lv_chart_set_type(ui->screen_chart_1, LV_CHART_TYPE_LINE);
    lv_chart_set_div_line_count(ui->screen_chart_1, 6, 8);
    lv_chart_set_point_count(ui->screen_chart_1, 20);
    lv_chart_set_range(ui->screen_chart_1, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_axis_tick(ui->screen_chart_1, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 5, true, 40);
    lv_chart_set_range(ui->screen_chart_1, LV_CHART_AXIS_SECONDARY_Y, 0, 100);
    lv_chart_set_axis_tick(ui->screen_chart_1, LV_CHART_AXIS_PRIMARY_X, 10, 6, 11, 5, true, 16);
    lv_chart_set_zoom_x(ui->screen_chart_1, 256);
    lv_chart_set_zoom_y(ui->screen_chart_1, 256);
    lv_obj_set_style_size(ui->screen_chart_1, 0, LV_PART_INDICATOR);
    ui->screen_chart_1_0 = lv_chart_add_series(ui->screen_chart_1, lv_color_hex(0x00FF00), LV_CHART_AXIS_PRIMARY_Y);
#if LV_USE_FREEMASTER == 0
    lv_chart_set_next_value(ui->screen_chart_1, ui->screen_chart_1_0, 13);
    lv_chart_set_next_value(ui->screen_chart_1, ui->screen_chart_1_0, 49);
#endif
    ui->screen_chart_1_1 = lv_chart_add_series(ui->screen_chart_1, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
#if LV_USE_FREEMASTER == 0
    lv_chart_set_next_value(ui->screen_chart_1, ui->screen_chart_1_1, 28);
    lv_chart_set_next_value(ui->screen_chart_1, ui->screen_chart_1_1, 25);
#endif
    lv_obj_set_pos(ui->screen_chart_1, 38, 19);
    lv_obj_set_size(ui->screen_chart_1, 257, 187);
    lv_obj_set_scrollbar_mode(ui->screen_chart_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_chart_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_chart_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_chart_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_chart_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_chart_1, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_chart_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_chart_1, lv_color_hex(0xe8e8e8), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_chart_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_chart_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(ui->screen_chart_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->screen_chart_1, lv_color_hex(0xe8e8e8), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->screen_chart_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_chart_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_chart_1, Part: LV_PART_TICKS, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_chart_1, lv_color_hex(0x151212), LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_chart_1, &lv_font_montserratMedium_12, LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_chart_1, 255, LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(ui->screen_chart_1, 2, LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->screen_chart_1, lv_color_hex(0xe8e8e8), LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->screen_chart_1, 255, LV_PART_TICKS|LV_STATE_DEFAULT);

    //The custom code of screen.
    // 修改图表初始化参数
    lv_chart_set_type(ui->screen_chart_1, LV_CHART_TYPE_SCATTER); // 改为散点图
    lv_chart_set_point_count(ui->screen_chart_1, 20);           // 增加数据点数
    lv_chart_set_range(ui->screen_chart_1, LV_CHART_AXIS_PRIMARY_X, 0, x_axis_max);
    lv_chart_set_axis_tick(ui->screen_chart_1, LV_CHART_AXIS_PRIMARY_X, 
                          5, 2, 5, 2, true, 40); // 优化刻度

		
    //Update current screen layout.
    lv_obj_update_layout(ui->screen);

}

void trans(lv_ui *ui) {
    static uint32_t last_tick = 0;
    static uint8_t y = 0;
    const uint32_t interval = 100; // 更新间隔200ms

    if (lv_tick_elaps(last_tick) > interval) {
        last_tick = lv_tick_get();

        // 生成Y轴数据
        y = (y + 1) % 100;

        // 添加新数据点
        lv_chart_set_next_value2(ui->screen_chart_1, ui->screen_chart_1_0, current_x, y);

        // 当数据点超过20时，自动滚动图表
        if (current_x >= 20) {
            lv_coord_t new_x_start = current_x - 19; // 保持显示最近的20个数据点
            lv_chart_set_range(ui->screen_chart_1, LV_CHART_AXIS_PRIMARY_X, new_x_start, new_x_start + 19);
        }

        current_x++;
    }
}
