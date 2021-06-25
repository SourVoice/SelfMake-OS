;os
;TAB=4
;进入32位模式前,应用16位模式下的寄存器调度BIOS
;BIOS-INFO,这部分内容放到内存0x0ff0处,作为操作系统的设置
CYLS    equ     0x0ff0
LEDS    equ     0x0ff1
VMODE   equ     0x0ff2
SCRNX   equ     0x0ff4              ;分辨率设定
SCRNY   equ     0x0ff6
VRAM    equ     0x0ff8              ;图像缓冲区开始地址

        ORG     0xc200              
        MOV     AL,0x13             
        MOV     AH,0x00
        INT     0x10
        MOV     WORD [SCRNX],320
        MOV     WORD [SCRNY],200
        MOV     DWORD [VRAM],0x000a0000
        ;BIOS获取键盘状态(取得键盘上各种LED指示灯的状态)
        MOV     AH,0x02
        INT     0x16                ;键盘的BIOS调用函数https://stanislavs.org/helppc/int_16-2.html
        MOV     [LEDS],al           ;取得键盘当前状态   AL = ASCII character or zero if special function key(0x16返回值)
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
        