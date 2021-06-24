;os.asm程序,将调用BIOS加载第一扇区的部分独立
;将读盘读到的ipl加载进内存指定位置
;即:该程序装载ipl到指定扇区
;这个里面应该写具体的操作系统实现,但我们同样的将其独立开来
;                                       +---------------------------------+
;                                       |MBR装载区                         |
;                                       |   该区域写入load_os内容          |
;                                       |MBR装载区                         |
;                                       |       该区域写入load_os内容      |
;   BIOS只负责装载第一扇区,其余扇区需要从MBR写程序装载
;                                       +---------------------------------+
;                                                第二扇区(以及其他部分)
;                                       +---------------------------------+
;                                       |                                 | <=内核文件写入位置
;                                       |                                 |
;                                       +---------------------------------+
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
        