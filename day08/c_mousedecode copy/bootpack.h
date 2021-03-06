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
void io_stihlt(void);
void io_out8(int port, int data);
unsigned char io_in8(int port);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
/*graphic.c屏幕绘制函数声明*/

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
struct SEGMENT_DESCRIPTOR
{
    short limit_low, base_low;
    char base_mid, access_right; /*内存的起始地址和段个数存在GDTR寄存器,这里该寄存器的写入要用到汇编函数*/
    char limit_high, base_high;
};
struct GATE_DESCRIPTOR
{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e

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
void init_pic(void);         /*初始化PIC*/
void inthandler21(int *esp); /*来自PS/2键盘的中断 */
void inthandler27(int *esp); /*来自PS/2鼠标的中断 */
void inthandler2c(int *esp);

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
struct FIFO8
{
    unsigned char *buf;
    int p, q, size, free, flags; /*p for next_w,q for next_r*/
};

/*fifo8.c*/
#define FLAFS_OVERRUN 0x0001
int fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf); /*缓冲区初始化*/
int fifo8_put(struct FIFO8 *fifo, unsigned char data);            /*向FIFO传送数据并保存*/
int fifo8_get(struct FIFO8 *fifo);                                /*从fifo取出数据*/
int fifo8_status(struct FIFO8 *fifo);                             /*返回buf中有效数据数目*/
/*bootpack.c*/
#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTRADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

struct MOUSE_DEC
{
    unsigned char buf[3], phase;
    int x, y, btn; /*鼠标移动信息和按键状态*/
};                 /*鼠标状态*/

void wait_KBC_sendready(void);                               /*等待键盘控制电路准备完毕*/
void init_keyboard(void);                                    /*初始化键盘控制电路*/
void enable_mouse(struct MOUSE_DEC *mdec);                   /*鼠标激活*/
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat); /*分析鼠标传送至结构中的数据*/
