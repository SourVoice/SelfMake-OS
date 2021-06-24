;os.asm程序,将调用BIOS加载第一扇区的部分独立
;将读盘读到的ipl加载进内存指定位置
;即:该程序装载ipl到指定扇区
;这个里面应该写具体的操作系统实现,但我们同样的将其独立开来
;                                       +---------------------------------+
;                                       |MBR装载区                         |
;                                       |   该区域写入load_os内容          |
;                                       |MBR装载区                         |
;                                       |       该区域写入load_os内容      |
;   BIOS只负责装载第一扇区,其余扇区需要从MBR写程序装载
;                                       +---------------------------------+
;                                                第二扇区(以及其他部分)
;                                       +---------------------------------+
;                                       |                                 | <=内核文件写入位置
;                                       |                                 |
;                                       +---------------------------------+
;os
;TAB=4
        ORG     0xc200              ;磁盘内容装载到内存,磁盘0x4200处 + 0x8000得到操作系统内核位置,
                                    ;这里是指内存的位置
        MOV     AL,0x13             ;BIOS调用显卡
        MOV     AH,0x00
        INT     0x10
    fin:
        hlt
        jmp     fin


; ;下面为测试代码        
; ;         作者：kaka
; ; 链接：https//zhuanlan.zhihu.com/p/100757731
; ; 来源：知乎
; ; 著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

; jmp  start

; %macro   pos_get 0                           ;读取光标位置
;          mov ah,03h
;          mov bh,0
;          int 10h
; %endmacro

; %macro win 6     ;建立窗口

;          mov ah,06h
;          mov al,%1
;          mov bh,%2
;          mov ch,%3
;          mov cl,%4
;          mov dh,%5
;          mov dl,%6
;          int 10h
; %endmacro



; %macro pos_curse 3            ;设置光标位置
;          mov ah,2
;          mov bh,%1
;          mov dh,%2
;          mov dl,%3
;          int 10h
; %endmacro

; %macro printstr  4
;        ;lea BP,welcome;ES:BP = 串地址  ;显示内存中的连续字符串(用指针)
;        mov cx,%1  ;字符数量
;        mov ah,13h
;        mov al,01h ;前移光标
;        mov bh,0   ;页
;        mov bl,%2  ;颜色属性16h
;        mov dh,%3  ;行
;        mov dl,%4  ;列
;        int 10h

; %endmacro


; %macro printbuf 0        ;显示内存中的连续字符串(不用指针)
;       mov si,0
;       mov cl,[keybuf+1]     ;cl为控制需要输出的字符数量
; again:
;       mov ah,0eh
;       mov al,[keybuf+2+si]
;       int 10h
;       inc si
;       loop again
; %endmacro


; officemsg db 'Welcome OFFICE Program,Press Esc to Exit'
; gamesmsg db 'Welcome GAMES Program,Press Esc to Exit'
; videomsg db 'Welcome VIDEO Program,Press Esc to Exit'
; networkmsg db 'Welcome NETWORK Program,Press Esc to Exit'

; tabinfo  db 'Please Input Tab Key to Select,Press Esc to Exit'

;          manu_1   db '1. OFFICE   '
;          manu_2   db '2. GAMES    '
;          manu_3   db '3. VIDEO    '
;          manu_4   db '4. NETWORK  '

; firstinfo db 'Now You Have Comed OS Kernal,Please Press Enter Key to Run'
; welcome db 'Welcome Jiang OS'
; datetime  db  '????/??/?? ??:??:??'
; dataend  db  '?'
; row      db 0


; start:
     
; mov  ax, cs     ;从MBR跳转到此内核之后,CS=c20h ,IP=0。也即程序从偏移地址为0的地方开始放置 
; mov  ds, ax     ;那么就无需指定ORG，只需要把DS,ES和CS指向同一段即可。 
; mov  es, ax     ;这三句话在ORG 100H调试内核的时候同样适用，因此调试完成之后正式启用也无需修改程序。 


; first:  call mainwin
;         call shwelcome
;         call showtime
;         mov ah,0
;         int 16h                        ;从键盘读字符 ah=扫描码 al=字符码
;         cmp   ah,1ch                   ;enter键
;         jz   firstenter
;         jmp  first

; submenu:
;          mov  bp, manu_1
;          mov  cx, manu_2-manu_1
;          printstr  cx,16h,6,32

;               mov  bp, manu_2
;          mov  cx, manu_3-manu_2
;          printstr  cx,16h,10,32

;             mov  bp, manu_3
;          mov  cx, manu_4-manu_3
;          printstr  cx,16h,14,32

;                   mov  bp, manu_4
;          mov  cx, firstinfo-manu_4
;          printstr  cx,16h,18,32

;          ret

; firstenter:

;          win 0,3eh,1,0,23,79           ;按 enter键后刷新窗口

;          mov bp, tabinfo
;          mov cx,  manu_1- tabinfo
;          printstr  cx,3eh,3,18

