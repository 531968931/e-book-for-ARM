#include <config.h>
#include <disp_manager.h>
#include <string.h>  /*strcmp函数需要*/

static PT_DispOpr g_ptDispOprHead;

int RegisterDispOpr(PT_DispOpr ptDispOpr)
{
	PT_DispOpr ptTemp;
	if (!g_ptDispOprHead)
	{		
		g_ptDispOprHead = ptDispOpr;
		ptDispOpr->PT_Next = NULL;
	}
	else
	{
		ptTemp = g_ptDispOprHead;
		while (ptTemp->PT_Next)
		{
			ptTemp = ptTemp->PT_Next;
		}
		ptTemp->PT_Next    = ptDispOpr;
		ptDispOpr->PT_Next = NULL;		
	}
	return 0;
}


void ShowDispOpr(void)
{
	int i = 0;
	PT_DispOpr ptTemp = g_ptDispOprHead;

	while (ptTemp)
	{
		printf("%02d   %s\n", i++, ptTemp->name);
		ptTemp = ptTemp->PT_Next;
	}
}


PT_DispOpr GetDispOpr(char *pcName)
{
	PT_DispOpr ptTemp = g_ptDispOprHead;
	while (ptTemp)
	{
		if (strcmp(pcName, ptTemp->name) == 0)
		{
			return(ptTemp);
		}
		ptTemp = ptTemp->PT_Next;
	}
	return NULL;
}


int DisplayInit(void)
{
	int iError;
	
	iError = FbInit();
	if(iError)
	{ 
		DBG_PRINTF("FbInit error!\n");
		return -1;
	}

	return 0;
}

