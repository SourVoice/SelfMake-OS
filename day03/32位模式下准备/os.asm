;os
;TAB=4
        ORG     0xc200              ;磁盘内容装载到内存,磁盘0x4200处 + 0x8000得到操作系统内核位置,
                                    ;这里是指内存的位置
        MOV     AL,0x13             ;BIOS调用显卡
        MOV     AH,0x00
        INT     0x10
    fin:
        hlt
        jmp     fin
