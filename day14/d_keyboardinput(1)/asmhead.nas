;说明:这部分配置了32位保护模式,并使得C语言能够导入,这部分新增代码以后讲解(内核部分)

;os
;TAB=4
[INSTRSET "i486p"]

VBEMODE	EQU		0x105			; 1024 x  768 x 8bit 彩色
; 显示模式
;	0x100 :  640 x  400 x 8bit 彩色
;	0x101 :  640 x  480 x 8bit 彩色
;	0x103 :  800 x  600 x 8bit 彩色
;	0x105 : 1024 x  768 x 8bit 彩色
;	0x107 : 1280 x 1024 x 8bit 彩色


;BIOS-INFO,这部分内容放到内存0x0ff0处,作为操作系统的设置

;根据函数入口地址调节32位模式,这TM都没讲保护模式的进入，我都没法用gcc连接目标文件

BOTPAK	EQU		0x00280000		;加载bootpack
DSKCAC	EQU		0x00100000		;磁盘缓存的位置
DSKCAC0	EQU		0x00008000		;磁盘缓存的位置（实模式）

;BOOT_INFO相关
CYLS	EQU		0x0ff0			;引导扇区设置
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			;关于颜色的信息
SCRNX	EQU		0x0ff4			;分辨率X
SCRNY	EQU		0x0ff6			;分辨率Y
VRAM	EQU		0x0ff8			;图像缓冲区的起始地址

		ORG		0xc200			; 这个的程序要被装载的内存地址

;画面(屏幕设定)
		;确认VBE是否存在(VBE是VESA协会的BIOS标准)
		MOV		AX,0x9000
		MOV		ES,AX			
		MOV		DI,0			;显卡能利用的VBE信息需要写入到内存中以ES:DI开始的512字节中,0x9000指明了写入地址
		MOV		AX,0x4f00
		INT		0x10			;执行INT 0x10后,若存在VCE则AX会变成0x004f
		CMP		AX,0x004f
		JNE		scrn320
		;检查VBE版本
		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320							;if(AX<0x0200) got o scrn320
		;取得画面信息
		MOV		CX,VBEMODE						;读取画面模式信息
		MOV		AX,0x4f01
		INT  	0x10
		CMP		AX,0x004f						;再次确认了AX的值
		JNE		scrn320
		;画面模式信息确认
		CMP		BYTE [ES:DI+0x19],8				;该地址存储颜色数(我们设定了为8)
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4				;该地址存储Y分辨率,确认Y分辨率
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]					;模式属性修改
		AND		AX,0x0080
		JZ		scrn320		
		
		;画面模式切换
		MOV		BX,VBEMODE+0x4000				;VBE的640x480x8bi色彩
		MOV		AX,0x4f02						;切换高分辨模式
		INT		0x10
		MOV		BYTE [VMODE],8					;屏幕的模式,8bit（参考C语言的引用）
		MOV		AX,[ES:DI+0x12]					;X的分辨率写入scrnX
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]					;y的分辨率写入scrnY
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]				;VRAM的地址
		MOV		[VRAM],EAX
		JMP		keystatus						;跳过sran320部分

scrn320:										;显卡不支持VBE,改为使用320*320模式
		MOV		AL,0x13							;VGA图,320x200x8bit彩色
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

;通过BIOS获取指示灯状态
keystatus:
		MOV		AH,0x02
		INT		0x16 			;keyboard BIOS
		MOV		[LEDS],AL

;防止PIC接受所有中断(执行报护模式转换时屏蔽所有中断)
;	AT兼容机的规范、PIC初始化
;	然后之前在CLI不做任何事就挂起
;	PIC在同意后初始化

		MOV		AL,0xff
		OUT		0x21,AL			;禁止从主PIC的全部中断
		NOP						;不断执行OUT指令
		OUT		0xa1,AL			;禁止从从PIC的全部中断

		CLI						;进一步中断CPU

;让CPU支持1M以上内存、设置A20GATE
;CPU在16位模式下最大内存只有1MB
;以下使内存可用
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			;enable A20
		OUT		0x60,AL
		CALL	waitkbdout

;保护模式转换(就是这里!!被坑了)

[INSTRSET "i486p"]				;说明使用486指令(可以使用LGDT,EAX,CR0等32位有关指令)

		LGDT	[GDTR0]			;设置临时GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	;使用bit31（禁用分页）
		OR		EAX,0x00000001	;bit0到1转换（保护模式过渡）
		MOV		CR0,EAX			;通过CR0切换到保护模式
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			; 写32bit的段
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

;bootpack传递

		MOV		ESI,bootpack	;源
		MOV		EDI,BOTPAK		;目标
		MOV		ECX,512*1024/4
		CALL	memcpy

;传输磁盘数据

;从引导区开始

		MOV		ESI,0x7c00		;源
		MOV		EDI,DSKCAC		;目标
		MOV		ECX,512/4
		CALL	memcpy

;剩余的全部

		MOV		ESI,DSKCAC0+512	;源
		MOV		EDI,DSKCAC+512	;目标
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	;除以4得到字节数
		SUB		ECX,512/4		;IPL偏移量
		CALL	memcpy

;由于还需要asmhead才能完成
;完成其余的bootpack任务

;bootpack启动

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			;ECX += 3;
		SHR		ECX,2			;ECX /= 4;
		JZ		skip			;传输完成
		MOV		ESI,[EBX+20]	;源
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	;目标
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	;堆栈的初始化
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		;AND结果不为0跳转到waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			;运算结果不为0跳转到memcpy
		RET
;memcpy地址前缀大小

		ALIGNB	16
GDT0:
		RESB	8				;初始值
		DW		0xffff,0x0000,0x9200,0x00cf	;写32bit位段寄存器
		DW		0xffff,0x0000,0x9a28,0x0047	;可执行的文件的32bit寄存器（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
