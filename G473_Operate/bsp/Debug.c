/* Debug.c */
#include "Debug.h"
#include <string.h>
#include <stdlib.h>
#include "Uart_Debug.h"

/* 命令表最大项数 */
#define MAX_COMMANDS 16

/* 命令表项结构 */
typedef struct
{
  const char* cmd;
  CommandHandler handler;
} CommandEntry;

/* 命令表 */
static CommandEntry commandTable[MAX_COMMANDS] = {0};
static uint8_t commandCount = 0;

/* 错误处理函数指针 */
static void (*errorHandler)(const char* errorMsg) = DefaultErrorHandler;

/* 初始化命令处理器 */
void CommandProcessorInit(void)
{
  memset(commandTable, 0, sizeof(commandTable));
  commandCount = 0;
}

/* 注册命令处理函数 */
void RegisterCommand(const char* cmd, CommandHandler handler)
{
  if (commandCount >= MAX_COMMANDS || handler == NULL || cmd == NULL)
    {
      if (errorHandler) errorHandler("Command registration failed");
      return;
    }

  /* 检查命令是否已存在 */
  for (uint8_t i = 0; i < commandCount; i++)
    {
      if (strcmp(commandTable[i].cmd, cmd) == 0)
        {
          if (errorHandler) errorHandler("Command already exists");
          return;
        }
    }

  /* 添加新命令 */
  commandTable[commandCount].cmd = cmd;
  commandTable[commandCount].handler = handler;
  commandCount++;
}

/* 设置错误处理函数 */
void SetErrorHandler(void (*handler)(const char* errorMsg))
{
  errorHandler = handler;
}

/* 从命令行提取参数 */
static const char* ExtractParameter(const char* cmdLine)
{
  const char* param = strchr(cmdLine, ':');
  if (param)
    {
      return param + 1; /* 跳过冒号 */
    }
  return NULL;
}

/* 处理接收到的命令 */
void ProcessCommand(const char* cmdLine)
{
  if (cmdLine == NULL || *cmdLine == '\0') return;

  /* 查找匹配的命令 */
  for (uint8_t i = 0; i < commandCount; i++)
    {
      const char* cmd = commandTable[i].cmd;
      size_t cmdLen = strlen(cmd);

      /* 检查命令前缀匹配且后面是冒号或结束符 */
      if (strncmp(cmdLine, cmd, cmdLen) == 0 &&
          (cmdLine[cmdLen] == ':' || cmdLine[cmdLen] == '\0'))
        {

          const char* param = ExtractParameter(cmdLine);
          commandTable[i].handler(param);
          return;
        }
    }

  /* 未找到匹配命令 */
  if (errorHandler) errorHandler("Unknown command");
}

/* 触发命令处理（统一接口，处理所有类型的命令） */
void TriggerCommandProcessing(const char* cmdLine)
{
    if (cmdLine == NULL || *cmdLine == '\0')
        return;

    /* 处理基本命令（如SetA、SetB等） */
    ProcessCommand(cmdLine);

		/* 处理系统监视 */
#if Monitor_Flag
		vSystemMonitorTask(NULL);
#endif
}



/* 标准错误处理函数 */
void DefaultErrorHandler(const char* errorMsg)
{
  fr_printf("Error: %s", errorMsg);
}
