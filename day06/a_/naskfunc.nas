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
GLOBAL          _boxfill8
GLOBAL          _load_gdtr
[SECTION .text]
_io_hlt:                                ;void io_hlt(void),��������+����
        hlt
        RET                             ;����������־
_io_cli:                                ;�жϱ�־��Ϊ0
        cli
        RET
_io_sti:                                ;�жϱ�־��Ϊ1
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
_io_load_eflags:                ;��ȡIF��־λ��ֵ���鷳,��Ҫ��eflags�Ĵ���ȫ��ѹ��ջ,���ҵ�IF��ֵ
        pushfd                  ;��eflagsȫ��ѹ��ջ
        pop     eax             ;����ֵΪeax
        RET
_io_store_eflags:
        mov     eax,[esp+4]     ;espջ��ָ��
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
