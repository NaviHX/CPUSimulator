#ifndef __SIO_H__
#define __SIO_H__

#include "global.h"
/*----------io相关----------*/
//打印寄存器状态
void printRegisterState(cpu *core);
//输入
void input(cpu *core, short re1, short re2, short imme);
//输出
void output(cpu *core, short re1, short re2, short imme);
//打印代码段
void printCode();
//打印数据段
void printData();

#endif
