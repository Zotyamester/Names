#pragma once

#include <windows.h>

typedef enum operation_t
{
	OP_ADD = 0
} OPERATION;

typedef struct stack_t STACK;
typedef STACK* LPSTACK;

BOOL StackPush(LPSTACK* ppStack, LPSTR str, OPERATION op);
BOOL StackPop(LPSTACK* ppStack, LPSTR* pStr, OPERATION* pOp);
VOID StackClear(LPSTACK* ppStack);
