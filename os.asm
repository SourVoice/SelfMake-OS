;os.asm����,������BIOS���ص�һ�����Ĳ��ֶ���
;�����̶�����ipl���ؽ��ڴ�ָ��λ��
;��:�ó���װ��ipl��ָ������
;�������Ӧ��д����Ĳ���ϵͳʵ��,������ͬ���Ľ����������
;                                       +---------------------------------+
;                                       |MBRװ����                         |
;                                       |   ������д��load_os����          |
;                                       |MBRװ����                         |
;                                       |       ������д��load_os����      |
;   BIOSֻ����װ�ص�һ����,����������Ҫ��MBRд����װ��
;                                       +---------------------------------+
;                                                �ڶ�����(�Լ���������)
;                                       +---------------------------------+
;                                       |                                 | <=�ں��ļ�д��λ��
;                                       |                                 |
;                                       +---------------------------------+
;os
;TAB=4
        ORG     0xc200              ;��������װ�ص��ڴ�,����0x4200�� + 0x8000�õ�����ϵͳ�ں�λ��,
                                    ;������ָ�ڴ��λ��
        MOV     AL,0x13             ;BIOS�����Կ�
        MOV     AH,0x00
        INT     0x10
    fin:
        hlt
        jmp     fin


; ;����Ϊ���Դ���        
; ;         ���ߣ�kaka
; ; ���ӣ�https//zhuanlan.zhihu.com/p/100757731
; ; ��Դ��֪��
; ; ����Ȩ���������С���ҵת������ϵ���߻����Ȩ������ҵת����ע��������

; jmp  start

; %macro   pos_get 0                           ;��ȡ���λ��
;          mov ah,03h
;          mov bh,0
;          int 10h
; %endmacro

; %macro win 6     ;��������

;          mov ah,06h
;          mov al,%1
;          mov bh,%2
;          mov ch,%3
;          mov cl,%4
;          mov dh,%5
;          mov dl,%6
;          int 10h
; %endmacro



; %macro pos_curse 3            ;���ù��λ��
;          mov ah,2
;          mov bh,%1
;          mov dh,%2
;          mov dl,%3
;          int 10h
; %endmacro

; %macro printstr  4
;        ;lea BP,welcome;ES:BP = ����ַ  ;��ʾ�ڴ��е������ַ���(��ָ��)
;        mov cx,%1  ;�ַ�����
;        mov ah,13h
;        mov al,01h ;ǰ�ƹ��
;        mov bh,0   ;ҳ
;        mov bl,%2  ;��ɫ����16h
;        mov dh,%3  ;��
;        mov dl,%4  ;��
;        int 10h

; %endmacro


; %macro printbuf 0        ;��ʾ�ڴ��е������ַ���(����ָ��)
;       mov si,0
;       mov cl,[keybuf+1]     ;clΪ������Ҫ������ַ�����
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
     
; mov  ax, cs     ;��MBR��ת�����ں�֮��,CS=c20h ,IP=0��Ҳ�������ƫ�Ƶ�ַΪ0�ĵط���ʼ���� 
; mov  ds, ax     ;��ô������ָ��ORG��ֻ��Ҫ��DS,ES��CSָ��ͬһ�μ��ɡ� 
; mov  es, ax     ;�����仰��ORG 100H�����ں˵�ʱ��ͬ�����ã���˵������֮����ʽ����Ҳ�����޸ĳ��� 


; first:  call mainwin
;         call shwelcome
;         call showtime
;         mov ah,0
;         int 16h                        ;�Ӽ��̶��ַ� ah=ɨ���� al=�ַ���
;         cmp   ah,1ch                   ;enter��
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

;          win 0,3eh,1,0,23,79           ;�� enter����ˢ�´���

;          mov bp, tabinfo
;          mov cx,  manu_1- tabinfo
;          printstr  cx,3eh,3,18

;          win 0,65h,6,32,6,42
;          win 0,65h,10,32,10,42
;          win 0,65h,14,32,14,42
;          win 0,65h,18,32,18,42

;          call submenu

;          pos_curse 0,6,32               ;��������ڵ�1���˵���

; tabselect:mov  ah,0
;          int  16h
;          cmp  ah,0fh                    ;TAB��
;          jz   inputtab

