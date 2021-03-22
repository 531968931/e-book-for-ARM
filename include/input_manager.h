#ifndef _INPUT_MANAGER_H 
#define _INPUT_MANAGER_H

#include <sys/time.h>

#define INPUT_TYPE_STDIN          0
#define INPUT_TYPE_TOUCHSCREEN    1

#define INPUT_VALUE_UP            0
#define INPUT_VALUE_DOWN          1
#define INPUT_VALUE_EXIT          2
#define INPUT_VALUE_UNKNOWN       -1



typedef struct InputEvent {
	struct timeval tTime;
	int iType;
	int iVal;
}T_InputEvent, *PT_InputEvent;



typedef struct InputOpr {
	char *name;
	int (*DeviceInit)(void);
	int (*DeviceExit)(void);
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	struct InputOpr *PT_Next;
}T_InputOpr, *PT_InputOpr;

int InputInit(void);
int StdinInit(void);
int TouchScreenInit(void);

int AllInputDevicesInit(void);
int RegisterInputOpr(PT_InputOpr ptInputOpr);
void ShowInputOpr(void);
int GetInputEvent(PT_InputEvent ptInputEvent);

#endif


