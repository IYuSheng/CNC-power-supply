#include "Gui_Change.h"

// 运行时长计数（秒）
static volatile uint32_t run_seconds = 0;
// FreeRTOS运行时计时器句柄
static TimerHandle_t xRunTimeTimer = NULL;
// 标记计时器是否正在运行，防止重复创建计时器
static bool is_timer_running = false;

char time_str[16];

/**
 * @brief 运行时计时器回调函数
 * @note 每1秒触发一次，累计运行时间并更新UI显示
 */
void vRunTimeTimerCallback(TimerHandle_t xRunTimeTimer)
{
    // 累计秒数
    run_seconds++;
    
    // 转换为时分秒格式
    uint8_t hours, minutes, seconds;
    ConvertSecondsToHMS(run_seconds, &hours, &minutes, &seconds);
    // 格式化时间字符串用于LVGL标签
    lv_snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);
}

static volatile float voltage_in = 0.0f;
static volatile float current_in = 0.0f;
static volatile float voltage_out = 0.0f;
static volatile float current_out = 0.0f;
static volatile float power_out = 0.0f;
static volatile float energy_out = 0.0f;
static volatile float voltage_set = 0.0f;
static volatile float current_set = 0.0f;
static volatile float temperature = 0.0f;

// 添加静态变量来存储上一次的值
static float last_voltage_in = -1.0f;
static float last_current_in = -1.0f;
static float last_voltage_out = -1.0f;
static float last_current_out = -1.0f;
static float last_power_out = -1.0f;
static float last_energy_out = -1.0f;
static float last_voltage_set = -1.0f;
static float last_current_set = -1.0f;
static float last_temperature = -1.0f;
static uint8_t last_mode_stop = 0xFF;
static uint8_t last_mode_flag = 0xFF;