;          cmp   ah,1ch                   ;ѡ��֮����enter��
;          jz   nextenter

;          cmp   ah,01h                   ;ESC��
;          jz    first

;          jmp  firstenter                ;��ѡ���һֱ�ȴ�

; inputtab:pos_get                ;TAB��
;          mov [row],dh

;          add byte [row],4              ;Ų����һ���˵���
;          cmp byte [row],22
;          je  tabend
;          jmp noend
; tabend:  mov byte [row],6                     ;����ѡ�񳬽�
; noend:   win 0,6dh,[row],32,[row],40
;          call submenu
;          pos_curse 0,[row],32
;          jmp tabselect

; nextenter:
;         pos_get                       ;��λʲô�ط�����ENTER
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
;          cmp  ah,01h                    ;ESC����
;          jz   firstenter
;          jmp  office


; games:  win 0,3eh,1,0,23,79

;          mov bp, gamesmsg
;          mov cx,  videomsg- gamesmsg
;          printstr  cx,3eh,3,22

;          mov  ah,0
;          int  16h
;          cmp  ah,01h                    ;ESC����
;          jz   firstenter
;          jmp  games

; video:   win 0,3eh,1,0,23,79

;          mov bp, videomsg
;          mov cx,  networkmsg- videomsg
;          printstr  cx,3eh,3,22

;          mov  ah,0
;          int  16h
;          cmp  ah,01h                    ;ESC����
;          jz   firstenter
;          jmp  video

; network:win 0,3eh,1,0,23,79

;          mov bp, networkmsg
;          mov cx,  tabinfo- networkmsg
;          printstr  cx,3eh,3,22

;          mov  ah,0
;          int  16h
;          cmp  ah,01h                    ;ESC����
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
;      win 0,3eh,0,0,24,79           ;���崰�ڱ���
;      win 0,1fh,0,0,0,79            ;��һ��
;      win 0,1fh,24,0,24,79          ;���һ��
;       ret



; showtime:

;         mov ah,04H             ;DH:DL=��:�� 01:02   ch:cl=����:��,20:19
;         int 1ah

;         mov  bh,ch
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;Ch�ĸ�4λto ascii
;         and  ch,0000_1111b
;         add  ch,30h            ;Ch�ĵ�4λto ascii

;         mov [datetime],bh
;         mov [datetime+1],ch

;         mov  bh,cl
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add bh,30h            ;Cl�ĸ�4λto ascii
;         and cl,0000_1111b
;         add cl,30h            ;Cl�ĵ�4λto ascii

;         mov [datetime+2],bh
;         mov [datetime+3],cl

;         mov  bh,dh
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;dh�ĸ�4λto ascii
;         and  dh,0000_1111b
;         add  dh,30h            ;dh�ĵ�4λto ascii

;         mov [datetime+5],bh
;         mov [datetime+6],dh

;         mov  bh,dl
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;dl�ĸ�4λto ascii
;         and  dl,0000_1111b
;         add  dl,30h            ;dl�ĵ�4λto ascii

;         mov [datetime+8],bh
;         mov [datetime+9],dl

;         mov ah,02H             ;CH:CL=ʱ:�� DH=��
;         int 1ah

;         mov  bh,ch
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;Ch�ĸ�4λto ascii
;         and  ch,0000_1111b
;         add  ch,30h            ;Ch�ĵ�4λto ascii

;         mov [datetime+11],bh
;         mov [datetime+12],ch

;         mov  bh,cl
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add bh,30h            ;Cl�ĸ�4λto ascii
;         and cl,0000_1111b
;         add cl,30h            ;Cl�ĵ�4λto ascii

;         mov [datetime+14],bh
;         mov [datetime+15],cl

;         mov  bh,dh
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         SHR  bh,1
;         add  bh,30h            ;dh�ĸ�4λto ascii
;         and  dh,0000_1111b
;         add  dh,30h            ;dh�ĵ�4λto ascii

;         mov [datetime+17],bh
;         mov [datetime+18],dh


;         win 0,1dh,24,60,24,79     ;���ô�����ɫ

;              mov bp,datetime
;      mov cx, dataend-datetime
;      printstr  cx,16h,24,60

;        ret