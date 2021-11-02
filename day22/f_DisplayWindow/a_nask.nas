[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]
        GLOBAL  _api_putchar
        GLOBAL  _api_end
        GLOBAL  _api_putstr0
        GLOBAL  _api_openwin        
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
