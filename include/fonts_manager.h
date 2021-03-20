#ifndef _FONTS_MANAGER_H
#define _FONTS_MANAGER_H

typedef struct FontBitMap{
	int iXLeft;
	int iYTop;
	int iXMax;
	int iYMax;
	int iBpp;
	/* 对于单色位图，两行像素之间的跨度*/
	int iPitch;
	int iCurOriginX;
	int iCurOriginY;
	int iNextOriginX;
	int iNextOriginY;
	unsigned char *pucBuffer;
}T_FontBitMap, *PT_FontBitMap;

typedef struct FontOpr{
	char *name;
	int (*FontInit)(char *pcFontFile, unsigned int dwFontSize);
	int (*GetFontBitmap)(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
	struct FontOpr *PT_Next;
}T_FontOpr, *PT_FontOpr;

int RegisterFontOpr(PT_FontOpr ptFontOpr);
void ShowFontOpr(void);
int FontsInit(void);
PT_FontOpr GetFontOpr(char *pcName);
int FreeInit(void);
int AsciiInit(void);
int GBKInit(void);

#endif




