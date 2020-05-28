#ifndef __MULTI_CORE_H__
#define __MULTI_CORE_H__

#include "global.h"
/*----------多线程相关----------*/
//线程函数
unsigned __stdcall run(void *core);
//初始化核心
void initCPU(cpu *core, short id, char *filename, short address);

#endif
