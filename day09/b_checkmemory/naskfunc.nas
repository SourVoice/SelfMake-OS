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
GLOBAL          _load_gdtr,_load_idtr
GLOBAL          _load_cr0,_store_cr0
GLOBAL          _asm_inthandler21,_asm_inthandler27,_asm_inthandler2c
EXTERN          _inthandler21,_inthandler27,_inthandler2c       ;中断处理程序 ,extern表示函数声明在外部   
[SECTION .text]
_io_hlt:                                ;void io_hlt(void),函数声明+定义
        hlt
        RET                             
_io_cli:                                ;中断标志置为0
        cli
        RET
_io_sti:                                ;中断标志置为1
        sti
        RET     
_io_stihlt:             ; void io_stihlt(void);
        sti
        hlt
        ret
_io_in8:
        mov     edx,[esp+4]
        mov     eax,0
        in      al,dx
        RET
_io_in16:
        mov     edx,[esp+4]
        mov     eax,0
        in      ax,dx
        RET
_io_in32:
        mov     edx,[esp+4]
        in      eax,dx
        RET
_io_out8:             
        mov     edx,[esp+4]
        mov     al,[esp+8]
        out     dx,al
        RET    
_io_out16:
        mov     edx,[esp+4]
        mov     eax,[esp+8]
        out     dx,ax
        RET
_io_out32:
        mov     edx,[esp+4]
        mov     eax,[esp+8]
        out     dx,EAX
        RET
_io_load_eflags:                
        pushfd                  
        pop     eax             
        RET
_io_store_eflags:
        mov     eax,[esp+4]     ;esp栈底指针
        push    eax
        popfd
        RET        
_load_gdtr:
        mov     ax,[esp+4]
        mov     [esp+6],ax
        LGDT    [esp+6]
        RET
_load_idtr:
        mov     ax,[esp+4]
        mov     [esp+6],ax
        LIDT    [esp+6]
        RET
_load_cr0:      ;int store_cr0(void)
        mov     eax,CR0
        ret
_store_cr0:     ;void store_cr0(int cr0)
        mov     eax,[esp+4]
        mov     CR0,EAX
        ret
;中断信号处理器(PIC)
_asm_inthandler21:                      ;使用栈结构将寄存器返回中断前状态
        push    ES
        push    DS
        PUSHAD
        mov     EAX,ESP
        push    EAX
        mov     ax,SS
        mov     ds,ax
        mov     es,ax
        call    _inthandler21
        pop     eax
        POPAD
        pop     ds
        pop     es
        IRETD                           ;中断程序中只能使用IRETD返回
_asm_inthandler27:                      
        push    ES
        push    DS
        PUSHAD
        mov     EAX,esp
        push    EAX
        mov     ax,SS
        mov     ds,ax
        mov     es,ax
        call    _inthandler27
        pop     eax
        POPAD
        pop     ds
        pop     es
        IRETD                           
 
 _asm_inthandler2c:                      
        push    ES
        push    DS
        PUSHAD
        mov     EAX,esp
        push    EAX
        mov     ax,SS
        mov     ds,ax
        mov     es,ax
        call    _inthandler2c
        pop     eax
        POPAD
        pop     ds
        pop     es
        IRETD                           
 