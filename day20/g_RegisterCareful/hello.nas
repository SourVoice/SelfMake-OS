[BITS 32]
        MOV     AL,'H'
        INT     0x40      ;asm_cons_putcharע��֤IDT��,ֱ��ʹ��INT�������ü���
        MOV     AL,'E'
        INT     0x40
        MOV     AL,'L'
        INT     0x40
        MOV     AL,'L'
        INT     0x40
        MOV     AL,'O'
        INT     0x40
        RETF                    ;far-call��������,������hlt

fin:
        HLT
        JMP     fin
