#include <config.h>
#include <encoding_manager.h>
#include <string.h>


static int Utf8GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode);
static int isUtf8Coding(unsigned char *pucBufHead);


static T_EncodOpr g_tUtf8EncodOpr = {
	.name = "utf-8",
	.iHeadLen = 3,
	.isSupport = isUtf8Coding,
	.GetCodeFrmBuf = Utf8GetCodeFrmBuf,
};
static int isUtf8Coding(unsigned char *pucBufHead)
{
	const char aStrUtf8[]    = {0xEF, 0xBB, 0xBF, 0};

	if (strncmp((const char *)pucBufHead, aStrUtf8, 3) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static int GetPreOneBits(unsigned char ucVal)
{
	int i;
	int j = 0;

	for (i = 7; i >= 0; i--)
	{
		if (ucVal & (1<<i))
			j++;
		else
			break;
	}
	return j;
}

static int Utf8GetCodeFrmBuf(unsigned char *pucBufStart, unsigned char *pucBufEnd, unsigned int *pdwCode)
{
	/*1.判断高位中1的个数，2.根据高位中1的个数确定编码值
	对于UTF-8编码中的任意字节B，如果B的第一位为0，则B为ASCII码，并且B独立的表示一个字符;
    如果B的第一位为1，第二位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的一个字节，并且不为字符的第一个字节编码;
    如果B的前两位为1，第三位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由两个字节表示;
    如果B的前三位为1，第四位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由三个字节表示;
    如果B的前四位为1，第五位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由四个字节表示;

    因此，对UTF-8编码中的任意字节，根据第一位，可判断是否为ASCII字符;
    根据前二位，可判断该字节是否为一个字符编码的第一个字节; 
    根据前四位（如果前两位均为1），可确定该字节为字符编码的第一个字节，并且可判断对应的字符由几个字节表示;
    根据前五位（如果前四位为1），可判断编码是否有错误或数据传输过程中是否有错误*/

	int i;
	int iOneNum;
	unsigned char ucVal;/*可以依次取出buf中的值*/
	unsigned int dwSum = 0; /*用来存放最终的code值*/
	
	if (pucBufStart >= pucBufEnd)
	{
		/* 文件结束*/
		return 0;
	}

	ucVal = pucBufStart[0];
	iOneNum = GetPreOneBits(pucBufStart[0]);

	if ((pucBufStart + iOneNum) > pucBufEnd)
	{
		/*文件结束*/
	 	return 0;
	}

	if (iOneNum == 0)
	{
		/*ASCII*/
		*pdwCode = pucBufStart[0];
		return 1;
	}
	else
	{
		ucVal = ucVal << iOneNum;
		ucVal = ucVal >> iOneNum; /*通过这两步得到第一部分有效值*/
		dwSum += ucVal;
		for(i = 1; i < iOneNum; i++)
		{
			ucVal = pucBufStart[i] & 0x3f;/*将高两位置0*/
			dwSum = dwSum << 6; /*每个字节前两位都是无效的*/
			dwSum += ucVal;
		}
		*pdwCode = dwSum;
		return iOneNum;
	}	

}


int Utf8EncodInit(void)
{	
	AddFontOprForEncod(&g_tUtf8EncodOpr, GetFontOpr("freetype"));
	AddFontOprForEncod(&g_tUtf8EncodOpr, GetFontOpr("ascii"));
	return RegisterEncodOpr(&g_tUtf8EncodOpr);
}







