#ifndef _ENCODINGH_MANAGER_H
#define _ENCODINGH_MANAGER_H

#include <fonts_manager.h>
#include <disp_manager.h>


typedef struct EncodOpr{
	char *name;
	int iHeadLen;
	int (*GetCodeFrmBuf)(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);
	int (*isSupport)(unsigned char *pucBufHead);/*根据字体文件编码的表头判断是否所持支的点阵*/
	PT_FontOpr PT_FontOprSupportedHead;	
	struct EncodOpr *PT_Next;
}T_EncodOpr, *PT_EncodOpr;

int RegisterEncodOpr(PT_EncodOpr ptEncodOpr);
void ShowEncodOpr(void);
PT_DispOpr GetDispOpr(char *pcName);
PT_EncodOpr SelectEncodOprForFile(unsigned char *pucFileBufHead);
int AddFontOprForEncod(PT_EncodOpr ptEncodOpr, PT_FontOpr ptFontOpr);
int DelFontOprFrmEncod(PT_EncodOpr ptEncodOpr, PT_FontOpr ptFontOpr);

int EncodInit(void);
int AsciiEncodInit(void);
int Utf8EncodInit(void);
int Utf16leEncodInit(void);
int Utf16beEncodInit(void);


#endif





