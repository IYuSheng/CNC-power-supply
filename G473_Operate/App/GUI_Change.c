#include "Gui_Change.h"

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
    // 更新启动状态显示
    if (current_data.mode_stop == Stop)
    {
        // 停止时重置能量
        energy_out = 0.0f;
        lv_label_set_text(g_ui.screen_label_Start, "停止");
    }
    else if (current_data.mode_stop == Run)
    {
        // 启动状态时计算累计能量
        energy_out += current_out  * Tran_mAh;
        lv_label_set_text(g_ui.screen_label_Start, "启动");
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
    }
}
