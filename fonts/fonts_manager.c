#include <config.h>
#include <fonts_manager.h>
#include <string.h>




static PT_FontOpr g_ptFontOprHead = NULL;

int RegisterFontOpr(PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTemp;

	if(!g_ptFontOprHead)
	{
		g_ptFontOprHead = ptFontOpr;
		ptFontOpr->PT_Next = NULL;
	}
	else
	{
		ptTemp = g_ptFontOprHead;
		while(ptTemp->PT_Next)
		{
			ptTemp = ptTemp->PT_Next;
		}
		ptTemp->PT_Next = ptFontOpr;
		ptFontOpr->PT_Next = NULL;
	}
	
	return 0;
}

void ShowFontOpr(void)
{
	int i =0;
	PT_FontOpr ptTemp = g_ptFontOprHead;

	while (ptTemp)
	{
		printf("%02d : %s\n", i++, ptTemp->name);
		ptTemp = ptTemp->PT_Next;
	}
}


PT_FontOpr GetFontOpr(char *pcName)
{
	PT_FontOpr ptTemp = g_ptFontOprHead;

	while (ptTemp)
	{
		if(strcmp(pcName, ptTemp->name) == 0)
		{
			return ptTemp;
		}
		ptTemp = ptTemp->PT_Next;
	}
	
	return NULL;	
}

int FontsInit(void)
{
	int iError;
	iError = FreeInit();
	if(iError)
	{ 
		DBG_PRINTF("FreeTypeInit error!\n");
		return -1;
	}

	iError = AsciiInit();
	if(iError)
	{ 
		DBG_PRINTF("AsciiTypeInit error!\n");
		return -1;
	}
	iError = GBKInit();
	if(iError)
	{ 
		DBG_PRINTF("GBKTypeInit error!\n");
		return -1;
	}
	return 0;
}




