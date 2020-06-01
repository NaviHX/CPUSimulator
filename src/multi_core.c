#include "multi_core.h"

void initCPU(cpu *core, short id, char *filename, short address)
{
    //循环变量
    int i;
    //初始化相关信息
    core->filename = filename;
    core->id = id;
    core->addressStart = address;
    //将各寄存器归零
    core->markerRegister = 0;
    core->orderRegister = 0;
    core->PC = address;
    for (i = 0; i < 8; i++)
        core->gRegister[i] = 0;
}

unsigned __stdcall run(void *corePtr)
{
    int stop = 1; //保存程序运行状态，是否停止
    short imme;   //用于存储立即数
    cpu *core = (cpu *)corePtr;
    FILE *fin = fopen(core->filename, "r"); //打开存储命令的文件
    if (fin)
    {
        loadCommand(memory + core->addressStart, fin); //载入指令
        while (stop)
        {
            imme = getCommand(core);                                //获取指令
            stop = analyseCommand(core, core->orderRegister, imme); //运行指令
            WaitForSingleObject(outputLock, INFINITE);              //等待输出解锁
            printRegisterState(core);                               //打印寄存器状态
            ReleaseMutex(outputLock);                               //输出解锁
        }
    }
    else
        printf("CORE %d : Load Error\n", core->id);
    fclose(fin);
    _endthreadex(0);
    return 0;
}