void Gui_Event_Data(void)
{
    /*----------------------数据更新显示---------------------*/
    UART_RxStruct current_data = get_uart_rx_data();
    char buffer[20];
    
    voltage_out = (float)current_data.voltage_out / 1000.0f;
    current_out = (float)current_data.current_out / 1000.0f;
    voltage_in = (float)current_data.voltage_in  / 1000.0f;
    current_in = (float)current_data.current_in  / 1000.0f;
    temperature = ConvertNTCTemperature(current_data.adc_tmp1);
    power_out = voltage_out * current_out;

    // 更新当前输出电压显示（仅在变化时）
    if (last_voltage_out != voltage_out)
    {
        last_voltage_out = voltage_out;
        lv_snprintf(buffer, sizeof(buffer), "%.3f", voltage_out);
        lv_label_set_text(g_ui.screen_label_Voltage_now, buffer);

        if(current_data.mode_flag == Voltage_LOOP || current_data.mode_flag == Disable_LOOP)
        {
            // 主数据显示为当前电压
            lv_label_set_text(g_ui.screen_label_main,buffer);
        }
    }
        
    // 更新当前输出电流显示（仅在变化时）
    if (last_current_out != current_out)
    {
        last_current_out = current_out;
        lv_snprintf(buffer, sizeof(buffer), "%.3f", current_out);
        lv_label_set_text(g_ui.screen_label_Current_now, buffer);

        if (current_data.mode_flag == Current_LOOP)
        {
            // 主数据显示为当前电流
            lv_label_set_text(g_ui.screen_label_main,buffer);
        }
    }

    // 功率数据显示（仅在变化时）
    if (last_power_out != power_out) {
        last_power_out = power_out;
        if(power_out <= 10.0f)
        {
            lv_snprintf(buffer, sizeof(buffer), "\n%.3f W", power_out);
        }
        else if(power_out <= 100.0f)
        {
            lv_snprintf(buffer, sizeof(buffer), "\n%.2f W", power_out);
        }
        else if(power_out <= 1000.0f)
        {
            lv_snprintf(buffer, sizeof(buffer), "\n%.1f W", power_out);
        }
        else
        {
            lv_snprintf(buffer, sizeof(buffer), "\n%.0f W", power_out);
        }
        lv_label_set_text(g_ui.screen_label_power, buffer);
    }

    // 能量数据显示（仅在变化时）
    if (last_energy_out != energy_out) {
        last_energy_out = energy_out;
        if(energy_out <= 10.0f)
        {
            lv_snprintf(buffer, sizeof(buffer), "\n%.2f mWh", energy_out);
        }
        else
        {
            lv_snprintf(buffer, sizeof(buffer), "\n%.2f Wh", energy_out / 1000.0f);
        }
        lv_label_set_text(g_ui.screen_label_energy, buffer);
    }

    // 输入电压显示（仅在变化时）
    if (last_voltage_in != voltage_in) {
        last_voltage_in = voltage_in;
        lv_snprintf(buffer, sizeof(buffer), "%.2fV", voltage_in);
        lv_label_set_text(g_ui.screen_label_in_v, buffer);
    }

    // 输入电流显示（仅在变化时）
    if (last_current_in != current_in) {
        last_current_in = current_in;
        lv_snprintf(buffer, sizeof(buffer), "%.2fA", current_in);
        lv_label_set_text(g_ui.screen_label_in_a, buffer);
    }
        
    // 温度显示（仅在变化时）
    if (last_temperature != temperature) {
        last_temperature = temperature;
        lv_snprintf(buffer, sizeof(buffer), "%.1f°C", temperature);
        lv_label_set_text(g_ui.screen_label_Temperature, buffer);
    }

    /*-------------------设置参数显示------------------*/
    UART_TxStruct tx_data = get_uart_tx_data();

    //将设定DAC电压转换为实际值
    voltage_set = tx_data.dac_b * 21.0f;
    //将设定DAC电流转换为实际值
    current_set = (tx_data.dac_a - 1.65f) * 21.0f;

    // 显示设定电压（仅在变化时）
    if (last_voltage_set != voltage_set) {
        last_voltage_set = voltage_set;
        lv_snprintf(buffer, sizeof(buffer), "%.3fV", voltage_set);
        lv_label_set_text(g_ui.screen_label_Voltage_Set, buffer);
    }
    
    // 显示设定电流（仅在变化时）
    if (last_current_set != current_set) {
        last_current_set = current_set;
        lv_snprintf(buffer, sizeof(buffer), "%.3fA", current_set);
        lv_label_set_text(g_ui.screen_label_Current_Set, buffer);
    }

    /*--------------状态切换------------*/ 
    // 根据运行状态显示切换停止状态和运行时的计时
    if (current_data.mode_stop != last_mode_stop)
    {
        last_mode_stop = current_data.mode_stop;
        
        if (current_data.mode_stop == Stop)
        {
            // 停止状态时清空累计的能量和运行时间
            energy_out = 0.0f;
            lv_obj_set_style_bg_opa(g_ui.screen_label_Start, 69, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_label_set_text(g_ui.screen_label_Start, "  S");
            lv_obj_set_style_bg_color(g_ui.screen_label_Start, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
            
            // 显示初始时间
            lv_label_set_text(g_ui.screen_label_Timeout, "00:00:00");
            
            // 停止运行时计时器时防止重复创建
            if (is_timer_running && xRunTimeTimer != NULL)
            {
                xTimerStop(xRunTimeTimer, 0);
                is_timer_running = false;
            }
        }
        else if (current_data.mode_stop == Run)
        {
            // 更新运行状态显示
            lv_obj_set_style_bg_opa(g_ui.screen_label_Start, 240, LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_label_set_text(g_ui.screen_label_Start, "  R");
            lv_obj_set_style_bg_color(g_ui.screen_label_Start, lv_color_hex(0x1d741f), LV_PART_MAIN|LV_STATE_DEFAULT);

            // 如果运行时计时器未创建则创建
            if (!is_timer_running && xRunTimeTimer != NULL)
            {
                xTimerStart(xRunTimeTimer, 0);
                is_timer_running = true;
                run_seconds = 0; // 重置运行时长
                lv_snprintf(time_str, sizeof(time_str), "00:00:00"); // 初始化时间字符串
            }
        }
    }
    
    // 持续处理能量累积（仅在运行状态下）
    if (current_data.mode_stop == Run)
    {
        // 运行状态时累计的能量
        energy_out += power_out * Tran_mWh;
        
        // 运行时时间显示
        lv_label_set_text(g_ui.screen_label_Timeout, time_str);
    }

    // 更新工作模式显示（仅在变化时）
    if (current_data.mode_flag != last_mode_flag) {
        last_mode_flag = current_data.mode_flag;
        
        if(current_data.mode_flag == Voltage_LOOP)         // 电压模式
        {
            // 切换模式显示为恒压
            lv_label_set_text(g_ui.screen_label_mode, "C C");
            // 主数据显示为当前电压
            lv_label_set_text(g_ui.screen_label_select_unit, "V");
        }
        else if (current_data.mode_flag == Current_LOOP)   // 电流模式
        {
            // 切换模式显示为恒流
            lv_label_set_text(g_ui.screen_label_mode, "C V");
            // 主数据显示为当前电流
            lv_label_set_text(g_ui.screen_label_select_unit, "A");
        }
        else                                               // 未定义模式
        {
            lv_label_set_text(g_ui.screen_label_mode, "CC / CV");
            // 停止时默认主数据显示为当前电压
            lv_label_set_text(g_ui.screen_label_select_unit, "V");
        }
    }
}

void Timer_Init(void)
{
    if (xRunTimeTimer == NULL)
    {
        // 创建运行时计时器任务，每1000ms自动触发
        xRunTimeTimer = xTimerCreate(
            "RunTimeTimer",          // 计时器名称
            pdMS_TO_TICKS(1000),     // 周期1秒
            pdTRUE,                  // 自动重载
            NULL,                    // 不使用参数
            vRunTimeTimerCallback    // 回调函数
        );
    }
}
