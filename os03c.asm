;hello-os
;Tab = 4
        cyls equ 10
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
              
    ;读盘(全2-18扇区)
        MOV     AX,0x0820           
        MOV     ES,AX               
        MOV     CH,0                
        MOV     DH,0                               
        MOV     CL,2                ;从第二扇区写入IPL
    readloop:
        MOV     si,0
    retry:
        MOV     AH,0x02             
        MOV     AL,1                
        MOV     BX,0                
        MOV     DL,0x00             
        INT     0x13                
        jnc     next                ;该扇区无错误读取下一扇区
        ;该扇区读取错误
        add     si,1                ;下面为出错后的循环判断,循环5次读取
        cmp     si,5                
        jae     error               
        MOV     AH,0x00             
        MOV     DL,0x00             
        INT     0x13                ;重置驱动            
        jmp     retry               
    next:
        MOV     AX,ES               ;扇区读取成功后,内存地址向后移0x200 == MOV ex,512
        add     ax,0x0020           ;ES*16 = 512, 0x0020 = 512/16
        MOV     BX,AX               ;上面这三个指令用于设定下一个扇区的读盘范围
        add     CL,1                ;cl用于记录扇区
        cmp     CL,18               
        jbe     readloop            ;cl<=18则继续读取(jump if below or equal)
        ;正面读完
        MOV     CL,1                
        add     DH,1
        cmp     DH,2
        jb      readloop            ;DH<2开始写反面(jump if blow)
        MOV     DH,0
        add     CH,1                ;改用磁头1号,CH给1
        cmp     CH,cyls
        jb      readloop

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
    MSG:
        DB      0x0a,0x0a
        db      "hello world"
        db      0x0a
        db      0
    ;判断终止启动区
        RESB    510-($-$$)          
        db      0x55,0xaa           
    error:
        DB      0x0a,0x0a
        DB      "READ ERROR"
        jmp     fin
