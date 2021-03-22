#include <config.h>
#include <input_manager.h>
#include <string.h>



static PT_InputOpr g_ptInputOprHead;
static T_InputEvent g_tInputEvent;

static pthread_mutex_t g_tMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tConVar = PTHREAD_COND_INITIALIZER;

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

/*线程运行函数的起始地址
	这个参数代表一个指针，这个指针指向的函数必须满足以下条件：
	1、函数参数只有一个为void指针，可以为任意数据类型的指针。
	2、函数的返回值必须是void指针，可以为返回任意数据类型的指针*/
static void *InputEventTreadFunction(void *pVoid)
{
	T_InputEvent tInputEvent;
	/* 定义函数指针 */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pVoid;

	/* 死循环，直到GetInputEvent返回值为0，表示有数据 */
	while (1)
	{		
		if ((GetInputEvent(&tInputEvent)) == 0)
		{
			/* 唤醒主线程, 把tInputEvent的值赋给一个全局变量 */
			/* 访问临界资源前，先获得互斥量 */
			pthread_mutex_lock(&g_tMutex);
			g_tInputEvent = tInputEvent;

			/*  唤醒主线程 */
			pthread_cond_signal(&g_tConVar);

			/* 释放互斥量 */
			pthread_mutex_unlock(&g_tMutex);
			
		}

	}

	return NULL;
}


/*打开所有的输入设备,并创建多个子线程*/
int AllInputDevicesInit(void)
{
	PT_InputOpr ptTemp = g_ptInputOprHead;
	int iError =-1;


	while (ptTemp)
	{
		if (ptTemp->DeviceInit() == 0)
		{	
			/* 创建子线程 */
			if (pthread_create(&ptTemp->tThreadID, NULL, InputEventTreadFunction, ptTemp->GetInputEvent))
			{
				DBG_PRINTF("pthread_create failed\n");
			}
			iError = 0;
		}
		ptTemp = ptTemp->PT_Next;
	}
	
	return iError;
}


int GetInputEvent(PT_InputEvent ptInputEvent)
{
	/* 首先休眠，然后被唤醒后返回数据 */
	/* 休眠 */
	pthread_mutex_lock(&g_tMutex);
	pthread_cond_wait(&g_tConVar, &g_tMutex);	

	/* 被唤醒后,返回数据 */
	*ptInputEvent = g_tInputEvent;
	pthread_mutex_unlock(&g_tMutex);
	return 0;	
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


