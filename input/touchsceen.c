#include <config.h>
#include <input_manager.h>
#include <show.h>
#include <stdlib.h>
#include <tslib.h>
#include <pthread.h>



static int TouchScreenDevInit(void);
static int TouchScreenDevExit(void);
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent);


static T_InputOpr g_tTouchScreenOpr = {
	.name = "touchscreen",
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
		g_ptTSDev = ts_open(pcTSName, 0); /* 以阻塞方式打开 */
	}
	else
	{
		g_ptTSDev = ts_open("/dev/input/evnet0", 0);/* 以阻塞方式打开 */
	}

	if (!g_ptTSDev)
	{
		DBG_PRINTF("ts_open failed\n");
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



/*采用滑动模式*/
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	struct ts_sample tSamp;
	struct ts_sample tSampPressed;
	struct ts_sample tSampReleased;
			
	int iRet;
	int bStart = 0;
	int iDelta;

	/*先读数据，然后处理数据*/
	

	while (1)
	{
		iRet =ts_read(g_ptTSDev, &tSamp, 1); /* 如果无数据则休眠，停在这块 */
		if (iRet == 1)
		{
			/* 刚按下 */
				/* 记录刚开始压下的点 */
			if ((tSamp.pressure >0) && (bStart == 0))
			{
				tSampPressed = tSamp;
				bStart = 1;

			}
			/* 已松开 */
			if (tSamp.pressure <= 0)
			{
				tSampReleased = tSamp;

				/*处理数据*/
				if (!bStart)
				{
					return -1;
				}
				else
				{
					iDelta = tSampReleased.x - tSampPressed.x;
					ptInputEvent->tTime = tSampReleased.tv;
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					
					if (iDelta > giXres/5)
					{
						/* 翻到上一页 */
						ptInputEvent->iVal = INPUT_VALUE_UP;
					}
					else if (iDelta < 0 - giXres/5)
					{
						/* 翻到下一页 */
						ptInputEvent->iVal = INPUT_VALUE_DOWN;
					}
					else
					{
						ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
					}
					return 0;	
				}
			}			
		}
		else
		{
			return -1;
		}		
	}
	return 0;		
}


int TouchScreenInit(void)
{
	return RegisterInputOpr(&g_tTouchScreenOpr);
}





