[INSTRSET "i486p"]
[BITS 32]
        MOV     EAX,1*8         ;����ϵͳ���ڶκ�
        MOV     DS,AX           ;����δ���DS
        MOV     BYTE [0x102600],0
        RETF
        