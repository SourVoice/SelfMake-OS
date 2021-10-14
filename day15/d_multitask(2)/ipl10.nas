;os-loader
;Tab = 4
        CYLS    equ     10
        ORG     0x7c00              ;程序加载至0x7c00处(现在这里的ORG是一个相对的偏移量了,并不真正指物理地址)
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
        db      "FAT12   "             
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
        MOV     ES,AX               
        add     CL,1                
        cmp     CL,18              
        jbe     readloop      
        ;反面
        MOV     CL,1                
        ADD     DH,1                
        cmp     DH,2                
        jb      readloop            
        ;读取柱面
        MOV     DH,0                
        ADD     CH,1               
        cmp     CH,CYLS             
        jb      readloop
    success:
        DB      0x0a,0x0a
        db      "ENTER the OS"
        db      0x0a
        db      0
        ; begin to execute os from boot sector
        mov     [0xff0], ch         
        jmp     0xc200              
    error:
        MOV     si,MSG
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
        db      "LOAD ERROR"
        db      0x0a
        db      0
        ;判断终止启动区
        RESB    0x7dfe-$          
        db      0x55,0xaa           