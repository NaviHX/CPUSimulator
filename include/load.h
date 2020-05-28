#ifndef __LOAD_H__
#define __LOAD_H__


#include "global.h"
/*----------载入内存----------*/
//读入一个字节
BYTE loadByte(FILE *fin);
//载入指令
int loadCommand(BYTE *memory, FILE *fin);

#endif
