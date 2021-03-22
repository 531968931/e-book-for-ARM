#ifndef _SHOW_H
#define _SHOW_H
int OpenTextFile(char *pcFileName);
int SetTextDetail(char *pcHZKfile, char *pcFileFreetype, unsigned int dwFontSize);
int SelectAndInitDisplay(char *pcName);
int GetDispResolution(int *piXres, int *piYres);
int ShowNextPage(void);
int ShowPrePage(void);

#endif





