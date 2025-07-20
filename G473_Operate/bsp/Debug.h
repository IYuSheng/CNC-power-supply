/* Debug.h */
#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>
#include <stdbool.h>

/* 命令处理函数原型 */
typedef void (*CommandHandler)(const char* param);

/* 注册命令处理函数 */
void RegisterCommand(const char* cmd, CommandHandler handler);

/* 初始化命令处理器 */
void CommandProcessorInit(void);

/* 处理接收到的命令 */
void ProcessCommand(const char* cmdLine);

/* 标准错误处理函数 */
void DefaultErrorHandler(const char* errorMsg);

void TriggerCommandProcessing(const char* cmdLine);

#endif /* __DEBUG_H */
