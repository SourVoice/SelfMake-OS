[BITS 32]
        MOV     AL,'A'
        CALL    2*8:0x0B9C      ;�������ϵͳ���ڶ�,far-CALL(�ô���ַ��Ҫ�ڻ��(bootpack.h-bootpack.map)���ҵ�,����е�˳���Ӱ�캯����ַ)
        RETF                    ;far-call��������,������hlt
fin:
        HLT
        JMP     fin
