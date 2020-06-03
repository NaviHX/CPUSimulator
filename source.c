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

typedef unsigned char BYTE;      //单字节模拟内存
BYTE memory[dataOffset * 2 + 2]; //内存，多出两字节防止越界
HANDLE core1, core2;             //线程的句柄
HANDLE outputLock;               //防止输出混乱添加的互斥对象
HANDLE memoryLock[maxOffset / 2];
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

#ifndef __MULTI_CORE_H__
#define __MULTI_CORE_H__

/*----------多线程相关----------*/
//线程函数
unsigned __stdcall run(void *core);
//初始化核心
void initCPU(cpu *core, short id, char *filename, short address);

#endif

#ifndef __SIO_H__
#define __SIO_H__

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

#ifndef __LOAD_H__
#define __LOAD_H__

/*----------载入内存----------*/
//读入一个字节
BYTE loadByte(FILE *fin);
//载入指令
int loadCommand(BYTE *memory, FILE *fin);

#endif

#ifndef __COMMAND_H__
#define __COMMAND_H__

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

int main()
{

#ifdef DEBUG

    freopen("log", "w", stdout); //测试所用输出

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
    //初始化内存锁
    for (i = 0; i < maxOffset / 2; i++)
        memoryLock[i] = CreateMutex(NULL, FALSE, NULL);
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
//打印寄存器状态
void printRegisterState(cpu *core)
{
    int i;
    printf("==== id = %d ====\n", core->id);
    printf("ip = %d\n", core->PC);
    printf("flag = %d\n", core->markerRegister);
    printf("ir = %d\n", core->orderRegister);
    for (i = 0; i < 8; i++)
    {
        printf("ax%d = %d", i + 1, core->gRegister[i]);
        if ((i + 1) % 4 == 0)
            printf("\n");
        else
            printf(" ");
    }
    printf("\n");
}
//输入
void input(cpu *core, short re1, short re2, short imme)
{
    short *ptr = getPtr(core, re1);
    printf("in:\n", core->id);
    scanf("%hd", ptr);
}
//输出
void output(cpu *core, short re1, short re2, short imme)
{
    short *ptr = getPtr(core, re1);
    WaitForSingleObject(outputLock, INFINITE);
    printf("\n### id = %d  out: %d ###\n\n", core->id, *ptr);
    ReleaseMutex(outputLock);
}

void printCode() //打印代码段
{
    int i;
    int line = 0, count = 0;
    BYTE *memPtr = memory;
    int *intPtr;
    BYTE temp[4];
    printf("\ncodeSegment :\n");
    for (line = 0; line < 16; line++)
    {
        for (count = 0; count < 8; count++)
        {
            for (i = 0; i < 4; i++)
                temp[i] = *(memPtr + 3 - i); //为了保证数字正确，这里需要将每一个字节的内容前后调换顺序
            intPtr = (int *)temp;
            printf("%d", *intPtr);
            if (count != 7)
                printf(" ");
            memPtr += 4;
        }
        printf("\n");
    }
}

void printData() //打印数据段
{
    int line = 0, count = 0;
    BYTE *memPtr = memory + dataOffset;
    short *shortPtr;
    short num;
    printf("\ndataSegment :\n");
    for (line = 0; line < 16; line++)
    {
        for (count = 0; count < 16; count++)
        {
            shortPtr = (short *)memPtr;
            num = *shortPtr;
            printf("%d", num);
            if (count != 15)
                printf(" ");
            memPtr += 2;
        }
        printf("\n");
    }
}
BYTE loadByte(FILE *fin) //载入1长度的字节内容，得到对应的十进制数
{
    int i;
    char temp;
    BYTE val = 0;
    for (i = 0; i < 8 && feof(fin) == 0; i++)
    {
        temp = fgetc(fin);
        if (temp == EOF) //命令中断
            return 0;
        else if (temp != '0' && temp != '1') //排除非法字符
            i--;
        else
            val = val * 2 + (temp - '0');
    }
    return val;
}

int loadCommand(BYTE *memory, FILE *fin) //载入指令到内存中去，返回使用的字节数top
{
    int top = 0;
    while (feof(fin) == 0)
        memory[top++] = loadByte(fin);
    return top;
}

//获取需要操作的寄存器的指针，为后面的运算减少判断
short *getPtr(cpu *core, short re)
{
    if (re == 0)
        return NULL;
    else if (re <= 4)
        return &(core->gRegister[re - 1]);
    else
        return (short *)&memory[core->gRegister[re - 1]];
}

//将指令放到指令寄存器中，返回立即数
int getCommand(cpu *core)
{
    short imme;
    BYTE order[2], num[2];
    order[0] = memory[core->PC + 1];
    order[1] = memory[core->PC];
    core->orderRegister = (short)*((short *)order);
    num[0] = memory[core->PC + 3];
    num[1] = memory[core->PC + 2];
    imme = (short)*((short *)num);
    return imme;
}
//分析并运行指令
int analyseCommand(cpu *core, short com, short imme)
{
    short re1 = (com >> 4) % 16, re2 = com % 16, oper = com >> 8; //分别存储命令中的两个寄存器地址和指令
    short flag = 0;                                               //判断PC是否+4
    switch (oper) //根据不同的指令码选择不同的函数
    {
    case 0:
        break;
    case 1:
        dataTrans(core, re1, re2, imme);
        break;
    case 2 ... 5:
        dataCalc(core, re1, re2, imme, oper);
        break;
    case 6 ... 8:
        logicCalc(core, re1, re2, imme, oper);
        break;
    case 9:
        dataCompare(core, re1, re2, imme);
        break;
    case 10:
        flag = GOTO(core, re1, re2, imme);
        break;
    case 11:
        input(core, re1, re2, imme);
        break;
    case 12:
        output(core, re1, re2, imme);
        break;
    case 13:
        LOCK(core, re1, re2, imme);
        break;
    case 14:
        RELEASE(core, re1, re2, imme);
        break;
    case 15:
        SLEEP(core, re1, re2, imme);
        break;
    }
    if (flag != 1) //在没有跳转的情况下,pc+4
        core->PC += 4;
    return oper;
}
//数据传送
void dataTrans(cpu *core, short re1, short re2, short imme)
{
    short temp1, temp2;
    short *ptr1, *ptr2;
    temp1 = core->gRegister[re1 - 1];
    temp2 = core->gRegister[re2 - 1];
    if (re2 == 0)
        core->gRegister[re1 - 1] = imme;
    else
    {
        if (re1 > 4)
        {
            ptr1 = (short *)&memory[temp1];
            if (re2 > 4)
            {
                ptr2 = (short *)&memory[temp2];
                *ptr1 = *ptr2;
            }
            else
                *ptr1 = temp2;
        }
        else
        {
            if (re2 > 4)
            {
                ptr2 = (short *)&memory[temp2];
                core->gRegister[re1 - 1] = *ptr2;
            }
            else
                core->gRegister[re1 - 1] = temp2;
        }
    }
}
//比较函数，返回指定的数据
short compare(short num1, short num2)
{
    if (num1 > num2)
        return 1;
    else if (num1 == num2)
        return 0;
    else if (num1 < num2)
        return -1;
}
//数据比较，操作标志寄存器
void dataCompare(cpu *core, short re1, short re2, short imme)
{
    short *temp1 = getPtr(core, re1), *temp2 = getPtr(core, re2);
    if (re1 != 0 && re2 != 0)
        core->markerRegister = compare(*temp1, *temp2);
    else
    {
        temp1 = (temp2 == NULL) ? temp1 : temp2;
        core->markerRegister = compare(*temp1, imme);
    }
}
//跳转
short GOTO(cpu *core, short re1, short re2, short imme)
{
    short flag = 0;
    if (re2 == 0)
        core->PC += imme, flag = 1;
    else if (re2 == 1 && core->markerRegister == 0)
        core->PC += imme, flag = 1;
    else if (re2 == 2 && core->markerRegister == 1)
        core->PC += imme, flag = 1;
    else if (re2 == 3 && core->markerRegister == -1)
        core->PC += imme, flag = 1;
    return flag;
}

void dataCalc(cpu *core, short re1, short re2, short imme, short order)
{
    short *ptr1 = getPtr(core, re1), *ptr2 = getPtr(core, re2), *temp = malloc(sizeof(short));
    *temp = imme;
    if (ptr2 == NULL)
        ptr2 = temp;
    switch (order)
    {
    case 2:
        *ptr1 = (*ptr1) + (*ptr2);
        break;
    case 3:
        *ptr1 = (*ptr1) - (*ptr2);
        break;
    case 4:
        *ptr1 = (*ptr1) * (*ptr2);
        break;
    case 5:
        *ptr1 = (*ptr1) / (*ptr2);
        break;
    }
    free(temp);
}

void logicCalc(cpu *core, short re1, short re2, short imme, short order)
{
    short *ptr1 = getPtr(core, re1), *ptr2 = getPtr(core, re2), *temp = malloc(sizeof(short));
    *temp = imme;
    switch (order)
    {
    case 6:
        *ptr1 = (*ptr1) && (*(ptr2 == NULL ? temp : ptr2));
        free(temp);
        break;
    case 7:
        *ptr1 = (*ptr1) || (*(ptr2 == NULL ? temp : ptr2));
        free(temp);
        break;
    case 8:
        free(temp); //下一步会改变指针值，所以先释放内存，防止泄露
        temp = (ptr1 == NULL ? ptr2 : ptr1);
        *temp = (!(*temp));
        break;
    }
}

void LOCK(cpu *core, short re1, short re2, short imme)
{
    WaitForSingleObject(memoryLock[imme - dataOffset],INFINITE);
}

void RELEASE(cpu *core, short re1, short re2, short imme)
{
    ReleaseMutex(memoryLock[imme - dataOffset]);
}

void SLEEP(cpu *core, short re1, short re2, short imme)
{
    Sleep(imme);
}
