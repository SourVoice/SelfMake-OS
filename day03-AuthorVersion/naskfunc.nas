;汇编语言实现与底层硬件的交互逻辑,用C调用底层硬件接口实现上层逻辑
;nask_func
;TAB = 4
[FORMAT "WCOFF"]                        ;制作目标文件的格式
[BITS 32]                               ;制作32位模式的机器语言
[instrset "i486p"]                      ; for 486(using nask)
[FILE "naskfunc.asm"]                   ;源文件名

GLOBAL      _io_hlt                     ;程序中包含的函数名       
GLOBAL      _write_mem8
[SECTION .text]
_io_hlt:                                ;void io_hlt(void),函数声明+定义
        hlt
        RET                             ;函数结束标志
_write_mem8:                            ;write_meme8(int addresss, int data),C中的参数传入寄存器[esp+4],[esp+8]..                            
        mov     ecx,[esp+4]             ;取函数传入的参数存入ecx寄存器,ecx存入一个add
        mov     al,[esp+8]              ;al存入一个数据       
        mov     [ecx],al                ;al的数据放入内存[ecx]
        RET


