#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#define codeOffset 0     //代码段偏移
#define dataOffset 16384 //数据段偏移
#define maxOffset 32768  //内存最大值

#ifdef DEBUG

#define MAX_TICKETS 10 //测试最大票数

#else

#define MAX_TICKETS 100 //最大票数

#endif

typedef unsigned char BYTE;             //单字节模拟内存
extern BYTE memory[dataOffset * 2 + 2]; //内存，多出两字节防止越界
extern HANDLE core1, core2;             //线程的句柄
extern HANDLE outputLock;               //防止输出混乱添加的互斥对象
//cpu核心和寄存器结构体
typedef struct CPU_info
{
    short id;             //cpu id
    char *filename;       //读入文件名
    short addressStart;   //指令初始地址偏移
    short PC;             //指令寄存器
    short gRegister[8];   //通用寄存器
    short orderRegister;  //指令寄存器
    short markerRegister; //标志寄存器
} cpu;

#endif
