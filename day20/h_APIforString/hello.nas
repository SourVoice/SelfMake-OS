[INSTRSET "i486p"]
[BITS 32]
        MOV     ECX,msg         ;msgд��ECX
putloop:
        MOV     AL,[CS:ECX]     
        CMP     AL,0            
        JE      fin
        INT     0x40            ;asm_cons_putcharע��֤IDT��,ֱ��ʹ��INT�������ü���
        ADD     ECX,1           ;+1
        JMP     putloop
fin:
        RETF                    ;far-call��������,������hlt
msg:
        DB      "hello",0