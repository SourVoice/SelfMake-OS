/*asmhead.nas*/
struct BOOTINFO
{
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};

#define ADR_BOOTINFO 0x00000ff0
/*naskfunc.nas汇编函数实现部分声明*/
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
/*graphic.c屏幕绘制函数声明*/
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /*参数: vram地址,大小,色号,位置x,位置y,*/
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
#define PIC0_ICW1 0x0020 /*ICW初始化控制数据*/
#define PIC0_OCW2 0x0020 /*PIC内有许多寄存器,以端口号码进行区别*/
#define PIC0_IMR 0x0021  /*有关ICW的简要信息见书上*/
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1
