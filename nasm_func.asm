;�������ʵ����ײ�Ӳ���Ľ����߼�,��C���õײ�Ӳ���ӿ�ʵ���ϲ��߼�
;nask_func
;TAB = 4
[FORMAT "WCOFF"]                    ;����Ŀ���ļ��ĸ�ʽ
[BITS 32]                           ;����32λģʽ�Ļ�������
[FILE "nasm_func.asm"]              ;Դ�ļ���

GLOBAL      _io_hlts                ;�����а����ĺ�����       

[SECTION .text]
_io_hlt:                            ;void io_hlt(void),��������+����
        hlt
        RET                         ;����������־





