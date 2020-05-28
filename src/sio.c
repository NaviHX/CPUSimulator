#include "sio.h"

//打印寄存器状态
void printRegisterState(cpu *core)
{
    int i;
    printf("### CORE : %d ###\n", core->id);
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
    printf("out: %d\n", *ptr);
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