#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

typedef struct DispOpr{
	char *name;
	int  iXres;
	int  iYres;
	int  iBPP;
	int  (*DeviceInit)(void);
	int  (*ShowPixel)(int iPenX, int iPenY, unsigned int dwColor);
	int  (*CleanScreen)(unsigned int dwBackColor);
	struct DispOpr *PT_Next;
}T_DispOpr, *PT_DispOpr;

int RegisterDispOpr(PT_DispOpr ptDispOpr);
void ShowDispOpr(void);
PT_DispOpr GetDispOpr(char *pcName);
int DisplayInit(void);
int FbInit(void);



#endif




