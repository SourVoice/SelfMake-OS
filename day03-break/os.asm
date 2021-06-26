;os
;TAB=4
;BIOS-INFO,�ⲿ�����ݷŵ��ڴ�0x0ff0��,��Ϊ����ϵͳ������
CYLS    equ     0x0ff0
LEDS    equ     0x0ff1
VMODE   equ     0x0ff2
SCRNX   equ     0x0ff4              
SCRNY   equ     0x0ff6
VRAM    equ     0x0ff8              

        ORG     0xc200              
        MOV     AL,0x13             
        MOV     AH,0x00
        INT     0x10
        MOV     WORD [SCRNX],320
        MOV     WORD [SCRNY],200
        MOV     DWORD [VRAM],0x000a0000
        ;BIOS��ȡ����״̬(ȡ�ü����ϸ���LEDָʾ�Ƶ�״̬)
        MOV     AH,0x02
        INT     0x16                
        MOV     [LEDS],al           
    success:
        MOV     si,MSG
    output:
        MOV     al,[si]
        ADD     si,1
        cmp     al,0
        je      fin

        MOV     AH,0x0e        
        MOV     bx,15          
        INT     0x10           
        jmp     output
    fin:
        hlt
        jmp     fin
    MSG:
        DB      0x0a,0x0a
        DB      "testing..."
        DB      "Success "
        DB      0x0a
        