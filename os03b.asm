;开始写入装载区（IPL）
;全18扇区装载区
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

    MOV     SI,MSG          ;源变址寄存器写入MSG地址
putloop:
    MOV     al,[si]         ;[SI]表示内存的地址，al寄存器填入了一个地址
    add     si,1
    cmp     al,0            ;累加寄存器地位为零进入下面执行

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
    db      "hello,world"
    db      0x0a
    db      0
;判断终止启动区
    RESB    510-($-$$)          ;软盘510字节后面为两个固定值，写道这里停住|0x7c00~0x7dff(511字节)用于启动区
    db      0x55,0xaa           ;规定第一个扇区（集启动扇区）最后两个字节为0x55 aa，读取到该字节后计算机认为前面一个扇区为boot启动区
;读盘(全18-1扇区)
    MOV     AX,0x0820           ;软盘数据装载到内存的地址
    MOV     ES,AX               ;地址给到ES(段寄存器，辅助地址),ES存储地址0x0820
    MOV     CH,0                ;柱面号，0
    MOV     DH,0                ;磁头号，0(正面磁头)
    MOV     CL,2                ;扇区号，2
readloop:
    MOV     si,0                ;记录启动失败次数寄存器
retry:
    MOV     AH,0x02             ;读盘
    MOV     AL,1                ;处理对象的扇区数
    MOV     BX,0                ;缓冲地址（仅能读盘使用）
                                ;即内存地址,将软盘读到的数据载入内存的位置
                                ;书写格式为[ES,BX],ES为一个段寄存器,用于乘BX的位数(BX为16位寄存器)
    MOV     DL,0x00             ;驱动器号(指定该驱动读取软盘)
    INT     0x13                ;调用磁盘BIOS，该函数返回一个进位标识(CF)到AH
                                ;没错误AH＝０，有错误则错误号码存入AH,
    jnc     next                ;该扇区无错误读取下一扇区
    add     si,1                ;下面为出错后的循环判断
    cmp     si,5                ;将判断结果存入寄存器(CF,ZF,OF,PF),CF:carry flag
    jae     error               ;si>=5,跳至error(jump if above or equal),该指令借助CF寄存器判断
    MOV     AH,0x00             ;si<5,重置驱动
    MOV     DL,0x00             ;选定A驱动
    INT     0x13                ;重置驱动器(AH=0x00,DL=0x00,0x13函数),物理复位
    jmp     retry               ;jump if carry，进位跳转指令
next:
    MOV     AX,ES               ;扇区读取成功后,内存地址向后移0x200 == MOV ex,512
    add     ax,0x0020           ;ES*16 = 512, 0x0020 = 512/16
    MOV     BX,AX
                                ;上面这三个指令用于设定下一个扇区的读盘范围
    add     CL,1                ;cl用于记录扇区
    cmp     CL,18               
    jbe     readloop            ;cl<=18则继续写(jump if below or equal)

error:
    DB      0x0a,0x0a,0x0a
    DB      "READ ERROR"
succeed:
    MOV     si,MSG
    jump    MSG