[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "bug1.nas"] 
        GLOBAL _api_putchar
        GLOBAL _api_end
[SECTION .text]
_api_putchar:
        MOV     EDX,1
        MOV     AL,[ESP+4]
        INT     0x40
        RET
_api_end:
        MOV     EDX,4
        INT     0x40       
