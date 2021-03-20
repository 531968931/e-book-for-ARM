#include <config.h>
#include <encoding_manager.h>
#include <string.h>


static int AsciiGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);
static int isAsciiCoding(unsigned char *pucBufHead);


static T_EncodOpr g_tAsciiEncodOpr = {
	.name = "ascii",
	.iHeadLen = 0,
	.isSupport = isAsciiCoding,
	.GetCodeFrmBuf = AsciiGetCodeFrmBuf,
};
static int isAsciiCoding(unsigned char *pucBufHead)
{
	const char aStrUtf8[]    = {0xEF, 0xBB, 0xBF, 0};
	const char aStrUtf16le[] = {0xFF, 0xFE, 0};
	const char aStrUtf16be[] = {0xFE, 0xFF, 0};

	if (strncmp((const char *)pucBufHead, aStrUtf8, 3) == 0)
	{
		return 0;
	}
	else if (strncmp((const char *)pucBufHead, aStrUtf16le, 2) == 0)
	{
		return 0;
	}
	else if (strncmp((const char *)pucBufHead, aStrUtf16be, 2) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static int AsciiGetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	unsigned char *pucBuf = pucBufStart;
	unsigned char c = *pucBuf;

	if ((pucBuf < pucBufEnd) && (c < (unsigned char)0x80 ))
	{
		/* 返回ASCII码*/
		*pdwCode = (unsigned int)c;
		return 1;
	}
	
	if (((pucBuf +1)  < pucBufEnd) && (c < (unsigned char)0x80 ))
	{
		/* 返回ASCII码*/
		*pdwCode = pucBuf[0] +(((unsigned int)pucBuf[1])<<8);
		return 2;
	}

	if (pucBuf < pucBufEnd)
	{
		/* 文件可能有损坏，但仍然返回一个码*/
		*pdwCode = (unsigned int)c;
		return 1;
	}

	else
	{
		/*文件处理完成,即pucBufStat >= pucBufEnd*/
		return 0;
	}

}


int AsciiEncodInit(void)
{	
	AddFontOprForEncod(&g_tAsciiEncodOpr, GetFontOpr("freetype"));
	AddFontOprForEncod(&g_tAsciiEncodOpr, GetFontOpr("ascii"));
	AddFontOprForEncod(&g_tAsciiEncodOpr, GetFontOpr("gbk"));
	return RegisterEncodOpr(&g_tAsciiEncodOpr);
}


