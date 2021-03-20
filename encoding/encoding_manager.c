#include <config.h>
#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>
#include <fonts_manager.h>




static PT_EncodOpr g_ptEncodOprHead;

int RegisterEncodOpr(PT_EncodOpr ptEncodOpr)
{
	PT_EncodOpr ptTemp;

	if(!g_ptEncodOprHead)
	{
		g_ptEncodOprHead = ptEncodOpr;
		ptEncodOpr->PT_Next = NULL;
	}
	else
	{
		ptTemp = g_ptEncodOprHead;
		while(ptTemp->PT_Next)
		{
			ptTemp = ptTemp->PT_Next;
		}
		ptTemp->PT_Next = ptEncodOpr;
		ptEncodOpr->PT_Next = NULL;
	}
	return 0;
}

void ShowEncodOpr(void)
{
	int i = 0;
	PT_EncodOpr ptTemp = g_ptEncodOprHead;

	while (ptTemp)
	{
		printf("%02d : %s\n", i++, ptTemp->name);
		ptTemp = ptTemp->PT_Next;
	}
}

PT_EncodOpr SelectEncodOprForFile(unsigned char *pucFileBufHead)
{	
	PT_EncodOpr ptTemp = g_ptEncodOprHead;

	while (ptTemp)
	{	/* 如果支持的话issupport返回1，这个在各编码文件中进行了定义*/
		if (ptTemp->isSupport(pucFileBufHead))
			return ptTemp;
		else 
			ptTemp = ptTemp->PT_Next;
	}
	return NULL;
}

int AddFontOprForEncod(PT_EncodOpr ptEncodOpr, PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptFontOprCpy;

	if (!ptEncodOpr || !ptFontOpr)/*传入的两个结构体中存在空结构体*/
	{
		return -1;
	}
	else
	{	
		/*malloc时需要得到这个体真实的大小，而不是指向结构体的指针的大小*/	
		ptFontOprCpy = malloc(sizeof(T_FontOpr));
		if (!ptFontOprCpy) /*防止malloc时这块地址已有数据*/
		{
			return -1;
		}
		else
		{	
			/*注意c语言中对数组和结构体进行赋值时，不能直接赋值，需要使用strcpy或 memcpy，
				这是因为C语言中数组名和指针名时没有区别的*/
			memcpy(ptFontOprCpy, ptFontOpr, sizeof(T_FontOpr));
			/*将这个加入表头*/
			ptFontOprCpy->PT_Next = ptEncodOpr->PT_FontOprSupportedHead;
			ptEncodOpr->PT_FontOprSupportedHead = ptFontOprCpy;
			return 0;
						
		}
	}

}

int DelFontOprFrmEncod(PT_EncodOpr ptEncodOpr, PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTemp;
	PT_FontOpr ptPre;

	if (!ptEncodOpr || !ptFontOpr)/*传入的两个结构体中存在空结构体*/
	{
		return -1;
	}
	else
	{	
	 	ptTemp = ptEncodOpr->PT_FontOprSupportedHead;
		if (strcmp(ptTemp->name, ptFontOpr->name) == 0)
		{
			/*需要删除的是头结点*/
			ptEncodOpr->PT_FontOprSupportedHead = ptTemp->PT_Next;
			free(ptTemp);
			return 0;
		}
		
		/*不是头结点，需要从链表中取出这个节点，释放掉，并且重建链表*/
		ptPre = ptEncodOpr->PT_FontOprSupportedHead;
		ptTemp = ptPre->PT_Next;
		while (ptTemp)
		{	
			if (strcmp(ptTemp->name, ptFontOpr->name) == 0)
			{	
				ptPre->PT_Next = ptTemp->PT_Next;/*取出Temp指向的链表元素*/
				free(ptTemp);
				return 0;
					
			}
			else
			{
				ptPre = ptTemp;
				ptTemp = ptTemp->PT_Next;		
			}
		}
		return -1;						
	}
}

int EncodInit(void)
{
	int iError;

	iError = AsciiEncodInit();
	if (iError)
	{
		DBG_PRINTF("AsciiEncodInit error \n");
		return -1;
	}

	iError = Utf8EncodInit();
	if (iError)
	{
		DBG_PRINTF("Utf8EncodInit error \n");
		return -1;
	}

	iError = Utf16leEncodInit();
	if (iError)
	{
		DBG_PRINTF("Utf16leEncodInit error \n");
		return -1;
	}

	iError = Utf16beEncodInit();
	if (iError)
	{
		DBG_PRINTF("Utf16beEncodInit error \n");
		return -1;
	}
	
	return 0;
}



