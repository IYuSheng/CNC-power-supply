/* CommandHandlers.h нд╪Ч */
#ifndef __COMMANDHANDLERS_H
#define __COMMANDHANDLERS_H

#include "Debug.h"
#include "Uart_Debug.h"
#include "Monitor.h"
#include "Control.h"
#include "Uart_comm.h"
#include <string.h>
#include <stdlib.h>

#define CHECK_NULL_PARAM(param, functionName) \
  if (param == NULL) { \
    fr_printf("%s: missing parameter", functionName); \
    return; \
  }

#define HANDLE_COMMAND(param, command, action) \
  if (strcmp(param, command) == 0) { \
    action; \
    return; \
  }

typedef struct
{
  const char* command;
  void (*handler)(const char*);
} CommandMapping;

void RegisterAllCommands(void);

#endif /* __COMMANDHANDLERS_H */
