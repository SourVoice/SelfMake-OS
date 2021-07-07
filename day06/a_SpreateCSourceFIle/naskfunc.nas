;汇编语言实现与底层硬件的交互逻辑,用C调用底层硬件接口实现上层逻辑
;nask_func
;TAB = 4

;提供该接口文件信息
[FORMAT "WCOFF"]                        ;制作目标文件的格式
[bits 32]                               ;制作32位模式的机器语言
[instrset "i486p"]                      ; for 486(using nask)
[FILE "naskfunc.nas"]                   ;源文件名

GLOBAL          _io_hlt                     ;程序中包含的函数名       
GLOBAL          _io_cli,_io_sti,_io_stihlt
GLOBAL          _io_in8,_io_in16,_io_in32
GLOBAL          _io_out8,_io_out16,_io_out32
GLOBAL          _io_load_eflags,_io_store_eflags         
GLOBAL          _boxfill8
GLOBAL          _load_gdtr
[SECTION .text]
_io_hlt:                                ;void io_hlt(void),函数声明+定义
        hlt
        RET                             ;函数结束标志
_io_cli:                                ;中断标志置为0
        cli
        RET
_io_sti:                                ;中断标志置为1
        sti
        RET
_io_sti;
        sti
        RET
_io_stihlt:
        mov     edx,[esp+4]
        mov     eax,0
        in      al,dx
        RET
_io_in8:
        mov     edx,[esp+4]
        mov     eax,0
        in      al,dx
_io_in16:
        mov     edx,[esp+4]
        mov     eax,0
        in      ax,dx
        RET
_io_in32:
        mov     edx,[esp+4]
        mov     eax,0
_io_out8:             
        mov     edx,[esp+4]
        mov     eax,[esp+8]
        mov     dx,ax
        RET    
_io_out16:
        mov     edx,[esp+4]
        mov     eax,[esp+8]
        out     dx,ax
        RET
_io_out32:
        mov     edx,[esp+4]
        mov     eax,[esp+8]
        out     dx,ax
        RET
_io_load_eflags:                ;读取IF标志位的值较麻烦,需要将eflags寄存器全部压入栈,再找到IF的值
        pushfd                  ;将eflags全部压入栈
        pop     eax             ;返回值为eax
        RET
_io_store_eflags:
        mov     eax,[esp+4]     ;esp栈底指针
        push    eax
        popfd
        RET        
_load_gdtr:
        mov     ax,[esp+4]
        mov     [esp+6],ax
        lgdt    [esp+6]
        RET
_load_idtr:
        mov     ax,[esp+4]
        mov     [esp+6],ax
        lidt    [esp+6]
        RET
