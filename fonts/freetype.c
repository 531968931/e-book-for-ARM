#include <config.h>
#include <fonts_manager.h>
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H


static int FreeFontInit(char *pcFontFile, unsigned int dwFontSize);
static int FreeGetBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap);

static T_FontOpr g_tFreeFontOpr = {
	.name  = "freetype",
	.FontInit = FreeFontInit,
	.GetFontBitmap = FreeGetBitmap,
};

static FT_Library g_tLibrary;
static FT_Face g_tFace;
static FT_GlyphSlot g_tSlot;

static int FreeFontInit(char *pcFontFile, unsigned int dwFontSize)
{
	int iError;
	/* 1.初始化*/
	iError = FT_Init_FreeType(&g_tLibrary);
	if (iError)
	{
		DBG_PRINTF("FT_Init_Freetype failed\n");
		return -1;
	}
	/* 2.加载字体*/
	iError = FT_New_Face(g_tLibrary, pcFontFile, 0, &g_tFace);
	if (iError)
	{
		DBG_PRINTF("FT_New_Face failed\n");
		return -1;
	}
	/* 3.设置像素大小*/
	iError = FT_Set_Pixel_Sizes(g_tFace, dwFontSize, 0);
	if (iError)
	{
		DBG_PRINTF("FT_Set_Pixel_Sizes failed\n");
		return -1;
	}
	return 0;
	
}


static int FreeGetBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap)
{	
	int iError;
	int iPenX = ptFontBitMap->iCurOriginX;
	int iPenY = ptFontBitMap->iCurOriginY;

	/* 1.根据编码值加载glyph, FT_LOAD_MONOCHROME单色位图*/
	iError = FT_Load_Char(g_tFace, dwCode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
	if (iError)
	{
		DBG_PRINTF("FT_Load_Char failed.\n");
		return -1;
	}
	
	g_tSlot = g_tFace->glyph;  /*取出字体的象形符号*/
	/* 给ptFontBitMap结构体赋值*/
	ptFontBitMap->iXLeft    = iPenX + g_tSlot->bitmap_left;
	ptFontBitMap->iYTop     = iPenY - g_tSlot->bitmap_top;
	ptFontBitMap->iXMax     = ptFontBitMap->iXLeft + g_tSlot->bitmap.width;
	ptFontBitMap->iYMax     = ptFontBitMap->iYTop  + g_tSlot->bitmap.rows;
	ptFontBitMap->iBpp      = 1;
	ptFontBitMap->iPitch    = g_tSlot->bitmap.pitch;
	ptFontBitMap->pucBuffer = g_tSlot->bitmap.buffer;
	
	ptFontBitMap->iNextOriginX = iPenX + g_tSlot->advance.x / 64;
	ptFontBitMap->iNextOriginY = iPenY;

	return 0;

}

int FreeInit(void)
{
	return RegisterFontOpr(&g_tFreeFontOpr);
}

