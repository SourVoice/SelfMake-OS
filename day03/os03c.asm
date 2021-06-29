;hello-os
;��ȡ����18��װ����
;Tab = 4
        ORG     0x7c00              ;���������0x7c00��(����ָ�ü����boot����)
        jmp     entry               ;�������
                                    ;ͬ    0s01.asm�Աȿ�֪��0xeb,0x4e = jmp entry,������ַ��Ӧ
;ģ��FAT12��ʽ���̣�ģ�����ڴ��������
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
;�������
    entry:
        MOV     AX,0                
        MOV     ss,AX
        MOV     sp,0x7c00           
        MOV     DS,AX
        MOV     es,AX
              
    ;����(ȫ2-18����)
        MOV     AX,0x0820           
        MOV     ES,AX               
        MOV     CH,0                
        MOV     DH,0                               
        MOV     CL,2                ;�ӵڶ�����д��IPL
    readloop:
        MOV     si,0
    retry:
        MOV     AH,0x02             
        MOV     AL,1                
        MOV     BX,0                
        MOV     DL,0x00             
        INT     0x13                
        jnc     next                ;�������޴����ȡ��һ����
        ;��������ȡ����
        add     si,1                ;����Ϊ������ѭ���ж�,ѭ��5�ζ�ȡ
        cmp     si,5                
        jae     error               
        MOV     AH,0x00             
        MOV     DL,0x00             
        INT     0x13                ;��������            
        jmp     retry               
    next:
        MOV     AX,ES               ;������ȡ�ɹ���,�ڴ��ַ�����0x200 == MOV ex,512
        add     ax,0x0020           ;ES*16 = 512, 0x0020 = 512/16
        MOV     ES,AX               ;����������ָ�������趨��һ�������Ķ��̷�Χ,ES���ܽ��в���,���Լ������AX����ƫ��
        add     CL,1                ;cl���ڼ�¼����
        cmp     CL,18               
        jbe     readloop            ;cl<=18�������ȡ(jump if below or equal)
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
        ;�ж���ֹ������
        RESB    510-($-$$)          
        db      0x55,0xaa           
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    4600
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    1469432
