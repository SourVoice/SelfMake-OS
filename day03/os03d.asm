;hello-os
;读取反面+正面10个柱面
;Tab = 4
        CYLS    equ     10
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
        MOV     CH,0                ;柱面标号从0开始 
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
        jnc     next                
        ;该扇区读取错误
        add     si,1                
        cmp     si,5               
        jae     error              
        MOV     AH,0x00            
        MOV     DL,0x00            
        INT     0x13                
        jmp     retry              
    next:
        MOV     AX,ES              
        add     ax,0x0020           
        MOV     BX,AX               
        add     CL,1                
        cmp     CL,18              
        jbe     readloop      
        ;反面
        MOV     CL,1                ;扇区重置
        ADD     DH,1                ;切换磁针
        cmp     DH,2                
        jb      readloop            ;DH<2,jump
        MOV     DH,0                ;磁针置0
        ADD     CH,1                
        cmp     CH,CYLS             ;CYLS宏10,10个柱面
        jb      readloop            ;CH < CYLS,jump
    success:
        MOV     si,MSG
        DB      "READ SUCCESS"
        DB      0x0a
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
        DB      0x0a,0x0a
        DB      "READ ERROR"
        jmp     fin
    MSG:
        DB      0x0a,0x0a
        db      "hello world"
        db      0x0a
        db      0
        ;判断终止启动区
        RESB    510-($-$$)          
        db      0x55,0xaa           
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    4600
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    1469432
