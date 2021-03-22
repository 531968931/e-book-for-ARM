#include <config.h>
#include <input_manager.h>
#include <string.h>


static PT_InputOpr g_ptInputOprHead;

int RegisterInputOpr(PT_InputOpr ptInputOpr)
{
	PT_InputOpr ptTemp;

	if(!g_ptInputOprHead)
	{
		g_ptInputOprHead = ptInputOpr;
		ptInputOpr->PT_Next = NULL;
	}
	else
	{
		ptTemp = g_ptInputOprHead;
		while(ptTemp->PT_Next)
		{
			ptTemp = ptTemp->PT_Next;
		}
		ptTemp->PT_Next = ptInputOpr;
		ptInputOpr->PT_Next = NULL;
	}
	return 0;
}

void ShowInputOpr(void)
{
	int i = 0;
	PT_InputOpr ptTemp = g_ptInputOprHead;

	while (ptTemp)
	{
		printf("%02d : %s\n", i++, ptTemp->name);
		ptTemp = ptTemp->PT_Next;
	}
}

/*打开所有的输入设备*/
int AllInputDevicesInit(void)
{
	PT_InputOpr ptTemp = g_ptInputOprHead;
	int iError =-1;

	while (ptTemp)
	{
		if (ptTemp->DeviceInit() == 0)
		{
			iError = 0;
		}
		ptTemp = ptTemp->PT_Next;
	}
	return iError;
}


int GetInputEvent(PT_InputEvent ptInputEvent)
{
	/* 把链表中的InputOpr的GetInputEvent都调用一次,一旦有数据即返回 */
	PT_InputOpr ptTemp = g_ptInputOprHead;

	while (ptTemp)
	{
		if (ptTemp->GetInputEvent(ptInputEvent) == 0)
		{
			return 0;
		}
		ptTemp = ptTemp->PT_Next;
	}
	return -1;
}

int InputInit(void)
{
	int iError;

	iError = StdinInit();
	if (iError)
	{
		DBG_PRINTF("StdinInit error \n");
		return -1;
	}

	iError = TouchScreenInit();
	if (iError)
	{
		DBG_PRINTF("TouchScreenInit \n");
		return -1;
	}

	return 0;
}


