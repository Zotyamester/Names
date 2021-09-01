#include "stack.h"

typedef struct stack_t
{
	LPSTR str;
	OPERATION op;
	struct stack_t* next;
};

BOOL StackPush(LPSTACK* ppStack, LPSTR str, OPERATION op)
{
	LPSTACK node = (LPSTACK)GlobalAlloc(GPTR, sizeof(STACK));
	if (!node)
	{
		return FALSE;
	}

	node->str = str;
	node->op = op;
	node->next = *ppStack;
	*ppStack = node;

	return TRUE;
}

BOOL StackPop(LPSTACK* ppStack, LPSTR* pStr, OPERATION* pOp)
{
	LPSTACK head = *ppStack;
	if (!head)
	{
		return FALSE;
	}

	*ppStack = head->next;
	*pStr = head->str;
	*pOp = head->op;

	GlobalFree(head);

	return TRUE;
}

VOID StackClear(LPSTACK* ppStack)
{
	LPSTACK mov = *ppStack;

	while (mov != NULL)
	{
		LPSTACK next = mov->next;
		GlobalFree(mov->str);
		GlobalFree(mov);
		mov = next;
	}

	*ppStack = NULL;
}
