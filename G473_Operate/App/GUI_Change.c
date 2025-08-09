#include "Gui_Change.h"

// 运行时间计数（秒）
static uint32_t run_seconds = 0;
// FreeRTOS软件定时器句柄
static TimerHandle_t xRunTimeTimer = NULL;
// 标记是否正在运行（避免重复启动定时器）
static bool is_timer_running = false;

char time_str[16];

/**
 * @brief 运行时间定时器回调函数
 * @note 每1秒触发一次，累加运行秒数并更新UI显示
 */
void vRunTimeTimerCallback(TimerHandle_t xRunTimeTimer)
{
    // 累加秒数
    run_seconds++;
    
    // 转换为时分秒格式
    uint8_t hours, minutes, seconds;
    ConvertSecondsToHMS(run_seconds, &hours, &minutes, &seconds);
    // 格式化时间字符串并更新LVGL标签
    sprintf(time_str, "%02d:%02d:%02d", hours, minutes, seconds);
}

static float voltage_in = 0.0f;
static float current_in = 0.0f;
static float voltage_out = 0.0f;
static float current_out = 0.0f;
static float power_out = 0.0f;
static float energy_out = 0.0f;
static float voltage_set = 0.0f;
static float current_set = 0.0f;
static float temperature = 0.0f;

void Gui_Event_Data(void)
{
    /*----------------------接收数据显示---------------------*/
    UART_RxStruct current_data = get_uart_rx_data();
    char buffer[20];
    
    voltage_out = (float)current_data.voltage_out / 1000.0f;
    current_out = (float)current_data.current_out / 1000.0f;
    voltage_in = (float)current_data.voltage_in  / 1000.0f;
    current_in = (float)current_data.current_in  / 1000.0f;
    temperature = ConvertNTCTemperature(current_data.adc_tmp1);
    power_out = voltage_out * current_out;

    // 更新当前输出电压显示
    sprintf(buffer, "%0.3f", voltage_out);
    lv_label_set_text(g_ui.screen_label_Voltage_now, buffer);
        
    // 更新当前输出电流显示
    sprintf(buffer, "%0.3f", current_out);
    lv_label_set_text(g_ui.screen_label_Current_now, buffer);

    // 更新输出功率显示
    sprintf(buffer, "\n%0.3f W", power_out);
    lv_label_set_text(g_ui.screen_label_power, buffer);

    // 更新能量显示
    sprintf(buffer, "\n%0.2f mAh", energy_out);
    lv_label_set_text(g_ui.screen_label_energy, buffer);

    // 更新输入电压显示
    sprintf(buffer, "%0.2fV", voltage_in);
    lv_label_set_text(g_ui.screen_label_in_v, buffer);

    // 输入电流显示
    sprintf(buffer, "%0.2fA", current_in);
    lv_label_set_text(g_ui.screen_label_in_a, buffer);
        
    // 更新温度显示
    sprintf(buffer, "%0.1f°C", temperature);
    lv_label_set_text(g_ui.screen_label_Temperature, buffer);

    /*-------------------发送数据显示------------------*/
    UART_TxStruct tx_data = get_uart_tx_data();

    //将设定DAC输出电压转换为实际值
    voltage_set = tx_data.dac_b * 21.0f;
    //将设定DAC输出电流转换为实际值
    current_set = (tx_data.dac_a - 1.65f) * 21.0f;

    // 显示设置电压
    sprintf(buffer, "%0.3fV", voltage_set);
    lv_label_set_text(g_ui.screen_label_Voltage_Set, buffer);
    
    // 显示设置电流
    sprintf(buffer, "%0.3fA", current_set);
    lv_label_set_text(g_ui.screen_label_Current_Set, buffer);

    /*--------------状态切换------------*/ 
    // 更新启动状态显示及运行时间
    if (current_data.mode_stop == Stop)
    {
        // 停止状态：重置能量，停止定时器
        energy_out = 0.0f;
        lv_obj_set_style_bg_opa(g_ui.screen_label_Start, 69, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_label_set_text(g_ui.screen_label_Start, "  S");
        lv_obj_set_style_bg_color(g_ui.screen_label_Start, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
        
        // 显示初始时间
        lv_label_set_text(g_ui.screen_label_Timeout, "00:00:00");
        
        // 停止定时器（如果正在运行）
        if (is_timer_running && xRunTimeTimer != NULL)
        {
            xTimerStop(xRunTimeTimer, 0);
            is_timer_running = false;
        }
    }
    else if (current_data.mode_stop == Run)
    {
        // 启动状态：计算累计能量，启动定时器
        energy_out += current_out * Tran_mAh;
        lv_obj_set_style_bg_opa(g_ui.screen_label_Start, 240, LV_PART_MAIN|LV_STATE_DEFAULT);
        lv_label_set_text(g_ui.screen_label_Start, "  R");
        lv_obj_set_style_bg_color(g_ui.screen_label_Start, lv_color_hex(0x1d741f), LV_PART_MAIN|LV_STATE_DEFAULT);

        // 启动定时器（如果未运行）
        if (!is_timer_running && xRunTimeTimer != NULL)
        {
            xTimerStart(xRunTimeTimer, 0);
            is_timer_running = true;
            run_seconds = 0; // 重置运行时间
            memcpy(time_str, "00:00:00", sizeof(time_str)); // 初始化时间字符串
        }
        // 更新运行时间显示
        lv_label_set_text(g_ui.screen_label_Timeout, time_str);
    }

    // 更新工作模式显示
    if(current_data.mode_flag == Voltage_LOOP)         // 电压环模式
    {
        // 切换模式显示为电压环
        lv_label_set_text(g_ui.screen_label_mode, "C C");
        // 更新主显示为输出电压
        sprintf(buffer, "%0.3f", voltage_out);
        lv_label_set_text(g_ui.screen_label_main,buffer);
        lv_label_set_text(g_ui.screen_label_select_unit, "V");
    }
    else if (current_data.mode_flag == Current_LOOP)   // 电流环模式
    {
        // 切换模式显示为电流环
        lv_label_set_text(g_ui.screen_label_mode, "C V");
        // 更新主显示为输出电流
        sprintf(buffer, "%0.3f", current_out);
        lv_label_set_text(g_ui.screen_label_main,buffer);
        lv_label_set_text(g_ui.screen_label_select_unit, "A");
    }
    else                                               // 未启动模式
    {
        lv_label_set_text(g_ui.screen_label_mode, "CC / CV");
        // 当停止时默认更新主显示为输出电压
        sprintf(buffer, "%0.3f", voltage_out);
        lv_label_set_text(g_ui.screen_label_main,buffer);
        lv_label_set_text(g_ui.screen_label_select_unit, "V");
    }
}

void Timer_Init(void)
{
    if (xRunTimeTimer == NULL)
    {
        // 创建软件定时器：周期1000ms，自动重载
        xRunTimeTimer = xTimerCreate(
            "RunTimeTimer",          // 定时器名称
            pdMS_TO_TICKS(1000),     // 周期1秒
            pdTRUE,                  // 自动重载
            NULL,                    // 不传递参数
            vRunTimeTimerCallback    // 回调函数
        );
    }
}
