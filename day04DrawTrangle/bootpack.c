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
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

void HariMain(void)
{
    int i;
    char *p; /*指针 默认p指向一个字节,指针变量本身用四个字节存储*/

    init_palette(); /*调色板*/

    p = (char *)0xa0000;
    boxfill8(p, 320, COL8_ff0000, 20, 20, 120, 120);
    boxfill8(p, 320, COL8_00ff00, 70, 50, 170, 150);
    boxfill8(p, 320, COL8_0000ff, 120, 80, 220, 180);

    // for (i = 0xa0000; i <= 0xaffff; i++)
    // {
    //     p = (char *)i;
    //     *p = i & 0x0f; //替换了func中的函数
    //     // write_mem8(i, i & 0x0f); /*调用汇编_write_mem8([i],)    i & 0x0f  高四位全部置0,现已删除*/
    // }
    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for ()
    {
    }
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