#include <config.h>
#include <disp_manager.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <show.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>



/*要想分页显示文件并控制，需要知道一下几个东西：
 	屏幕大小范围，字体大小，本页和下一页开始在文件中的位置等*/
typedef struct PageDesc {
	int iPage;
	unsigned char *pucLcdFirstPosAtFile;
	unsigned char *pucLcdNextPageFirstPosAtFile;
	struct PageDesc *ptPrePage; /*使用双向链表来表示一页*/
	struct PageDesc *ptNextPage;
}T_PageDesc, *PT_PageDesc;
static PT_PageDesc g_ptPages;
static PT_PageDesc g_ptCurPage;


static int g_iFdTextFile;
static unsigned char *g_pucTextFileMem;
static unsigned char *g_pucTextFileMemEnd;

static PT_DispOpr g_ptDispOpr;
static PT_EncodOpr g_ptEncodOprForFile;

static unsigned char *g_pucLcdFirstPosAtFile;
static unsigned char *g_pucLcdNextPosAtFile;

static int g_dwFontSize;



/*1.使用Encoding_manager.c来解析文件，并获得编码 
负责打开文件，并选择合适的编码，并且确定第一个显示位置*/
int OpenTextFile(char *pcFileName)
{	
	struct stat tStat;
	
	g_iFdTextFile = open(pcFileName, O_RDONLY);
	if(g_iFdTextFile < 0)
	{
		DBG_PRINTF("can't open text file %s\n", pcFileName);
		return -1;		
	}

	if (fstat(g_iFdTextFile, &tStat))
	{
		DBG_PRINTF("can't get fstat\n");
		return -1;
	}

	g_pucTextFileMem = (unsigned char *)mmap(NULL, tStat.st_size, PROT_READ, MAP_SHARED, g_iFdTextFile,0);
	if (g_pucTextFileMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap for text file\n");
		return -1;
	}
	
	g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size;

		/*以上操作后文件已成功打开，后面需要为文件选择合适的编码程序*/
	g_ptEncodOprForFile = SelectEncodOprForFile(g_pucTextFileMem);
  		/* SelectEncodOprForFile函数在Encod中定义，他会根据传入的头文件地
  		址找到对应的编码程序，如果支持他就返回1，选择函数将返回对应FontOpr结构体,否则返回空*/
	if (g_ptEncodOprForFile)
	{
		/*进而在这里确定Lcd的第一个位置*/
		g_pucLcdFirstPosAtFile = g_pucTextFileMem + g_ptEncodOprForFile->iHeadLen;
		return 0;
	}
	else
	{
		return -1;
	}
	
}


/* 2.使用Font_maneger.c来处理编码，获得字体点阵*/
int SetTextDetail(char *pcHZKfile, char *pcFileFreetype, unsigned int dwFontSize)
{
	/*根据传入的Econd结构体中的链接的Font字体点阵结构体中的名字确定点阵类型，
	然后根据调用Font_manager.c中的函数初始化点阵*/
	
	g_dwFontSize = dwFontSize;/*根据传进来的变量确定字体大小（全局）*/

	int iError = 0;
	int iRet = -1; /*控制返回值，如果未经赋值则返回错误*/

	PT_FontOpr ptFontOpr;

	ptFontOpr = g_ptEncodOprForFile->PT_FontOprSupportedHead;
	while (ptFontOpr)
	{
		if (strcmp(ptFontOpr->name, "ascii") == 0)
		{
			iError = ptFontOpr->FontInit(NULL, dwFontSize);
		}
		else if (strcmp(ptFontOpr->name, "gbk") == 0)
		{
			iError = ptFontOpr->FontInit(pcHZKfile, dwFontSize);
		}
		else
		{
			iError = ptFontOpr->FontInit(pcFileFreetype, dwFontSize);
		}
		/* FontInit函数如果执行成功则返回0*/
		DBG_PRINTF("%s, %d\n", ptFontOpr->name, iError);

		if (iError == 0)
		{
			iRet = 0;
		}
		else
		{	
			/*将这个点阵结构体的链接从本编码结构体中删除*/
			DelFontOprFrmEncod(g_ptEncodOprForFile, ptFontOpr);
		}
		ptFontOpr = ptFontOpr->PT_Next;
		
	}
	return iRet;
}

/* 3.使用display-maneger.c选择显示模式，并输出点阵
	根据传入的名字，初始化相应的显示设备*/
