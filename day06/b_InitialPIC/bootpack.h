/*asmhead.nas*/
struct BOOTINFO
{
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};
#define ADR_BOOTINFO 0x00000ff0 /*binfo开始的地址*/
/*naskfunc.nas汇编函数实现部分声明*/
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
/*graphic.c屏幕绘制函数声明*/

// static char font_A[16] = { //font部分
//     0x00, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
//     0x24, 0x7e, 0x42, 0x42, 0xe7, 0x00, 0x00}; /*A字母的像素点阵*/

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

void putfont8(char *vram, int xsize, char color, int x, int y, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, unsigned char color, int x, int y, unsigned char *str);
void init_mouse_cursor8(char *mouse, char bc_color);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
                 int px0, int py0, char *buf, int bxsize);
/*dsctbl.c全局段记录表和中断记录表设定函数声明*/
struct SegmentDescriptor
{
    short limit_low, base_low;
    char base_mid, access_right; /*内存的起始地址和段个数存在GDTR寄存器,这里该寄存器的写入要用到汇编函数*/
    short limit_high, base_high;
};
struct GATEDescriptor
{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_gdtidt();
void set_SegDesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar);
void set_GATEDesc(struct GATEDescriptor *gd, int offset, int selector, int ar);

#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e /*IDT属性,表明用于处理中断的有效设定*/

/*颜色定义*/
#define COL8_000000 0
#define COL8_ff0000 1
#define COL8_00ff00 2
#define COL8_ffff00 3
#define COL8_0000ff 4
#define COL8_ff00ff 5
#define COL8_00ffff 6
#define COL8_ffffff 7
#define COL8_c6c6c6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

/* int.c */
void init_pic(void); /*初始化PIC*/
void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);
/*PIC端口号,0是主PIC,1代表从PIC*/
#define PIC0_ICW1 0x0020 /*ICW(初始化控制数据),有关ICW的简要信息见书上*/
#define PIC0_OCW2 0x0020 /*PIC内有许多寄存器,以端口号码进行区别*/
#define PIC0_IMR 0x0021  /*IMR是PIC内中断屏蔽寄存器,8位对应8个IRQ信号*/
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1
