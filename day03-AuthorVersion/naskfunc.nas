;�������ʵ����ײ�Ӳ���Ľ����߼�,��C���õײ�Ӳ���ӿ�ʵ���ϲ��߼�
;nask_func
;TAB = 4
[FORMAT "WCOFF"]                        ;����Ŀ���ļ��ĸ�ʽ
[BITS 32]                               ;����32λģʽ�Ļ�������
[instrset "i486p"]                      ; for 486(using nask)
[FILE "naskfunc.asm"]                   ;Դ�ļ���

GLOBAL      _io_hlt                     ;�����а����ĺ�����       
GLOBAL      _write_mem8
[SECTION .text]
_io_hlt:                                ;void io_hlt(void),��������+����
        hlt
        RET                             ;����������־
_write_mem8:                            ;write_meme8(int addresss, int data),C�еĲ�������Ĵ���[esp+4],[esp+8]..                            
        mov     ecx,[esp+4]             ;ȡ��������Ĳ�������ecx�Ĵ���,ecx����һ��add
        mov     al,[esp+8]              ;al����һ������       
        mov     [ecx],al                ;al�����ݷ����ڴ�[ecx]
        RET


