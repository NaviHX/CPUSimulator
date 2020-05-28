#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "global.h"
/*----------指令相关----------*/
short *getPtr(cpu *core, short re);
//将指令放到指令寄存器中，返回立即数
int getCommand(cpu *core);
//分析并运行指令
int analyseCommand(cpu *core, short com, short imme);
//数据传送
void dataTrans(cpu *core, short re1, short re2, short imme);
//数值计算
void dataCalc(cpu *core, short re1, short re2, short imme, short order);
//逻辑运算
void logicCalc(cpu *core, short re1, short re2, short imme, short order);
//数值比较
short compare(short num1, short num2);
void dataCompare(cpu *core, short re1, short re2, short imme);
//跳转
short GOTO(cpu *core, short re1, short re2, short imme);
//锁定内存
void LOCK(cpu *core, short re1, short re2, short imme);
//解除锁定
void RELEASE(cpu *core, short re1, short re2, short imme);
//休眠
void SLEEP(cpu *core, short re1, short re2, short imme);

#endif