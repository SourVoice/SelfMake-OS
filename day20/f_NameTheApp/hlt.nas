[BITS 32]
        MOV     AL,'H'
        INT     0x40      ;asm_cons_putchar注册证IDT中,直接使用INT函数调用即可
        MOV     AL,'E'
        INT     0x40
        MOV     AL,'L'
        INT     0x40
        MOV     AL,'L'
        INT     0x40
        MOV     AL,'O'
        INT     0x40
        RETF                    ;far-call返回命令,不进行hlt

fin:
        HLT
        JMP     fin
