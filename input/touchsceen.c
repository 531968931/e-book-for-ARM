#include <config.h>
#include <input_manager.h>
#include <show.h>
#include <stdlib.h>
#include <tslib.h>



static int TouchScreenDevInit(void);
static int TouchScreenDevExit(void);
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent);


static T_InputOpr g_tTouchScreenOpr = {
	.name = "touchScreen",
	.DeviceInit = TouchScreenDevInit,
	.DeviceExit = TouchScreenDevExit,
	.GetInputEvent = TouchScreenGetInputEvent,
};

static struct tsdev *g_ptTSDev;
static int giXres;
static int giYres;

static int TouchScreenDevInit(void)
{
	char *pcTSName = NULL;
	/*首先由环境变量获得设备名字
	打开这个设备ts_open然后配置*/
	if ((pcTSName = getenv("TSLIB_TSDEVICE")) != NULL)
	{
		g_ptTSDev = ts_open(pcTSName, 1);
	}
	else
	{
		g_ptTSDev = ts_open("/dev/input/evnet0", 1);
	}

	if (!g_ptTSDev)
	{
		DBG_PRINTF("ts_oper failed\n");
		return -1;
	}

	if (ts_config(g_ptTSDev))
	{
		DBG_PRINTF("ts_config error\n");
		return -1;
	}

	/*获得LCD的分辨率，在draw.c实现这个函数，为了后面确定触摸屏的输入值*/
	if (GetDispResolution(&giXres, &giYres))
	{
		DBG_PRINTF("GetDisResolution error\n");
		return -1;
	}
		return 0;
}


static int TouchScreenDevExit(void)
{
	return 0;
}

static int isTureTouch(struct timeval *ptPretime, struct timeval *ptNowtime)
{
	int iPreMs;
	int iNowMs;

	iPreMs = ptPretime->tv_sec * 1000 + ptPretime->tv_usec / 1000;
	iNowMs = ptNowtime->tv_sec * 1000 + ptNowtime->tv_usec / 1000;

	return (iNowMs > iPreMs + 500);
}

static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	struct ts_sample tSamp;
	int iRet;

	static struct timeval tPreTime;
	tPreTime.tv_sec = 0;
	tPreTime.tv_usec = 0;
	/*先读数据，然后处理数据*/
	iRet =ts_read(g_ptTSDev, &tSamp, 1);

	if (iRet < 0)
	{
		return -1;
	}

	if (isTureTouch(&tPreTime, &tSamp.tv))
	{
		/* 如果此次触摸事件发生的时间, 距上次事件超过了500ms */
		tPreTime = tSamp.tv;
		ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
		ptInputEvent->tTime = tSamp.tv;

		if (tSamp.y < giYres/3)
		{
			ptInputEvent->iVal = INPUT_VALUE_UP;
		}
		else if (tSamp.y > 2*giYres/3)
		{
			ptInputEvent->iVal = INPUT_VALUE_DOWN;
		}
		else
		{
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		}
		return 0;
	}
	else
	{
		return -1;
	}
	return 0;		
}


int TouchScreenInit(void)
{
	return RegisterInputOpr(&g_tTouchScreenOpr);
}





