;os
;TAB=4
;BIOS-INFO,这部分内容放到内存0x0ff0处,作为操作系统的设置

;根据函数入口地址调节32位模式,这TM都没讲保护模式的进入，我都没法用gcc连接目标文件
BOTPAK	EQU		0x00280000		; 加载bootpack
DSKCAC	EQU		0x00100000		; 磁盘缓存的位置
DSKCAC0	EQU		0x00008000		; 磁盘缓存的位置（实模式）
;BIOS设定
CYLS    equ     0x0ff0
LEDS    equ     0x0ff1
VMODE   equ     0x0ff2
SCRNX   equ     0x0ff4              
SCRNY   equ     0x0ff6
VRAM    equ     0x0ff8              
;内核主体
        ORG     0xc200              
        MOV     AL,0x13             
        MOV     AH,0x00
        INT     0x10
        MOV     WORD [SCRNX],320
        MOV     WORD [SCRNY],200
        MOV     DWORD [VRAM],0x000a0000
        ;BIOS获取键盘状态(取得键盘上各种LED指示灯的状态)
        MOV     AH,0x02
        INT     0x16                
        MOV     [LEDS],al           
bootpack: