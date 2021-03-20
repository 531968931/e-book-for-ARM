
#include <config.h>
#include <encoding_manager.h>
#include <string.h>


static int Utf16beGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);
static int isUtf16beCoding(unsigned char *pucBufHead);


static T_EncodOpr g_tUtf16beEncodOpr = {
	.name = "utf-16be",
	.iHeadLen = 2,
	.isSupport = isUtf16beCoding,
	.GetCodeFrmBuf = Utf16beGetCodeFrmBuf,
};
static int isUtf16beCoding(unsigned char *pucBufHead)
{
	const char aStrUtf16be[] = {0xFF, 0xFE, 0};

	if (strncmp((const char *)pucBufHead, aStrUtf16be, 2) == 0)
	{
		/* UTF-16 little endian */
		return 1;
	}
	else
	{
		return 0;
	}
}


static int Utf16beGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{	
	if (pucBufStart + 1 < pucBufEnd)
	{
		*pdwCode = (((unsigned int)pucBufStart[0]) <<8) + pucBufStart[1]; 
		return 2;
	}
	else
	{	
		/* 文件结束*/
		return 0;

	}
}


int Utf16beEncodInit(void)
{	
	AddFontOprForEncod(&g_tUtf16beEncodOpr, GetFontOpr("freetype"));
	AddFontOprForEncod(&g_tUtf16beEncodOpr, GetFontOpr("ascii"));
	return RegisterEncodOpr(&g_tUtf16beEncodOpr);
}









