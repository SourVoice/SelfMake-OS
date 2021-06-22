;hello-os
;Tab = 4
ORG     0x7c00              ;程序加载至0x7c00处(程序指该计算机boot程序)
jmp     entry               ;进入程序
                            ;同    0s01.asm对比可知，0xeb,0x4e = jmp entry,两个地址对应
;模拟FAT12格式软盘，模拟其内存分区机制
db      0x90
db      "HELLOIPL"          ;启动区名称可以是任意字符串（8字节）
dw      512                 ;每个扇区大小，固定值，因为计算机读取软盘以512字节定长读取
db      1                   ;簇的大小
dw      1                   ;FAT起始位置
db      2                   ;FAT个数
dw      224                 ;根目录的大小
dw      2880                ;磁盘大小，，2880个扇区，2880x512 = 1474560字节
db      0xf0                ;磁盘种类
dw      9                   ;FAT长度
dw      18                  ;1个磁道所拥有的扇区（必须为18）个
dw      2                   ;磁头数（固定值为2）
dd      0                   ;不使用分区，必须为0
dd      2880                ;重写磁盘大小
db      0,0,0x29            ;固定数据
dd      0xffffffff          ;卷标号码
db      "HELLO-OS   "       ;磁盘名称（11字节） 
db      "FAT12"             ;磁盘格式名称（8字节）
RESB    18                  ;空出18字节
;程序核心
entry:
    MOV     AX,0            ;初始化寄存器
    MOV     ss,AX
    MOV     sp,0x7c00       ;使栈寄存器指向org启动地址
    MOV     DS,AX
    MOV     es,AX

    MOV     SI,MSG
putloop:
    MOV     al,[si]         ;[SI]表示内存的地址，al寄存器填入了一个地址
    add     si,1
    cmp     al,0

    je      fin
    MOV     AH,0x0e         ;显示文字
    MOV     bx,15           ;指定字符颜色
    INT     0x10            ;调用显卡BIOS
    jmp     putloop
fin:
    HLT
    jmp     fin
MSG:
    DB      0x0a,0x0a
    db      "hello world"
    db      0x0a
    db      0
;判断终止启动区
RESB    510-($-$$)        ;软盘510字节后面为两个固定值，写道这里停住
db      0x55,0xaa           ;规定第一个扇区（集启动扇区）最后两个字节为0x55 aa，读取到该字节后计算机认为前面一个扇区为boot启动区

;启动区之外输出部分
db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
RESB    4600
db      0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
RESB    1469432
