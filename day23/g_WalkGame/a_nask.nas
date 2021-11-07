[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]
        GLOBAL  _api_putchar
        GLOBAL  _api_end
        GLOBAL  _api_putstr0
        GLOBAL  _api_openwin        
        GLOBAL  _api_putstrwin
        GLOBAL  _api_boxfillwin
        GLOBAL  _api_initmalloc
        GLOBAL  _api_malloc
        GLOBAL  _api_free
        GLOBAL  _api_point
        GLOBAL  _api_linewin
        GLOBAL  _api_getkey
        GLOBAL  _api_refreshwin
        GLOBAL  _api_closewin
[SECTION .text]

_api_putchar:
        MOV     EDX,1
        MOV     AL,[ESP+4]
        INT     0x40
        RET

_api_putstr0:
        PUSH    EBX
        MOV     EDX,2
        MOV     EBX,[ESP+8]
        INT     0x40
        POP     EBX
        RET

_api_end:
        MOV     EDX,4
        INT     0x40

_api_openwin:
        PUSH    EDI
        PUSH    ESI
        PUSH    EBX
        MOV     EDX,5
        MOV     EBX,[ESP+16]    ;buf
        MOV     ESI,[ESP+20]    ;xsize               
        MOV     EDI,[ESP+24]    ;ysize                
        MOV     EAX,[ESP+28]    ;col_inv
        MOV     ECX,[ESP+32]    ;title             
        INT     0x40
        POP     EBX
        POP     ESI
        POP     EDI
        RET
        
_api_putstrwin:                 ;void api_putstrwin(int win, int x, int y, int col, int len, char* str)
        PUSh    EDI
        PUSH    ESI
        PUSH    EBP
        PUSH    EBX
        MOV	EDX,6
        MOV	EBX,[ESP+20]	; win
        MOV	ESI,[ESP+24]	; x
        MOV	EDI,[ESP+28]	; y
        MOV	EAX,[ESP+32]	; col
        MOV	ECX,[ESP+36]	; len
        MOV	EBP,[ESP+40]	; str
        INT	0x40
        POP	EBX
        POP	EBP
        POP	ESI
        POP	EDI
        RET

_api_boxfillwin:        ; void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
        PUSH	EDI
        PUSH	ESI
        PUSH	EBP
        PUSH	EBX
        MOV	EDX,7
        MOV	EBX,[ESP+20]	; win
        MOV	EAX,[ESP+24]	; x0
        MOV	ECX,[ESP+28]	; y0
        MOV	ESI,[ESP+32]	; x1
        MOV	EDI,[ESP+36]	; y1
        MOV	EBP,[ESP+40]	; col
        INT	0x40
        POP	EBX
        POP	EBP
        POP	ESI
        POP	EDI
        RET 

_api_initmalloc:
        PUSH    EBX
        MOV     EDX,8
        MOV     EBX,[CS:0x0020]         ;malloc内存空间地址
        MOV     EAX,EBX
        ADD     EAX,32*1024             ;空间分配32kb
        MOV     ECX,[CS:0x0000]         ;数据段大小
        SUB     ECX,EAX
        INT     0x40
        POP     EBX
        RET

_api_malloc:    ;char *api_malloc(int size)
        PUSh    EBX
        MOV     EDX,9
        MOV     EBX,[CS:0x0020]
        MOV     ECX,[ESP+8]
        INT     0x40
        POP     EBX
        RET

_api_free:      ;void api_free(char*addr,int size)
        PUSh    EBX
        MOV     EDX,10
        MOV     EBX,[CS:0x0020]
        MOV     EAX,[ESP+8]             ;addr
        MOV     ECX,[ESP+12]            ;size
        INT     0x40
        POP     EBX
        RET

_api_point:
        PUSH    EDI
        PUSH    ESI
        PUSH    EBX
        MOV     EDX,11
        MOV     EBX,[ESP+16]    ;win
        MOV     ESI,[ESP+20]    ;x
        MOV     EDI,[ESP+24]    ;y
        MOV     EAX,[ESP+28]    ;col
        INT     0x40
        POP     EBX
        POP     ESI
        POP     EDI
        RET

_api_linewin:           ;void api_lienwin(int win,int x0,int y0,int x1,int y1,int col);
        PUSH    EDI
        PUSh    ESI
        PUSH    EBP
        PUSH    EBX
        MOV     EDX,13
        MOV     EBX,[ESP+20]    ;win    
        MOV     EAX,[ESP+24]    ;x0
        MOV     ECX,[ESP+28]    ;y0
        MOV     ESI,[ESP+32]    ;x1
        MOV     EDI,[ESP+36]    ;y1
        MOV     EBP,[ESP+40]    ;col
        INT     0x40
        POP     EBX
        POP     EBP
        POP     ESI
        POP     EDI
        RET

_api_getkey:            ;int api_getkey(int mode)
        MOV     EDX,15
        MOV     EAX,[ESP+4]     ;mode
        INT     0x40
        RET

_api_refreshwin:
        PUSh    EDI
        PUSh    ESI
        PUSh    EBX
        MOV     EDX,12
        MOV     EBX,[ESP+16]    ;win
        MOV     EAX,[ESP+20]    ;x0
        MOV     ECX,[ESP+24]    ;y0
        MOV     ESI,[ESP+28]    ;x1
        MOV     EDI,[ESP+32]    ;y1
        INT     0x40
        POP     EBX
        POP     ESI
        POP     EDI
        RET

_api_closewin:
        PUSH    EBX
        MOV     EDX,14
        MOV     EBX,[ESP+8]     ;win
        INT     0x40
        POP     EBX
        RET
