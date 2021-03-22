#include <config.h>
#include <input_manager.h>
#include <string.h>
#include <sys/select.h>


static PT_InputOpr g_ptInputOprHead;
static fd_set g_tRFds;
static int g_iMaxFd = -1;

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

/*打开所有的输入设备,并记录其最大文件句柄*/
int AllInputDevicesInit(void)
{
	PT_InputOpr ptTemp = g_ptInputOprHead;
	int iError =-1;

	while (ptTemp)
	{
		if (ptTemp->DeviceInit() == 0)
		{
			FD_SET(ptTemp->iFD, &g_tRFds);
			if (g_iMaxFd < ptTemp->iFD)
				g_iMaxFd = ptTemp->iFD;
			iError = 0;
		}
		ptTemp = ptTemp->PT_Next;
	}
	
	return iError;
}


int GetInputEvent(PT_InputEvent ptInputEvent)
{
	/* 用select函数监测stdin,touchscreen,
	   有数据时再调用它们的GetInputEvent或获得具体事件 */
	PT_InputOpr ptTemp = g_ptInputOprHead;
	fd_set tRFds;
	int iRet;

	tRFds = g_tRFds;
	/*当所检测的两个文件有输入值时，返回值就大于0，然后我们再去取出值*/
	iRet = select(g_iMaxFd+1, &tRFds, NULL, NULL, NULL);
	if (iRet > 0)
	{
		while (ptTemp)
		{
			if (ptTemp->GetInputEvent(ptInputEvent) == 0)
			{
			return 0;
			}
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


