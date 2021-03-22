#include <input_manager.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

static int StdinDevInit(void);
static int StdinDevExit(void);
static int StdinGetInputEvent(PT_InputEvent ptInputEvent);


static T_InputOpr g_tStdinOpr = {
	.name = "stdin",
	.DeviceInit = StdinDevInit,
	.DeviceExit = StdinDevExit,
	.GetInputEvent = StdinGetInputEvent,
};


static int StdinDevInit(void)
{
	struct termios tTTYState;
	/*1.获得终端的状态*/
	tcgetattr(STDIN_FILENO, &tTTYState);
	/*2.关闭异步通知*/
	tTTYState.c_lflag &= ~ICANON;
	/*3.设置为有一个数据就立即返回*/
	tTTYState.c_cc[VMIN] = 1;
	
	/*4.设置终端属性*/
	tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);

	return 0;
}

static int StdinDevExit(void)
{
	struct termios tTTYState;
	/*1.获得终端的状态*/
	tcgetattr(STDIN_FILENO, &tTTYState);
	/*2.恢复异步通知*/
	tTTYState.c_lflag |= ICANON;
	
	/*4.设置终端属性*/
	tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);

	return 0;

}

static int StdinGetInputEvent(PT_InputEvent ptInputEvent)
{
	/*使用查询当时获得终端的输入，即有数据立即返回
		无数据也立即返回，程序会一直重复这个操作*/

	struct timeval tTime;/*用来设置select函数的时钟，如果为0则立即返回*/
	char c;
	fd_set tFDs;

	tTime.tv_sec = 0;
	tTime.tv_usec = 0;

	/* 初始化fd_set结构体的数据，然后select取出数据新的fd结构体
		，与原来置0的fd结构体进行比较判断是否有数据输入，然后处理数据*/
	FD_ZERO(&tFDs);
	FD_SET(STDIN_FILENO, &tFDs);

	select(STDIN_FILENO+1, &tFDs, NULL, NULL, &tTime);

	if (FD_ISSET(STDIN_FILENO, &tFDs))
	{
		/*处理数据*/
		ptInputEvent->iType = INPUT_TYPE_STDIN;
		gettimeofday(&ptInputEvent->tTime, NULL);

		c = fgetc(stdin);
		if (c == 'u')
		{
			ptInputEvent->iVal = INPUT_VALUE_UP;
		}
		else if (c == 'n')
		{
			ptInputEvent->iVal = INPUT_VALUE_DOWN;
		}
		else if (c == 'q')
		{
			ptInputEvent->iVal = INPUT_VALUE_EXIT;
		}
		else
		{
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		}
		return 0;

	}
	else
		return -1;

}


int StdinInit(void)
{
	return RegisterInputOpr(&g_tStdinOpr);
}






