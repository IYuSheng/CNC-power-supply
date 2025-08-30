/* LVGL_Init.h 文件 */
#ifndef __LVGL_INIT_H
#define __LVGL_INIT_H

#include "lvgl.h"
#include "lv_port_disp.h"
#include "st7789.h"

#define LVGL_TICK_PERIOD_MS 50 // LVGL定时器周期，单位为毫秒
#define Tran_mWh 0.001389f     // 充电能量转换系数

// UI界面结构体定义
typedef struct {
    // 主屏幕对象
    lv_obj_t *screen;
    
    // 电压显示相关控件
    lv_obj_t *screen_label_voltage;        // 电压标签背景
    lv_obj_t *screen_label_Voltage_now;    // 当前电压值
    lv_obj_t *screen_label_Voltage_Set;    // 设置电压值
    lv_obj_t *screen_btn_Voltage;          // 电压按钮
    lv_obj_t *screen_btn_Voltage_label;    // 电压按钮标签
    
    // 电流显示相关控件
    lv_obj_t *screen_label_Current;        // 电流标签背景
    lv_obj_t *screen_label_Current_now;    // 当前电流值
    lv_obj_t *screen_label_Current_Set;    // 设置电流值
    lv_obj_t *screen_btn_Current;          // 电流按钮
    lv_obj_t *screen_btn_Current_label;    // 电流按钮标签
    
    // 功率显示相关控件
    lv_obj_t *screen_label_power;          // 功率标签背景
    lv_obj_t *screen_btn_Power;            // 功率按钮
    lv_obj_t *screen_btn_Power_label;      // 功率按钮标签
    
    // 能量显示相关控件
    lv_obj_t *screen_label_energy;         // 能量标签背景
    lv_obj_t *screen_btn_energy;           // 能量按钮
    lv_obj_t *screen_btn_energy_label;     // 能量按钮标签
    
    // 状态显示相关控件
    lv_obj_t *screen_label_Timeout;        // 超时时间显示
    lv_obj_t *screen_label_Temperature;    // 温度显示
    lv_obj_t *screen_label_Start;          // 启动状态显示
    lv_obj_t *screen_label_mode;           // 工作模式显示
    lv_obj_t *screen_label_main;           // 主显示数值
    lv_obj_t *screen_label_select_unit;    // 单位显示
    
    // 输入信息显示相关控件
    lv_obj_t *screen_label_in1;            // 输入标签1
    lv_obj_t *screen_label_in2;            // 输入标签2
    lv_obj_t *screen_label_in_v;           // 输入电压值
    lv_obj_t *screen_label_in_a;           // 输入电流值
    lv_obj_t *screen_label_set_v;          // 设置电压标签
    lv_obj_t *screen_label_set_a;          // 设置电流标签
    
    // 分隔线
    lv_obj_t *screen_line_V;               // 电压分隔线
    lv_obj_t *screen_line_A;               // 电流分隔线
} lv_ui_t;

// 全局UI结构体实例声明
extern lv_ui_t g_ui;
extern bool main_screen_loaded;

void Gui_Init(void);
void create_splash_screen(void);

#endif /* __LVGL_INIT_H */
