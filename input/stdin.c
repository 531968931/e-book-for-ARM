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
	/*使用多线程*/

	char c;
	

	/* 不用做任何比较，线程会进行判断有无数据*/
	
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


int StdinInit(void)
{
	return RegisterInputOpr(&g_tStdinOpr);
}






