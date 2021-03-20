#include <config.h>
#include <disp_manager.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <show.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>



/* .show_file [-s Size] [-d DispName] [-f freetype] [-h HZK] <text_file> */

int main(int argc, char **argv)
{
	int iError;
	int bList = 0;/*用来记录帮助参数*/
	char cChoose;/*用来记录翻页及退出参数*/
	

	/*初始化字体大小为16*/
	unsigned int dwFontSize =16;

	char acHZKFile[128];
	char acFreetypeFile[128];
	char acTextFile[128];
	char acDisplay[128];

	acHZKFile[0] = '\0';
	acFreetypeFile[0] = '\0';
	acTextFile[0] = '\0';
	

	strcpy(acDisplay, "fb");
	
	/*如果选项成功找到，返回选项字母；如果所有命令行选项都解析完毕，返回 -1*/
	
	while((iError= getopt(argc, argv, "ls:f:h:d:")) != -1)
	{
		switch (iError)	
		{
			case 'l':
			{
				bList = 1;
				break;
			}

			case 's':
			{
				/*将optrag指向的数据转换为10进制，然后赋给字体大小*/
				dwFontSize = strtoul(optarg, NULL, 0);
				break;
			}

			case 'f':
			{
				strncpy(acFreetypeFile, optarg, 128);
				acFreetypeFile[127] = '\0';
				break;
			}
	
			case 'h':
			{
				strncpy(acHZKFile, optarg, 128);
				acHZKFile[127] = '\0';
				break;
			}

			case 'd':
			{
				strncpy(acDisplay, optarg, 128);
				acDisplay[127] = '\0';
				break;
			}

			default:
			{
				printf("Usage: %s [-s Size] [-d DispName] [-f freetype] [-h HZK] <text_file>\n", argv[0]);
				printf("Usage: %s -l\n", argv[0]);
				return -1;
				break;
			}
		}
	}

	if (!bList && (optind >= argc))/*如果没有按照要求输入参数*/
	{
		printf("Usage: %s [-s Size] [-d DispName] [-f freetype] [-h HZK] <text_file>\n", argv[0]);
		printf("Usage: %s -l\n", argv[0]);
		return -1;
	}

	/*各结构体初始化，成功返回0*/
	iError = DisplayInit();
	if (iError)
	{
		printf("DisplayInit error\n");
		return -1;		
	}
  /*记住初始化的顺序不能乱，encoding需要以来font，所以奴能颠倒*/
	iError = FontsInit();
	if (iError)
	{
		printf("FontsInit error\n");
		return -1;		
	}
	
	iError = EncodInit();
	if (iError)
	{
		printf("EncodInit error\n");
		return -1;		
	}
	

	if (bList)/*输入参数只有-l，即显示所能支持选项*/
	{
		printf("supported display: \n");
		ShowDispOpr();

		printf("supported font: \n");
		ShowFontOpr();

		printf("supported encoding: \n");
		ShowEncodOpr();

		printf("Usage: %s [-s Size] [-d DispName] [-f freetype] [-h HZK] <text_file>\n", argv[0]);
		return 0;
	}

	strncpy(acTextFile, argv[optind], 128);
	acTextFile[127] = '\0';

	/*1.使用Encoding_manager.c来解析文件，并获得编码 
负责打开文件，并选择合适的编码，并且确定第一个显示位置*/
	iError = OpenTextFile(acTextFile);
	if (iError)
	{
		printf("OpenTextFile error\n");
		return -1;
	}
	/* 2.使用Font_maneger.c来处理编码，获得字体点阵*/
	iError = SetTextDetail(acHZKFile, acFreetypeFile, dwFontSize);
	if (iError)
	{
		printf("SetTextDetail error\n");
		return -1;
	}
	/* 3.使用display-maneger.c选择显示模式，并输出点阵
	根据传入的名字，初始化相应的显示设备*/
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	
	iError = SelectAndInitDisplay(acDisplay);
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	if (iError)
	{
		printf("SelectAndInitDisplay error\n");
		return -1;
	}

	/* 4.显示页面*/
	iError = ShowNextPage();
	if (iError)
	{
		printf("Error to show first page\n");
		return -1;
	}

	while (1)
	{
		printf ("Enter 'n' to show next page, 'u' to previous page,'q' to quit");

		do{
			cChoose = getchar();
		}while ((cChoose != 'n') && (cChoose != 'u') && (cChoose != 'q'));

		if (cChoose == 'n')
		{
			ShowNextPage();
		}
		else if (cChoose == 'u')
		{
			ShowPrePage();
		}	
		else
		{
			return 0;
		}
	}
	return 0;	
}
		









				
