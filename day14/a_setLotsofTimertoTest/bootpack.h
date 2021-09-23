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
int io_load_eflags(void);                                       /*取出eflag寄存器的值*/
void io_store_eflags(int eflags);                               /*向eflags寄存器写入内容*/
void load_gdtr(int limit, int addr);                            /*将GDT加载到内存指定位置*/
void load_idtr(int limit, int addr);                            /*将IDT加载到内存指定位置*/
int load_cr0(void);                                             /*取出CR0寄存器值*/
void store_cr0(int cr0);                                        /*向CR0寄存器写入值*/
unsigned int memtest_sub(unsigned int start, unsigned int end); /*内存检查*/
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler20(void);
/*bootpack.c*/
//中断
void make_window(unsigned char *buf, int xsize, int ysize, char *title); /*暂时绘制窗口*/

/*memory.c*/
//内存检查,内存分配
#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000 /*缓存禁用*/
#define MEMMAN_FREES 4090            /*32KB*/
#define MEMMAN_ADDR 0x003c0000

struct FREEINFO /*内存可用信息(从addr开始的size字节空间可用),free[i].addr为可用空间开始的地址,*/
{
    unsigned int addr, size;
};

struct MEMMAN /*内存管理信息*/
{
    int frees;    /*可用信息数目(总共)*/
    int maxfrees; /*frees最大值*/
    int lostsize; /*释放失败的内存的大小*/
    int losts;    /*释放失败次数*/
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);                /*内存检查内含memtest_sub*/
void memman_init(struct MEMMAN *man);                                      /*初始化内存管理信息*/
unsigned int memman_total(struct MEMMAN *man);                             /*内存空余大小*/
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);          /*内存分配*/
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size); /*内存释放(MEMMAN更新可用信息)*/
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);       /*4k为单位进行内存分配(取舍处理)*/
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/*sheet.c窗口叠加*/
#define MAX_SHEETS 256 /*所能管理的最大图层数*/
#define SHEET_USE 1
struct SHEET
{
    unsigned char *buf;           /*图层内容(的地址)*/
    int bxsize, bysize;           /*图层大小 b for buffer*/
    int vram_x0, vram_y0;         /*图层位置*/
    int color_inv, height, flags; /*透明色色号,图层高度,图层信息设定*/
    struct SHTCTL *ctl;           /*SHEET中存入ctl(一些sheet类函数中可以不用指定ctl)*/
};
struct SHTCTL /*图层管理*/
{
    unsigned char *vram, *map;
    int xsize, ysize, top;
    struct SHEET *sheets[MAX_SHEETS]; /*图层地址,sheets0中排列好后,地址写入sheets*/
    struct SHEET sheets0[MAX_SHEETS]; /*图层内容*/
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl); /*生成图层,信息填入结构*/
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int color_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1); /*从下到上描绘所有图层*/
void sheet_slide(struct SHEET *sht, int vram_x0, int vram_y0);
void sheet_free(struct SHEET *sht);                                                                            /*释放已使用图层空间*/
void sheet_refreshsub(struct SHTCTL *ctl, int vram_x0, int vram_y0, int vram_x1, int vram_y1, int h0, int h1); /*h0表示需要刷新的图层*/
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);                         /*记录map图层*/

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
void putfonts_asc_sht(struct SHEET *sht, int x, int y, int color, int bg_color, char *s, int l); /*集成boxfill,putfonts8_asc,sheet_refresh*/

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

void init_pic(void); /*初始化PIC*/
void inthandler2c(int *esp);

/*fifo.c*/
#define FLAFS_OVERRUN 0x0001
struct FIFO32
{
    int *buf;
    int p, q, size, free, flags; /*p for next_w,q for next_r*/
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf); /*缓冲区初始化*/
int fifo32_put(struct FIFO32 *fifo, int data);             /*向FIFO传送数据并保存*/
int fifo32_get(struct FIFO32 *fifo);                       /*从fifo取出数据*/
int fifo32_status(struct FIFO32 *fifo);                    /*返回buf中有效数据数目*/

/*mouse.c*/
#define PORT_KEYDAT 0x0060
#define PORT_KEYCMD 0x0064
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

struct MOUSE_DEC
{
    unsigned char buf[3], phase;
    int x, y, btn; /*鼠标移动信息和按键状态*/
};                 /*鼠标状态*/

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec); /*鼠标激活*/
int mouse_decode(struct MOUSE_DEC *mdec, int data);                        /*分析鼠标传送至结构中的数据*/
void inthandler27(int *esp);                                               /*基于PS/2的鼠标中断*/

/*keyboard.c*/
#define PORT_KEYSTA 0x0064
#define KEYSTA_SEND_NOTRADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

void wait_KBC_sendready(void);                      /*等待键盘控制电路准备完毕*/
void init_keyboard(struct FIFO32 *fifo, int data0); /*初始化键盘控制电路*/
void inthandler21(int *esp);                        /*来自PS/2键盘的中断 */

/*timer.c间隔型定时器*/
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040     /*PIT和IRQ0相连*/
#define MAX_TIMER 500       /*定时器最多设为500个*/
#define TIMER_FLAGS_ALLOC 1 /*已配置状态*/
#define TIMER_FLAGS_USING 2 /*定时器运行中*/

struct TIMER
{
    struct TIMER *next;   /*存放下一个即将超时的定时器地址*/
    unsigned int timeout; /*记录超时数据(表示一个范围,timeout到达0后,程序向缓冲区发送数据)*/
    unsigned int flags;   /*用于记录各个定时器状态*/
    struct FIFO32 *fifo;
    int data;
};
struct TIMERCTL
{
    unsigned int count, next_time; /*计数变量，next_time表示下一个超时的时刻*/
    struct TIMER *t0;              /*t0表示timers[]中的第一个，我们仅需要第一个*/
    struct TIMER timers0[MAX_TIMER];
};
void init_pit(void); /*初始化PIT,即间隔定时器*/
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp); /*启用中断(汇编实现)*/
