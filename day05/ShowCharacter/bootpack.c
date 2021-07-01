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
void io_hlt(void);
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void io_cli(void);
void io_out8(int port, int data);
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /*参数: vram地址,大小,色号,位置x,位置y,*/
//
struct BOOTINFO
{
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
}; //严重怀疑这里使用结构体是为了节约空间的
//font部分
static char font_A[16] = {
    0x00, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
    0x24, 0x7e, 0x42, 0x42, 0xe7, 0x00, 0x00}; /*A字母的像素点阵*/
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void HariMain(void)
{
    init_palette(); /*调色板*/

    char *vram;
    int xsize, ysize;
    struct BOOTINFO *binfo;
    binfo = (struct BOOTINFO *)0x0ff0; /*指针binfo放入0x0ff0地址.*/
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram;              /*这里注意,vram也是一个指针变量,指向0x0ff8*/
    init_screen(vram, xsize, ysize); /*屏幕部分封装函数*/
    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
}
void init_screen(unsigned char *vram, int xsize, int ysize)
{
    // boxfill8(vram, xsize, COL8_ffffff, 20, 20, 120, 120);
    // boxfill8(vram, xsize, COL8_ff0000, 40, 40, 140, 140);
    // boxfill8(vram, xsize, COL8_848484, 60, 60, 160, 160);
    putfont8(vram, xsize, 1, 1, COL8_00ffff, font_A);

    boxfill8(vram, xsize, COL8_848484, 0, 0, xsize - 1, ysize - 29);

    boxfill8(vram, xsize, COL8_ffffff, 3, ysize - 24, 59, ysize - 24);
    boxfill8(vram, xsize, COL8_ffffff, 2, ysize - 24, 2, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 3, ysize - 4, 59, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 59, ysize - 23, 59, ysize - 5);
    boxfill8(vram, xsize, COL8_000000, 2, ysize - 3, 59, ysize - 3);
    boxfill8(vram, xsize, COL8_000000, 60, ysize - 24, 60, ysize - 3);

    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize - 3);
    boxfill8(vram, xsize, COL8_ffffff, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
    boxfill8(vram, xsize, COL8_ffffff, xsize - 3, ysize - 24, xsize - 3, ysize - 3);
}
/*显示字体*/
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
    int i;
    char d;
    char *p;
    p = vram + (y + i) * xsize + x;
    for (i = 0; i < 16; i++)
    {
        d = font[i];
        if ((d & 0x80) != 0)
            *(p + 0) = c;
        if ((d & 0x40) != 0)
            *(p + 1) = c;
        if ((d & 0x20) != 0)
            *(p + 2) = c;
        if ((d & 0x10) != 0)
            *(p + 3) = c;
        if ((d & 0x08) != 0)
            *(p + 4) = c;
        if ((d & 0x04) != 0)
            *(p + 5) = c;
        if ((d & 0x02) != 0)
            *(p + 6) = c;
        if ((d & 0x01) != 0)
            *(p + 7) = c;
    }
    return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++)
    {
        for (x = x0; x <= x1; x++)
        {
            vram[y * xsize + x] = c; /*色号给到指定vram处*/
        }
    }
    return;
}

/*初始化调色板*/
void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00, /**/
        0xff, 0x00, 0x00, /**/
        0x00, 0xff, 0x00, /**/
        0xff, 0xff, 0x00, /**/
        0x00, 0x00, 0xff, /**/
        0xff, 0x00, 0xff, /**/
        0x00, 0xff, 0xff, /**/
        0xff, 0xff, 0xff, /**/
        0xc6, 0xc6, 0xc6, /**/
        0x84, 0x00, 0x00, /**/
        0x00, 0x84, 0x00, /**/
        0x84, 0x84, 0x00, /**/
        0x00, 0x00, 0x84, /**/
        0x84, 0x00, 0x84, /**/
        0x00, 0x84, 0x84, /**/
        0x84, 0x84, 0x84, /**/
    };
    set_palette(0, 15, table_rgb);
    return;
}
void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    /*设定调色板前首先中断,因此先读入中断标志*/
    eflags = io_load_eflags(); /*记录中断许可标志*/
    io_cli();                  /*中断许可标志为0,禁止中断(其实就是执行中断)*/
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++)
    {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); /*恢复中断标志*/
    return;
}