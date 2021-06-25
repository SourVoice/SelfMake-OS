;os
;TAB=4
;����32λģʽǰ,Ӧ��16λģʽ�µļĴ�������BIOS
;BIOS-INFO,�ⲿ�����ݷŵ��ڴ�0x0ff0��,��Ϊ����ϵͳ������
CYLS    equ     0x0ff0
LEDS    equ     0x0ff1
VMODE   equ     0x0ff2
SCRNX   equ     0x0ff4              ;�ֱ����趨
SCRNY   equ     0x0ff6
VRAM    equ     0x0ff8              ;ͼ�񻺳�����ʼ��ַ

        ORG     0xc200              
        MOV     AL,0x13             
        MOV     AH,0x00
        INT     0x10
        MOV     WORD [SCRNX],320
        MOV     WORD [SCRNY],200
        MOV     DWORD [VRAM],0x000a0000
        ;BIOS��ȡ����״̬(ȡ�ü����ϸ���LEDָʾ�Ƶ�״̬)
        MOV     AH,0x02
        INT     0x16                ;���̵�BIOS���ú���https://stanislavs.org/helppc/int_16-2.html
        MOV     [LEDS],al           ;ȡ�ü��̵�ǰ״̬   AL = ASCII character or zero if special function key(0x16����ֵ)
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
        