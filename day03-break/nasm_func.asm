;汇编语言实现与底层硬件的交互逻辑,用C调用底层硬件接口实现上层逻辑
;nask_func
;TAB = 4
[BITS 32]                           ;制作32位模式的机器语言

GLOBAL      _io_hlt                ;程序中包含的函数名       
GLOBAL      _write_mem8
[SECTION .text]

_io_hlt:                            ;void io_hlt(void),函数声明+定义
        hlt
        RET                         ;函数结束标志

_write_mem8:                        ;void write_mem8(int address,int data);
        MOV     ecx,[ESP+4]         ;esp+4存放地址,读入ecx
        MOV     AL,[ESP+8]          ;[esp+8]中存放数据,读入al    
        MOV     [ecx],al            
        RET
