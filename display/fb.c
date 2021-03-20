#include <config.h>
#include <disp_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>

/* 驱动程序
	1.分配一个结构体；
	2.设置结构体；
	3.注册*/

static int  FBDeviceInit(void);
static int  FBShowPixel(int iPenX, int iPenY, unsigned int dwColor);
static int  FBCleanScreen(unsigned int dwBackColor);


static int g_fd;
static struct fb_var_screeninfo g_tFbVar;
static struct fb_fix_screeninfo g_tFbFix;
static unsigned int g_dwScreenSize;
static unsigned char *g_pucFbMem;
static unsigned int g_wdLineWidth;
static unsigned int g_wdPixelWidth;


static T_DispOpr g_tFbOpr = {
		.name        = "fb",
		.DeviceInit  = FBDeviceInit,
		.ShowPixel   = FBShowPixel,
		.CleanScreen = FBCleanScreen,
};

/* 初始化显示设备*/
static int  FBDeviceInit(void)
{
DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);	
	int ret;
	/* 1.打开LCD*/
	g_fd = open(FB_DEVICE_NAME, O_RDWR);
	if (g_fd < 0)
	{
		DBG_PRINTF("can't open %s\n", FB_DEVICE_NAME);
	}

	/* 2.获得LCD的可变信息*/
	ret = ioctl(g_fd, FBIOGET_VSCREENINFO, &g_tFbVar);
	if (ret <0 ) 
	{
		DBG_PRINTF("can't get fb'var\n");
		return -1;
	}
	/* 3.获得LCD固定的屏幕信息*/
	ret = ioctl(g_fd, FBIOGET_FSCREENINFO, &g_tFbFix);
	if (ret <0 ) 
	{
		DBG_PRINTF("can't get fb'fix\n");
		return -1;
	}
	/*4.获得屏幕大小*/
	g_dwScreenSize = g_tFbVar.xres * g_tFbVar.yres * g_tFbVar.bits_per_pixel /8;
	/*5.映射内存*/
	g_pucFbMem = (unsigned char *)mmap(NULL , g_dwScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
	if (g_pucFbMem < 0)
	{ 
		DBG_PRINTF("can't mmap\n");
		return -1;
	}
   /* 6.对T_DispOpr结构体的里其他值进行赋值*/
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	g_tFbOpr.iXres = g_tFbVar.xres;
	g_tFbOpr.iYres = g_tFbVar.yres;
	g_tFbOpr.iBPP = g_tFbVar.bits_per_pixel;    /*像素*/

	g_wdLineWidth = g_tFbVar.xres * g_tFbVar.bits_per_pixel / 8 ;
	g_wdPixelWidth = g_tFbVar.bits_per_pixel / 8;

	return 0;
}

/* 显示字体*/
static int  FBShowPixel(int iPenX, int iPenY, unsigned int dwColor)
{
	unsigned char *pucFb;
	unsigned short *pwFb16bpp;
	unsigned int *pdwFb32bpp;
	unsigned short wColor16bpp; /*565*/
	int iRed;
	int iBule;
	int iGreen;

	if ((iPenX >= g_tFbVar.xres) || (iPenY >= g_tFbVar.yres))
	{
		DBG_PRINTF("out of region\n");
		return -1;
	}

	/* 1.计算出笔的位置*/
	pucFb = g_pucFbMem + iPenY * g_wdLineWidth + iPenX * g_wdPixelWidth;
	pwFb16bpp = (unsigned short *)pucFb;
	pdwFb32bpp = (unsigned int *)pucFb;

	/* 2.描绘，也就是将dwColor里的值赋给pen所在位置，即：pucFb，pwFb16bpp，pdwFb32bpp*/
	switch (g_tFbVar.bits_per_pixel)
		{
			case 8:
			{
				*pucFb = (unsigned char)dwColor;
				break;
			}
			case 16:  /*565*/
			{
				iRed   = (dwColor >> 16) & 0xff;
				iGreen = (dwColor >> 8) & 0xff;
				iBule  = (dwColor >> 0) & 0xff;
				wColor16bpp = ((iRed >> 3) << 11) | ((iGreen >> 2) << 5) | (iBule >>3);
				*pwFb16bpp = wColor16bpp;		
			}
			case 32:
			{
				*pdwFb32bpp = dwColor;
				break;
			}
			default:
			{
				DBG_PRINTF("can't support %d bpp\n", g_tFbVar.bits_per_pixel);
				return -1;
			}
		}

	return 0;
}

/* 清除屏幕*/
static int  FBCleanScreen(unsigned int dwBackColor)
{
	unsigned char *pucFb;
	unsigned short *pwFb16bpp;
	unsigned int *pdwFb32bpp;
	unsigned short wColor16bpp; /*565*/
	int iRed;
	int iBule;
	int iGreen;
	int i = 0;

	/* 1.清除时笔的位置就在地址映射的位置*/
	pucFb = g_pucFbMem;
	pwFb16bpp = (unsigned short *)pucFb;
	pdwFb32bpp = (unsigned int *)pucFb;

	/* 2.描绘，也就是将dwBackColor里的值赋给pen所在位置，即：pucFb，pwFb16bpp，pdwFb32bpp,
		注意不能越界*/
	switch (g_tFbVar.bits_per_pixel)
		{
			case 8:
			{	
				memset(g_pucFbMem, dwBackColor, g_dwScreenSize);
				break;
			}
			case 16:  /*565*/
			{
				iRed   = (dwBackColor >> 16) & 0xff;
				iGreen = (dwBackColor >> 8) & 0xff;
				iBule  = (dwBackColor >> 0) & 0xff;
				wColor16bpp = ((iRed >> 3) << 11) | ((iGreen >> 2) << 5) | (iBule >>3);
				while (i < g_dwScreenSize)
				{
					*pwFb16bpp = wColor16bpp;
					pwFb16bpp++;
					i += 2;
				}
				break;
			}
			case 32:
			{	
				while (i < g_dwScreenSize)
				{
					*pdwFb32bpp = dwBackColor;
					pdwFb32bpp++;
					i += 4;
				}
				break;
			}
			default:
			{
				DBG_PRINTF("can't support %d bpp\n", g_tFbVar.bits_per_pixel);
				return -1;
			}
		}

	return 0;

}

int FbInit(void)
{
	return RegisterDispOpr(&g_tFbOpr);
}