;          win 0,65h,6,32,6,42
;          win 0,65h,10,32,10,42
;          win 0,65h,14,32,14,42
;          win 0,65h,18,32,18,42

;          call submenu

;          pos_curse 0,6,32               ;光标设置在第1个菜单处

; tabselect:mov  ah,0
;          int  16h
;          cmp  ah,0fh                    ;TAB键
;          jz   inputtab

;          cmp   ah,1ch                   ;选择之后按下enter键
;          jz   nextenter

;          cmp   ah,01h                   ;ESC键
;          jz    first

;          jmp  firstenter                ;不选择就一直等待

; inputtab:pos_get                ;TAB键
;          mov [row],dh

;          add byte [row],4              ;挪到下一个菜单处
;          cmp byte [row],22
;          je  tabend
;          jmp noend
; tabend:  mov byte [row],6                     ;向下选择超界
; noend:   win 0,6dh,[row],32,[row],40
;          call submenu
;          pos_curse 0,[row],32
;          jmp tabselect

; nextenter:
;         pos_get                       ;定位什么地方按的ENTER
;         mov  [row],dh

;         CMP byte [row],6
;         je  office

;         CMP  byte [row],10
;         je  games

;         CMP  byte [row],14
;         je  video

;          CMP  byte [row],18
;         je  network

;         jmp  inputtab

; office: win 0,3eh,1,0,23,79

;          mov bp, officemsg
;          mov cx,  gamesmsg- officemsg
;          printstr  cx,3eh,3,22

;          mov  ah,0
;          int  16h
;          cmp  ah,01h                    ;ESC功能
;          jz   firstenter
;          jmp  office


; games:  win 0,3eh,1,0,23,79

;          mov bp, gamesmsg
;          mov cx,  videomsg- gamesmsg
;          printstr  cx,3eh,3,22

;          mov  ah,0
;          int  16h
;          cmp  ah,01h                    ;ESC功能
;          jz   firstenter
;          jmp  games

; video:   win 0,3eh,1,0,23,79

;          mov bp, videomsg
;          mov cx,  networkmsg- videomsg
;          printstr  cx,3eh,3,22

;          mov  ah,0
;          int  16h
;          cmp  ah,01h                    ;ESC功能
;          jz   firstenter
;          jmp  video

; network:win 0,3eh,1,0,23,79

;          mov bp, networkmsg
;          mov cx,  tabinfo- networkmsg
;          printstr  cx,3eh,3,22

;          mov  ah,0
;          int  16h
;          cmp  ah,01h                    ;ESC功能
;          jz   firstenter
;          jmp  network


; shwelcome:
;      mov bp,welcome
;      mov cx, datetime-welcome
;      printstr  cx,16h,0,30

;      mov bp, firstinfo
;      mov cx,  welcome- firstinfo
;      printstr  cx,3eh,5,10
;      ret

; mainwin:
;      win 0,3eh,0,0,24,79           ;定义窗口背景
;      win 0,1fh,0,0,0,79            ;第一行
;      win 0,1fh,24,0,24,79          ;最后一行
;       ret



; showtime:

;         mov ah,04H             ;DH:DL=月:日 01:02   ch:cl=世纪:年,20:19
;         int 1ah

;         mov  bh,ch
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;Ch的高4位to ascii
;         and  ch,0000_1111b
;         add  ch,30h            ;Ch的低4位to ascii

;         mov [datetime],bh
;         mov [datetime+1],ch

;         mov  bh,cl
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add bh,30h            ;Cl的高4位to ascii
;         and cl,0000_1111b
;         add cl,30h            ;Cl的低4位to ascii

;         mov [datetime+2],bh
;         mov [datetime+3],cl

;         mov  bh,dh
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;dh的高4位to ascii
;         and  dh,0000_1111b
;         add  dh,30h            ;dh的低4位to ascii

;         mov [datetime+5],bh
;         mov [datetime+6],dh

;         mov  bh,dl
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;dl的高4位to ascii
;         and  dl,0000_1111b
;         add  dl,30h            ;dl的低4位to ascii

;         mov [datetime+8],bh
;         mov [datetime+9],dl

;         mov ah,02H             ;CH:CL=时:分 DH=秒
;         int 1ah

;         mov  bh,ch
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;Ch的高4位to ascii
;         and  ch,0000_1111b
;         add  ch,30h            ;Ch的低4位to ascii

;         mov [datetime+11],bh
;         mov [datetime+12],ch

;         mov  bh,cl
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add bh,30h            ;Cl的高4位to ascii
;         and cl,0000_1111b
;         add cl,30h            ;Cl的低4位to ascii

;         mov [datetime+14],bh
;         mov [datetime+15],cl

;         mov  bh,dh
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;dh的高4位to ascii
;         and  dh,0000_1111b
;         add  dh,30h            ;dh的低4位to ascii

;         mov [datetime+17],bh
;         mov [datetime+18],dh


;         win 0,1dh,24,60,24,79     ;设置窗口颜色

;              mov bp,datetime
;      mov cx, dataend-datetime
;      printstr  cx,16h,24,60

;        ret