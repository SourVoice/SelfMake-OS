/*asmhead.nas*/
struct BOOTINFO
{
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};
#define ADR_BOOTINFO 0x00000ff0 /*binfo��ʼ�ĵ�ַ*/
/*naskfunc.nas��ຯ��ʵ�ֲ�������*/
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void io_out8(int port, int data);
unsigned char io_in8(int port);
int io_load_eflags(void);                                       /*ȡ��eflag�Ĵ�����ֵ*/
void io_store_eflags(int eflags);                               /*��eflags�Ĵ���д������*/
void load_gdtr(int limit, int addr);                            /*��GDT���ص��ڴ�ָ��λ��*/
void load_idtr(int limit, int addr);                            /*��IDT���ص��ڴ�ָ��λ��*/
int load_cr0(void);                                             /*ȡ��CR0�Ĵ���ֵ*/
void store_cr0(int cr0);                                        /*��CR0�Ĵ���д��ֵ*/
unsigned int memtest_sub(unsigned int start, unsigned int end); /*�ڴ���*/
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler20(void);
/*bootpack.c*/
//�ж�
void make_window(unsigned char *buf, int xsize, int ysize, char *title); /*��ʱ���ƴ���*/

/*memory.c*/
//�ڴ���,�ڴ����
#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000 /*�������*/
#define MEMMAN_FREES 4090            /*32KB*/
#define MEMMAN_ADDR 0x003c0000

struct FREEINFO /*�ڴ������Ϣ(��addr��ʼ��size�ֽڿռ����),free[i].addrΪ���ÿռ俪ʼ�ĵ�ַ,*/
{
    unsigned int addr, size;
};

struct MEMMAN /*�ڴ������Ϣ*/
{
    int frees;    /*������Ϣ��Ŀ(�ܹ�)*/
    int maxfrees; /*frees���ֵ*/
    int lostsize; /*�ͷ�ʧ�ܵ��ڴ�Ĵ�С*/
    int losts;    /*�ͷ�ʧ�ܴ���*/
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);                /*�ڴ����ں�memtest_sub*/
void memman_init(struct MEMMAN *man);                                      /*��ʼ���ڴ������Ϣ*/
unsigned int memman_total(struct MEMMAN *man);                             /*�ڴ�����С*/
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);          /*�ڴ����*/
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size); /*�ڴ��ͷ�(MEMMAN���¿�����Ϣ)*/
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);       /*4kΪ��λ�����ڴ����(ȡ�ᴦ��)*/
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/*sheet.c���ڵ���*/
#define MAX_SHEETS 256 /*���ܹ�������ͼ����*/
#define SHEET_USE 1
struct SHEET
{
    unsigned char *buf;           /*ͼ������(�ĵ�ַ)*/
    int bxsize, bysize;           /*ͼ���С b for buffer*/
    int vram_x0, vram_y0;         /*ͼ��λ��*/
    int color_inv, height, flags; /*͸��ɫɫ��,ͼ��߶�,ͼ����Ϣ�趨*/
    struct SHTCTL *ctl;           /*SHEET�д���ctl(һЩsheet�ຯ���п��Բ���ָ��ctl)*/
};
struct SHTCTL /*ͼ�����*/
{
    unsigned char *vram, *map;
    int xsize, ysize, top;
    struct SHEET *sheets[MAX_SHEETS]; /*ͼ���ַ,sheets0�����кú�,��ַд��sheets*/
    struct SHEET sheets0[MAX_SHEETS]; /*ͼ������*/
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl); /*����ͼ��,��Ϣ����ṹ*/
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int color_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1); /*���µ����������ͼ��*/
void sheet_slide(struct SHEET *sht, int vram_x0, int vram_y0);
void sheet_free(struct SHEET *sht);                                                                            /*�ͷ���ʹ��ͼ��ռ�*/
void sheet_refreshsub(struct SHTCTL *ctl, int vram_x0, int vram_y0, int vram_x1, int vram_y1, int h0, int h1); /*h0��ʾ��Ҫˢ�µ�ͼ��*/
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);                         /*��¼mapͼ��*/

