;�������ʵ����ײ�Ӳ���Ľ����߼�,��C���õײ�Ӳ���ӿ�ʵ���ϲ��߼�
;nask_func
;TAB = 4
[BITS 32]                           ;����32λģʽ�Ļ�������

GLOBAL      _io_hlt                ;�����а����ĺ�����       
GLOBAL      _write_mem8
[SECTION .text]

_io_hlt:                            ;void io_hlt(void),��������+����
        hlt
        RET                         ;����������־

_write_mem8:                        ;void write_mem8(int address,int data);
        MOV     ecx,[ESP+4]         ;esp+4��ŵ�ַ,����ecx
        MOV     AL,[ESP+8]          ;[esp+8]�д������,����al    
        MOV     [ecx],al            
        RET
