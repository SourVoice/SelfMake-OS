[BITS 32]
        MOV     AL,'A'
        CALL    2*8:0x0B9C;�������ϵͳ���ڶ�,far-CALL
fin:
        HLT
        JMP     fin
