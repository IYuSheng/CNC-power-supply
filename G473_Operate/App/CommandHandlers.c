#include "CommandHandlers.h"

extern UART_TxStruct send_gather;

/* 处理SetA命令 */
static void HandleSetA(const char* param)
{
  HandleSetDAC(param, &send_gather.dac_a, "SetDAC_A");
}

/* 处理SetB命令 */
static void HandleSetB(const char* param)
{
  HandleSetDAC(param, &send_gather.dac_b, "SetDAC_B");
}

/* 处理设置输出电压命令 */
static void HandleSetVoltage(const char* param)
{
  CHECK_NULL_PARAM(param, "V_Set");

  char* endPtr;
  float value = strtof(param, &endPtr);

  if (endPtr != param && *endPtr == '\0')
    {
      send_gather.dac_b = TransformVoltage(value);  //将电压转换为DAC输出电压值

      fr_printf("V_Set updated to: %.6f", send_gather.dac_b);
    }
  else
    {
      fr_printf("V_Set: invalid parameter");
    }
}

static void HandleSetCurrent(const char* param)
{
  CHECK_NULL_PARAM(param, "A_Set");

  char* endPtr;
  float value = strtof(param, &endPtr);

  if (endPtr != param && *endPtr == '\0')
    {
      send_gather.dac_a = TransformCurrent(value);
      fr_printf("A_Set updated to: %.6f", send_gather.dac_a);
    }
  else
    {
      fr_printf("A_Set: invalid parameter");
    }
}

/* 处理操作采集端命令 */
static void HandleSetGather(const char* param)
{
  CHECK_NULL_PARAM(param, "SetGather");

  HANDLE_COMMAND(param, "ShutDown", fr_printf("SetGather: Device Shut Down"));
  HANDLE_COMMAND(param, "Open", fr_printf("SetGather: Device Open"));
	
}

CommandMapping commandMappings[] =
{
  {"SetDAC_A", HandleSetA},
  {"SetDAC_B", HandleSetB},
  {"SetGather", HandleSetGather},
  {"V_Set", HandleSetVoltage},
  {"A_Set", HandleSetCurrent},
};

/* 注册所有命令处理函数 */
void RegisterAllCommands(void)
{
  for (size_t i = 0; i < sizeof(commandMappings) / sizeof(commandMappings[0]); ++i)
    {
      RegisterCommand(commandMappings[i].command, commandMappings[i].handler);
    }
}
