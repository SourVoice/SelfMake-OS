[BITS 32]
        MOV     AL,'H'
        CALL    2*8:0x0B9C      ;�������ϵͳ���ڶ�,far-CALL(�ô���ַ��Ҫ�ڻ��(bootpack.h-bootpack.map)���ҵ�,����е�˳���Ӱ�캯����ַ)
        MOV     AL,'E'
        CALL    2*8:0x0B9C
        MOV     AL,'L'
        CALL    2*8:0x0B9C
        MOV     AL,'L'
        CALL    2*8:0x0B9C
        MOV     AL,'O'
        CALL    2*8:0x0B9C
        RETF                    ;far-call��������,������hlt

fin:
        HLT
        JMP     fin
