;开始写入装载区（IPL）
;第一扇区的程序装载IPL+读盘试错
;hello-os
;Tab = 4
        ORG     0x7c00              
        jmp     entry               
;模拟FAT12格式软盘，模拟其内存分区机制
        db      0x90
        db      "HELLOIPL"          
        dw      512                 
        db      1                   
        dw      1                   
        db      2                   
        dw      224                 
        dw      2880                
        db      0xf0                
        dw      9                   
        dw      18                  
        dw      2                   
        dd      0                   
        dd      2880                
        db      0,0,0x29            
        dd      0xffffffff          
        db      "HELLO-OS   "       
        db      "FAT12"             
        RESB    18                  
;程序核心
    entry:
        MOV     AX,0                    
        MOV     ss,AX
        MOV     sp,0x7c00            
        MOV     DS,AX
        MOV     es,AX
        MOV     SI,MSG          

        MOV     AX,0x0820           ;软盘数据装载到内存的地址
        MOV     ES,AX               ;地址给到ES(段寄存器，辅助地址),ES存储地址0x0820
        MOV     CH,0                ;柱面号，0
        MOV     DH,0                ;磁头号，0(正面磁头)
        MOV     CL,2                ;扇区号，2
        MOV     AH,0x02             ;读盘
        MOV     AL,1                ;处理对象的扇区数
        MOV     BX,0                ;缓冲地址（仅能读盘使用）
                                    ;即内存地址,将软盘读到的数据载入内存的位置
                                    ;书写格式为[ES,BX],ES为一个段寄存器,用于乘BX的位数(BX为16位寄存器)
        MOV     DL,0x00             ;驱动器号(指定该驱动读取软盘)
        INT     0x13                ;调用磁盘BIOS，该函数返回一个进位标识(CF)到AH
                                    ;没错误AH＝０，有错误则错误号码存入AH,
        jc      ERROR               ;jump if carry    这里判断CF
    putloop:
        MOV     al,[si]             
        add     si,1
        cmp     al,0            

        je      fin
        MOV     AH,0x0e         
        MOV     bx,15           
        INT     0x10            
        jmp     putloop
    fin:
        HLT                     ; 让CPU停止，等待指令
        jmp     fin
    MSG:
        DB      0x0a,0x0a
        DB      "hello world"
        DB      0x0a
        DB      0


    ;判断终止启动区
        RESB    510-($-$$)          
        db      0x55,0xaa           
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    4600
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    1469432
;读盘错误打印
    ERROR:
        DB      0x0a,0x0a
        DB      "XXXXXXXXXXX"
        DB      "READ  ERROR"
        DB      "XXXXXXXXXXX"
        DB      0x0a
        DB      0
        jmp     fin