#include "CommandHandlers.h"

extern UART_TxStruct ua;

/* 处理SetA命令 */
static void HandleSetA(const char* param)
{
  if (param == NULL)
    {
      fr_printf("SetDAC_A: missing parameter");
      return;
    }

  char* endPtr;
  long value = strtol(param, &endPtr, 10);

  if (endPtr != param && *endPtr == '\0')
    {
      ua.reserved[0] = (uint8_t)value;
      fr_printf("SetDAC_A updated to: %d", ua.reserved[0]);
    }
  else
    {
      fr_printf("SetDAC_A: invalid parameter");
    }
}

/* 处理SetB命令 */
static void HandleSetB(const char* param)
{
  if (param == NULL)
    {
      fr_printf("SetDAC_B: missing parameter");
      return;
    }

  char* endPtr;
  long value = strtol(param, &endPtr, 10);

  if (endPtr != param && *endPtr == '\0')
    {
      ua.reserved[1] = (uint8_t)value;
      fr_printf("SetDAC_B updated to: %d", ua.reserved[1]);
    }
  else
    {
      fr_printf("SetDAC_B: invalid parameter");
    }
}

/* 处理操作采集端命令 */
static void HandleSetGather(const char* param)
{
    if (param == NULL)
    {
        fr_printf("SetGather: missing parameter");
        return;
    }

    /* 处理ShutDown命令 */
    if (strcmp(param, "ShutDown") == 0)
    {
        // 执行采集端关闭逻辑
        fr_printf("SetGather: Device Shut Down");
    }
    /* 处理Open命令 */
    else if (strcmp(param, "Open") == 0)
    {
        // 执行采集端开启逻辑
        fr_printf("SetGather: Device Open");
    }
    /* 未知参数 */
    else
    {
        fr_printf("SetGather: invalid parameter");
    }
}


/* 注册所有命令处理函数 */
void RegisterAllCommands(void)
{
  RegisterCommand("SetDAC_A", HandleSetA);
  RegisterCommand("SetDAC_B", HandleSetB);
	RegisterCommand("SetGather", HandleSetGather);
}
