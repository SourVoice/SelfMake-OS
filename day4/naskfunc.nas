;�������ʵ����ײ�Ӳ���Ľ����߼�,��C���õײ�Ӳ���ӿ�ʵ���ϲ��߼�
;nask_func
;TAB = 4

;�ṩ�ýӿ��ļ���Ϣ
[FORMAT "WCOFF"]                        ;����Ŀ���ļ��ĸ�ʽ
[bits 32]                               ;����32λģʽ�Ļ�������
[instrset "i486p"]                      ; for 486(using nask)
[FILE "naskfunc.nas"]                   ;Դ�ļ���

GLOBAL      _io_hlt                     ;�����а����ĺ�����       

[SECTION .text]
_io_hlt:                                ;void io_hlt(void),��������+����
        hlt
        RET                             ;����������־
