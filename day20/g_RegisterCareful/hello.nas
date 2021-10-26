[INSTRSET "i486p"]
[BITS 32]
        MOV     ECX,msg         ;msg写入ECX
putloop:
        MOV     AL,[CS:ECX]     
        CMP     AL,0            
        JE      fin
        INT     0x40            ;asm_cons_putchar注册证IDT中,直接使用INT函数调用即可
        ADD     ECX,1           ;+1
        JMP     putloop
fin:
        RETF                    ;far-call返回命令,不进行hlt
msg:
        DB      "hello",0