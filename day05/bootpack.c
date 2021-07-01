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
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /*参数: vram地址,大小,色号,位置x,位置y,*/

void HariMain(void)
{
    int i;
    init_palette(); /*调色板*/
    char *p;        /*指针 默认p指向一个字节,指针变量本身用四个字节存储*/
    int xsize, ysize;
    xsize = 320;
    ysize = 200;
    char *vram;

    p = (char *)0xa0000;
    boxfill8(p, 320, COL8_ff0000, 20, 20, 120, 120); /*调用显卡绘制矩形*/
    boxfill8(p, 320, COL8_00ff00, 70, 50, 170, 150);
    boxfill8(p, 320, COL8_0000ff, 120, 80, 220, 180);
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_c6c6c6, 0, ysize - 28, xsize - 1, ysize - 28);
    boxfill8(vram, xsize, COL8_ffffff, 0, 0, xsize - 1, ysize - 27);
    boxfill8(vram, xsize, COL8_c6c6c6, 0, 0, xsize - 1, ysize - 1);

    boxfill8(vram, xsize, COL8_ffffff, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_ffffff, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_848484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_848484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);

    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
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