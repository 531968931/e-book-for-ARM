
#include <config.h>
#include <encoding_manager.h>
#include <string.h>


static int Utf16leGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);
static int isUtf16leCoding(unsigned char *pucBufHead);


static T_EncodOpr g_tUtf16leEncodOpr = {
	.name = "utf-16le",
	.iHeadLen = 2,
	.isSupport = isUtf16leCoding,
	.GetCodeFrmBuf = Utf16leGetCodeFrmBuf,
};
static int isUtf16leCoding(unsigned char *pucBufHead)
{
	const char aStrUtf16le[] = {0xFF, 0xFE, 0};

	if (strncmp((const char *)pucBufHead, aStrUtf16le, 2) == 0)
	{
		/* UTF-16 little endian */
		return 1;
	}
	else
	{
		return 0;
	}
}


static int Utf16leGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{	
	if (pucBufStart + 1 < pucBufEnd)
	{
		*pdwCode = (((unsigned int)pucBufStart[1]) <<8) + pucBufStart[0]; 
		return 2;
	}
	else
	{	
		/* 文件结束*/
		return 0;

	}
}


int Utf16leEncodInit(void)
{	
	AddFontOprForEncod(&g_tUtf16leEncodOpr, GetFontOpr("freetype"));
	AddFontOprForEncod(&g_tUtf16leEncodOpr, GetFontOpr("ascii"));
	return RegisterEncodOpr(&g_tUtf16leEncodOpr);
}








