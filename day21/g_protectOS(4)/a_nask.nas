[FORMAT "WCOFF"]                ;���ɶ���ģʽ�ļ���ָ��
[INSTRSET "i486p"]              ;��ʾʹ��486����ָ�
[BITS 32]                       ;����32λģʽ��������
[FILE "a_nask.nas"]             ;Դ�ļ�����Ϣ
        GLOBAL  _api_putchar
        GLOBAL  _api_end
[SECTION .text]

_api_putchar:       ;void api_putchar(int c)
        MOV     EDX,1
        MOV     AL,[ESP+4]      ;c
        INT     0x40
        RET
_api_end:
        MOV     EDX,4
        INT     0x40