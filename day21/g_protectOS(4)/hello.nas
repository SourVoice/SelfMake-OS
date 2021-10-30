[INSTRSET "i486p"]
[BITS 32]
        MOV     ECX,msg         ;msg写入ECX
        MOV     EDX,1
putloop:
        MOV     AL,[CS:ECX]     
        CMP     AL,0            
        JE      fin
        INT     0x40            ;asm_cons_putchar注册证IDT中,直接使用INT函数调用即可
        ADD     ECX,1           ;+1
        JMP     putloop
fin:
        MOV     EDX,4
        INT     0x40
msg:
        DB      "hello",0