/*graphic.c��Ļ���ƺ�������*/

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

void putfont8(char *vram, int xsize, char color, int x, int y, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, unsigned char color, int x, int y, unsigned char *str);
void init_mouse_cursor8(char *mouse, char bc_color);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
                 int px0, int py0, char *buf, int bxsize);
void putfonts_asc_sht(struct SHEET *sht, int x, int y, int color, int bg_color, char *s, int l); /*����boxfill,putfonts8_asc,sheet_refresh*/

/*dsctbl.cȫ�ֶμ�¼����жϼ�¼���趨��������*/
struct SEGMENT_DESCRIPTOR
{
    short limit_low, base_low;
    char base_mid, access_right; /*�ڴ����ʼ��ַ�Ͷθ�������GDTR�Ĵ���,����üĴ�����д��Ҫ�õ���ຯ��*/
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

/*��ɫ����*/
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
#define PIC0_ICW1 0x0020 /*ICW(��ʼ����������),�й�ICW�ļ�Ҫ��Ϣ������*/
#define PIC0_OCW2 0x0020 /*PIC�������Ĵ���,�Զ˿ں����������*/
#define PIC0_IMR 0x0021  /*IMR��PIC���ж����μĴ���,8λ��Ӧ8��IRQ�ź�*/
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

void init_pic(void); /*��ʼ��PIC*/
void inthandler2c(int *esp);

/*fifo.c*/
#define FLAFS_OVERRUN 0x0001
struct FIFO32
{
    int *buf;
    int p, q, size, free, flags; /*p for next_w,q for next_r*/
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf); /*��������ʼ��*/
int fifo32_put(struct FIFO32 *fifo, int data);             /*��FIFO�������ݲ�����*/
int fifo32_get(struct FIFO32 *fifo);                       /*��fifoȡ������*/
int fifo32_status(struct FIFO32 *fifo);                    /*����buf����Ч������Ŀ*/

/*mouse.c*/
#define PORT_KEYDAT 0x0060
#define PORT_KEYCMD 0x0064
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

struct MOUSE_DEC
{
    unsigned char buf[3], phase;
    int x, y, btn; /*����ƶ���Ϣ�Ͱ���״̬*/
};                 /*���״̬*/

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec); /*��꼤��*/
int mouse_decode(struct MOUSE_DEC *mdec, int data);                        /*������괫�����ṹ�е�����*/
void inthandler27(int *esp);                                               /*����PS/2������ж�*/

/*keyboard.c*/
#define PORT_KEYSTA 0x0064
#define KEYSTA_SEND_NOTRADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

void wait_KBC_sendready(void);                      /*�ȴ����̿��Ƶ�·׼�����*/
void init_keyboard(struct FIFO32 *fifo, int data0); /*��ʼ�����̿��Ƶ�·*/
void inthandler21(int *esp);                        /*����PS/2���̵��ж� */

/*timer.c����Ͷ�ʱ��*/
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040     /*PIT��IRQ0����*/
#define MAX_TIMER 500       /*��ʱ�������Ϊ500��*/
#define TIMER_FLAGS_ALLOC 1 /*������״̬*/
#define TIMER_FLAGS_USING 2 /*��ʱ��������*/

struct TIMER
{
    struct TIMER *next;   /*�����һ��������ʱ�Ķ�ʱ����ַ*/
    unsigned int timeout; /*��¼��ʱ����(��ʾһ����Χ,timeout����0��,�����򻺳�����������)*/
    unsigned int flags;   /*���ڼ�¼������ʱ��״̬*/
    struct FIFO32 *fifo;
    int data;
};
struct TIMERCTL
{
    unsigned int count, next_time; /*����������next_time��ʾ��һ����ʱ��ʱ��*/
    struct TIMER *t0;              /*t0��ʾtimers[]�еĵ�һ�������ǽ���Ҫ��һ��*/
    struct TIMER timers0[MAX_TIMER];
};
void init_pit(void); /*��ʼ��PIT,�������ʱ��*/
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp); /*�����ж�(���ʵ��)*/
