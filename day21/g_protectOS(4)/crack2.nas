[INSTRSET "i486p"]
[BITS 32]
        MOV     EAX,1*8         ;操作系统所在段号
        MOV     DS,AX           ;程序段存入DS
        MOV     BYTE [0x102600],0
        RETF
        