int SelectAndInitDisplay(char *pcName)
{
	int iError;
	g_ptDispOpr = GetDispOpr(pcName);/*如果没有找到则返回NULL*/
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	if (!g_ptDispOpr)
	{
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	iError = g_ptDispOpr->DeviceInit();/* 初始化显示设备，如果成功返回值为0*/
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return iError;	
	
}

/*要想分页显示文件，需要知道一下几个东西：
	1.屏幕大小范围，字体大小，开始显示的位置*/
/* 控制翻页<-实现逐页显示 <-逐行显示<-逐字显示*/

int IncScreenX(int iX)  //如果要显示位置的X值不够显示一个字体，
{	
	/*如果要显示位置的X值不够显示一个字体，则返回0*/
	if((iX + 1) < g_ptDispOpr->iXres)
		return (iX + 1);
	else
		return 0;
}

int IncScreenY(int iY)  //有疑问
{	
	/*如果要显示位置的Y值不够显示下一行（g_dwFontSize字体大小），则返回0*/
	if((iY + g_dwFontSize) < g_ptDispOpr->iYres)
		return (iY + g_dwFontSize);
	else
		return 0;
}
/* 通过判断一个字体是否会超出屏幕显示范围
	得到页满，行满，并且给出下一行的初始位置*/
int RelocateFontPos(PT_FontBitMap ptFontBitMap)
{
	int iScreenY;
	int iDeltaX;
	int iDeltaY;

	if (ptFontBitMap->iYMax > g_ptDispOpr->iYres)
	{
		/*已满页*/
		return -1;
	}

	/* 判断超出LCD最右边的情况*/
	if (ptFontBitMap->iXMax > g_ptDispOpr->iXres)
	{
		/*判断行数是否已最大*/
		iScreenY = IncScreenY(ptFontBitMap->iCurOriginY);
		if (iScreenY == 0)
		{	
			/*已满页*/
			return -1;
		}
		else
		{
			/*正常换行*/
		iDeltaX = 0 - ptFontBitMap->iCurOriginX;
		iDeltaY = iScreenY - ptFontBitMap->iCurOriginY;

		ptFontBitMap->iCurOriginX  += iDeltaX;
		ptFontBitMap->iCurOriginY  += iDeltaY;

		ptFontBitMap->iNextOriginX += iDeltaX;
		ptFontBitMap->iNextOriginY += iDeltaY;

		ptFontBitMap->iXLeft += iDeltaX;
		ptFontBitMap->iXMax  += iDeltaX;

		ptFontBitMap->iYTop  += iDeltaY;
		ptFontBitMap->iYMax  += iDeltaY;

		return 0;		
		}	
	}
	return 0;
}

/* 显示单个字体点阵,根据FontBitMap结构体中的数据描绘*/
int ShowOneFont(PT_FontBitMap ptFontBitMap)
{
	int x;
	int y;
	unsigned char ucByte = 0;
	int i = 0;
	int bit;
	
	/*首先根据像素值的不同进行分类,我们现在使用的都是单像素*/
	if (ptFontBitMap->iBpp == 1)
	{
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
		{	
			/*计算出每个字符占用的字节数*/
			i = (y - ptFontBitMap->iYTop) * ptFontBitMap->iPitch;
			for (x =ptFontBitMap->iXLeft,bit = 7; x < ptFontBitMap->iXMax; x++)
			{
				if (bit == 7)
				{
					ucByte = ptFontBitMap->pucBuffer[i++];
				}

				if (ucByte & (1<<bit))
				{
					g_ptDispOpr->ShowPixel(x, y, COLOR_FOREGROUND);
				}
				else
				{
						/* 使用背景色, 不用描画 */
				}
				bit--;
				if (bit == -1)
				{
					bit = 7;
				}
			}			
		}
	}
	else if (ptFontBitMap->iBpp == 8)
	{
		for (y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++)
			for (x = ptFontBitMap->iXLeft; x < ptFontBitMap->iXMax; x++)
			{
				if (ptFontBitMap->pucBuffer[i++])
					g_ptDispOpr->ShowPixel(x, y, COLOR_FOREGROUND);
			}
	}
	else
	{
		DBG_PRINTF("ShowFont error, can't support %d bpp\n", ptFontBitMap->iBpp);
		return -1;
	}
	return 0;
}


/* 显示一页文字*/
int ShowOnePage(unsigned char * pucTextFileMemCurPos)
{	
	int iError;
	int iLen;/*用来记录字符编码的大小*/
	unsigned char *pucBufStart;
	unsigned int dwCode;
	
	int iHasNotClrScreen = 1;/*清屏标志位*/
	int iHasGetCode = 0;/*是否获得编码标志位*/

	T_FontBitMap tFontBitMap;/*保存得到点阵数据*/
	PT_FontOpr ptFontOpr;

	
	pucBufStart = pucTextFileMemCurPos;/*记录传入的文件开始值*/
	tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = g_dwFontSize;

	
	while (1)
		{	
			/* 获得编码的大小,并且用dwCode保存编码值*/
			iLen = g_ptEncodOprForFile->GetCodeFrmBuf(pucBufStart, g_pucTextFileMemEnd, &dwCode);
			if (iLen == 0)
			{
				/* 文件结束*/
				if (iHasGetCode)
				{	
					/*正常读取结束*/
					return 0;
				}
				else
				{	
					/*为空文件返回错误*/
					return -1;
				}
			}
				iHasGetCode = 1;

				pucBufStart += iLen;/*这个字符的编码值已经完成，让它指向下一个字符的开头*/
					
				/*判断文件中的换行号 
				 *有些文本, \n\r两个一起才表示回车换行
				 * 碰到这种连续的\n\r, 只处理一次
				 */
				if (dwCode == '\n')
				{
					g_pucLcdNextPosAtFile = pucBufStart;

					/*回车换行*/
					tFontBitMap.iCurOriginX = 0;
					tFontBitMap.iCurOriginY = IncScreenY(tFontBitMap.iCurOriginY);
					if (tFontBitMap.iCurOriginY == 0)
					{
						/* 当前页面已用完*/
						return 0;
					}
					else
					{
						continue;/*跳出循环，继续执行*/
					}
				}
				else if (dwCode == '\r')
				{
					continue;
				}
				/*TAB键用一个空格代替*/
				else if (dwCode == 't')
				{
					dwCode = ' ';
				}

				DBG_PRINTF("dwCode = 0x%x\n", dwCode);

				/*根据前面取出的编码值得到支持它的点阵*/
				ptFontOpr = g_ptEncodOprForFile->PT_FontOprSupportedHead;
				while (ptFontOpr)
				{
					/*得到这个编码的点阵数据,成功返回0，失败返回-1*/
					iError = ptFontOpr->GetFontBitmap(dwCode, &tFontBitMap);
					if (iError == 0)
					{	
						/*判断是否满页，并且在需要时换行。满页返回-1，没满页返回0*/
						if (RelocateFontPos(&tFontBitMap))/*-1为真，0为假*/
						{
							return 0;
						}

						if (iHasNotClrScreen)
						{
							/*清屏*/
							g_ptDispOpr->CleanScreen(COLOR_BACKGROUND);
							iHasNotClrScreen = 0;						
						}
						/*显示一个字符，成功显示返回0，否则返回0*/
						if (ShowOneFont(&tFontBitMap))
						{
							return -1;
						}

						tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
						tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
						g_pucLcdNextPosAtFile = pucBufStart;

						/* 跳出while循环，去取出下一个编码来显示*/
						break;
						
					}
					/*此编码用这个点阵Font无法得到正确的点阵，使用下一个font进行尝试*/
					ptFontOpr = ptFontOpr->PT_Next;				
				}
								
	}
	return 0;			
}

/*保存页面双向链表*/
static void RecordPage(PT_PageDesc ptPageNew)
{
	PT_PageDesc ptPageTemp;

	if (!g_ptPages)
	{
		g_ptPages = ptPageNew;
	}
	else
	{
		ptPageTemp = g_ptPages;
		while (ptPageTemp->ptNextPage)
		{
			ptPageTemp = ptPageTemp->ptNextPage;
		}
		ptPageTemp->ptNextPage = ptPageNew;
		ptPageNew->ptPrePage = ptPageTemp;
	}
}


/*显示上一页*/
int ShowNextPage(void)
{			
	int iError;
	PT_PageDesc ptPage;
	unsigned char *pucTextFileMenCurPos;

	if (g_ptCurPage)/*当前页面存在*/
	{
		pucTextFileMenCurPos = g_ptCurPage->pucLcdNextPageFirstPosAtFile;
	}
	else /*文件还未被显示，位置在初始化位置*/
	{
		pucTextFileMenCurPos = g_pucLcdFirstPosAtFile;
	}
	iError  = ShowOnePage(pucTextFileMenCurPos);

	if (iError == 0)
	{
		/*当前页面及下一个页面都存在*/
		if (g_ptCurPage && g_ptCurPage->ptNextPage)
		{
			g_ptCurPage = g_ptCurPage->ptNextPage;
			return 0;
		}
		/*又不在的情况，则首先分配内存，然后加入双项链表中*/
		ptPage = malloc(sizeof(T_PageDesc));
		if (ptPage)
		{
			ptPage->pucLcdFirstPosAtFile  = pucTextFileMenCurPos;
			/*下一个显示位置已经在showonepage函数中得到了确定*/
			ptPage->pucLcdNextPageFirstPosAtFile = g_pucLcdNextPosAtFile;
			ptPage->ptPrePage = NULL;
			ptPage->ptNextPage = NULL;
			g_ptCurPage = ptPage;
			RecordPage(ptPage);
			return 0;
		}
		else
		{
			DBG_PRINTF("malloc for T_PageDesc failed\n");
			return -1;
		}
	}
	return iError;
}

/*显示前一页*/
int ShowPrePage(void)
{
	int iError;
	/*1.判断当前页面是否为空，或者前一页是否为空*/
	if (!g_ptCurPage || !g_ptCurPage->ptPrePage)
	{
		/*当前页面或者前一页为空，则返回-1*/
		return -1;
	}

	iError = ShowOnePage(g_ptCurPage->ptPrePage->pucLcdFirstPosAtFile);
	if(iError == 0)/*表示显示成功*/
	{
		g_ptCurPage = g_ptCurPage->ptPrePage;
	}
	return iError;

}


