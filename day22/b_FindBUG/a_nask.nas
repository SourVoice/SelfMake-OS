[FORMAT "WCOFF"]                ;生成对象模式文件的指令
[INSTRSET "i486p"]              ;表示使用486兼容指令集
[BITS 32]                       ;生成32位模式机器语言
[FILE "a_nask.nas"]             ;源文件名信息
        GLOBAL  _api_putchar
        GLOBAL  _api_end
[SECTION .text]

_api_putchar:       ;void api_putchar(int c)
        MOV     EDX,1
        MOV     AL,[ESP+4]      ;c
        INT     0x40
        RET
_api_end:
        MOV     EDX,4
        INT     0x40