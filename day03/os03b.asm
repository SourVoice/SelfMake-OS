;��ʼд��װ������IPL��
;ȫ18����װ����
;hello-os
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
    ;����ѭ���Դ�
        MOV     AX,0x0820          
        MOV     ES,AX               
        MOV     CH,0                
        MOV     DH,0                
        MOV     CL,2                

        MOV     si,0                ;si��¼��������
    retry:
        MOV     AH,0x02             ;����
        MOV     AL,1                ;��������������
        MOV     BX,0                ;�����ַ�����ܶ���ʹ�ã�
                                    ;���ڴ��ַ,�����̶��������������ڴ��λ��
                                    ;��д��ʽΪ[ES,BX],ESΪһ���μĴ���,���ڳ�BX��λ��(BXΪ16λ�Ĵ���)
        MOV     DL,0x00             ;��������(ָ����������ȡ����)
        INT     0x13                ;���ô���BIOS���ú�������һ����λ��ʶ(CF)��AH
                                    ;û����AH�������д��������������AH,
        jnc     succeed
        add     si,1
        cmp     si,5
        jae     error               ;si>=5ֹͣ����

        MOV     AH,0x00
        MOV     DL,0X00
        INT     0X13                ;����������(AH=0x00,DL=0x00,0x13����),����λ
        jmp     retry               ;jump if carry����λ��תָ��(BIOS 0x13h��������CF�Ĵ���)

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
    ;�ж���ֹ������
        RESB    510-($-$$)          
        db      0x55,0xaa           
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    4600
        db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
        RESB    1469432


;�й�label:error
;����û�г��Թ����̴�������ô�����,����error����ҲӦ������si������bios ��0x10����ʾ����ӡ