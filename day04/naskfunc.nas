;汇编语言实现与底层硬件的交互逻辑,用C调用底层硬件接口实现上层逻辑
;nask_func
;TAB = 4

;提供该接口文件信息
[FORMAT "WCOFF"]                        ;制作目标文件的格式
[bits 32]                               ;制作32位模式的机器语言
[instrset "i486p"]                      ; for 486(using nask)
[FILE "naskfunc.nas"]                   ;源文件名

GLOBAL      _io_hlt                     ;程序中包含的函数名       

[SECTION .text]
_io_hlt:                                ;void io_hlt(void),函数声明+定义
        hlt
        RET                             ;函数结束标志
