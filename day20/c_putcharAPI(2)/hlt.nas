[BITS 32]
        MOV     AL,'A'
        CALL    2*8:0x0B9C;进入操作系统所在段,far-CALL
fin:
        HLT
        JMP     fin
