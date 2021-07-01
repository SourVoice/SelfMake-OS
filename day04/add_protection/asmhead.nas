;os
;TAB=4
;BIOS-INFO,�ⲿ�����ݷŵ��ڴ�0x0ff0��,��Ϊ����ϵͳ������

;���ݺ�����ڵ�ַ����32λģʽ,��TM��û������ģʽ�Ľ��룬�Ҷ�û����gcc����Ŀ���ļ�
BOTPAK	EQU		0x00280000		; ����bootpack
DSKCAC	EQU		0x00100000		; ���̻����λ��
DSKCAC0	EQU		0x00008000		; ���̻����λ�ã�ʵģʽ��
;BIOS�趨
CYLS    equ     0x0ff0
LEDS    equ     0x0ff1
VMODE   equ     0x0ff2
SCRNX   equ     0x0ff4              
SCRNY   equ     0x0ff6
VRAM    equ     0x0ff8              
;�ں�����
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
bootpack: