;�������ʵ����ײ�Ӳ���Ľ����߼�,��C���õײ�Ӳ���ӿ�ʵ���ϲ��߼�
;nask_func
;TAB = 4

;�ṩ�ýӿ��ļ���Ϣ
[FORMAT "WCOFF"]                        ;����Ŀ���ļ��ĸ�ʽ
[bits 32]                               ;����32λģʽ�Ļ�������
[instrset "i486p"]                      ; for 486(using nask)
[FILE "naskfunc.nas"]                   ;Դ�ļ���

GLOBAL          _io_hlt                     ;�����а����ĺ�����       
GLOBAL          _io_cli,_io_sti,_io_stihlt
GLOBAL          _io_in8,_io_in16,_io_in32
GLOBAL          _io_out8,_io_out16,_io_out32
GLOBAL          _io_load_eflags,_io_store_eflags         
GLOBAL          _load_gdtr,_load_idtr
GLOBAL          _asm_inthandler21,_asm_inthandler27,_asm_inthandler2c
EXTERN          _inthandler21,_inthandler27,_inthandler2c       ;�жϴ������� ,extern��ʾ�����������ⲿ   
[SECTION .text]
_io_hlt:                                ;void io_hlt(void),��������+����
        hlt
        RET                             
_io_cli:                                ;�жϱ�־��Ϊ0
        cli
        RET
_io_sti:                                ;�жϱ�־��Ϊ1
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
        mov     eax,[esp+4]     ;espջ��ָ��
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
;�ж��źŴ�����(PIC)
_asm_inthandler21:                      ;ʹ��ջ�ṹ���Ĵ��������ж�ǰ״̬
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
        IRETD                           ;�жϳ�����ֻ��ʹ��IRETD����
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
 