[INSTRSET "i486p"]
[BITS 32]
        MOV     ECX,msg         ;msgд��ECX
        MOV     EDX,1
putloop:
        MOV     AL,[CS:ECX]     
        CMP     AL,0            
        JE      fin
        INT     0x40            ;asm_cons_putcharע��֤IDT��,ֱ��ʹ��INT�������ü���
        ADD     ECX,1           ;+1
        JMP     putloop
fin:
        MOV     EDX,4
        INT     0x40
msg:
        DB      "hello",0