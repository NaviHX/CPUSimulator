#include "command.h"

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
    char *loc = (char *)malloc(sizeof(char) * 16);
    sprintf(loc, "%d", imme);
    HANDLE lock = CreateMutex(NULL, FALSE, loc);
    WaitForSingleObject(lock, INFINITE);
    CloseHandle(lock);
}

void RELEASE(cpu *core, short re1, short re2, short imme)
{
    char *loc = (char *)malloc(sizeof(char) * 16);
    sprintf(loc, "%d", imme);
    HANDLE lock = CreateMutex(NULL, FALSE, loc);
    ReleaseMutex(lock);
    CloseHandle(lock);
}

void SLEEP(cpu *core, short re1, short re2, short imme)
{
    Sleep(imme);
}
