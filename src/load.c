#include "load.h"

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