#include "command.h"
#include "global.h"
#include "load.h"
#include "multi_core.h"
#include "sio.h"

BYTE memory[dataOffset * 2 + 2]; //内存，多出两字节防止越界
HANDLE core1, core2;             //线程的句柄
HANDLE outputLock;               //防止输出混乱添加的互斥对象

int main()
{

#ifdef DEBUG

    freopen("log", "w", stdout);//测试所用输出

#endif

    //循环变量
    int i;
    //初始化内存中的票数
    short *ptr;
    ptr = (short *)&memory[dataOffset];
    *ptr = MAX_TICKETS;
    //申请核心需要的内存
    cpu *cpu1, *cpu2;
    cpu1 = malloc(sizeof(cpu));
    cpu2 = malloc(sizeof(cpu));
    //初始化核心
    initCPU(cpu1, 1, "dict1.dic", 0);
    initCPU(cpu2, 2, "dict2.dic", 256);
    //为了分辨输出而添加的锁
    outputLock = CreateMutex(NULL, FALSE, NULL);
    //建立线程
    core1 = _beginthreadex(NULL, 0, run, (void *)cpu1, 0, NULL);
    core2 = _beginthreadex(NULL, 0, run, (void *)cpu2, 0, NULL);
    WaitForSingleObject(core1, INFINITE);
    CloseHandle(core1);
    WaitForSingleObject(core2, INFINITE);
    CloseHandle(core2);
    printCode();
    printData();
    return 0;
}
