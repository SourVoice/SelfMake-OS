;os
;TAB=4
        ORG     0xc200              ;��������װ�ص��ڴ�,����0x4200�� + 0x8000�õ�����ϵͳ�ں�λ��,
                                    ;������ָ�ڴ��λ��
        MOV     AL,0x13             ;BIOS�����Կ�
        MOV     AH,0x00
        INT     0x10
    fin:
        hlt
        jmp     fin
