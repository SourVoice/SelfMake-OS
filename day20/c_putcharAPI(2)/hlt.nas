[BITS 32]
        MOV     AL,'A'
        CALL    2*8:0x0B9C;进入操作系统所在段,far-CALL(该处地址需要在汇编(bootpack.h-bootpack.map)中找到,汇编中的顺序会影响函数地址)
fin:
        HLT
        JMP     fin
