;开始写入装载区（IPL）
;全18扇区装载区
;hello-os
;Tab = 4
    ORG     0x7c00              ;程序加载至0x7c00处(程序指该计算机boot程序)
    jmp     entry               ;进入程序
                                ;同    0s01.asm对比可知，0xeb,0x4e = jmp entry,两个地址对应
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
    ;读盘循环试错
        MOV     AX,0x0820          
        MOV     ES,AX               
        MOV     CH,0                
        MOV     DH,0                
        MOV     CL,2                

        MOV     si,0                ;si记录启动次数
    retry:
        MOV     AH,0x02             ;读盘
        MOV     AL,1                ;处理对象的扇区数
        MOV     BX,0                ;缓冲地址（仅能读盘使用）
                                    ;即内存地址,将软盘读到的数据载入内存的位置
                                    ;书写格式为[ES,BX],ES为一个段寄存器,用于乘BX的位数(BX为16位寄存器)
        MOV     DL,0x00             ;驱动器号(指定该驱动读取软盘)
        INT     0x13                ;调用磁盘BIOS，该函数返回一个进位标识(CF)到AH
                                    ;没错误AH＝０，有错误则错误号码存入AH,
        jnc     succeed
        add     si,1
        cmp     si,5
        jae     error               ;si>=5停止读盘

        MOV     AH,0x00
        MOV     DL,0X00
        INT     0X13                ;重置驱动器(AH=0x00,DL=0x00,0x13函数),物理复位
        jmp     retry               ;jump if carry，进位跳转指令(BIOS 0x13h函数返回CF寄存器)

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
        HLT
        jmp     fin
    error:
        DB      0x0a,0x0a,0x0a
        DB      "READ ERROR"
        jmp     fin
    succeed:
        MOV     si,MSG
        jmp    putloop
    MSG:
        DB      0x0a,0x0a
        DB      "READ SUCCEED"
        db      0x0a,0x0a
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


;有关label:error
;这里没有尝试过读盘错误是怎么个情况,但是error部分也应该送入si处调用bios 的0x10在显示器打印