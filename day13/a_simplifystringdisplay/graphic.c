//屏幕绘制相关
#include "bootpack.h"

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
    eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++)
    {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags);
    return;
}
void init_screen(unsigned char *vram, int xsize, int ysize)
{
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29); /*背景色*/

    boxfill8(vram, xsize, COL8_c6c6c6, 0, ysize - 28, xsize - 1, ysize - 28);
    boxfill8(vram, xsize, COL8_ffffff, 0, ysize - 27, xsize - 1, ysize - 27);
    boxfill8(vram, xsize, COL8_c6c6c6, 0, ysize - 26, xsize - 1, ysize - 1);

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

/*显示字符串*/
void putfonts8_asc(unsigned char *vram, int xsize, unsigned char color, int x, int y, unsigned char *str)
{
    extern char hankaku[4096];
    for (; *str != 0x00; str++)
    {
        putfont8(vram, xsize, color, x, y, hankaku + (*str) * 16);
        x += 8;
    }
    return;
}
/*显示字体*/
void putfont8(char *vram, int xsize, char color, int x, int y, char *font)
{
    int i;
    char d;
    char *p;
    for (i = 0; i < 16; i++) /*16行*/
    {
        p = vram + (y + i) * xsize + x;
        d = font[i];
        if ((d & 0x80) != 0)
            *(p + 0) = color;
        if ((d & 0x40) != 0)
            *(p + 1) = color;
        if ((d & 0x20) != 0)
            *(p + 2) = color;
        if ((d & 0x10) != 0)
            *(p + 3) = color;
        if ((d & 0x08) != 0)
            *(p + 4) = color;
        if ((d & 0x04) != 0)
            *(p + 5) = color;
        if ((d & 0x02) != 0)
            *(p + 6) = color;
        if ((d & 0x01) != 0)
            *(p + 7) = color;
    }
    return;
}
/*初始化屏幕,填充颜色,0表示左上角,1表示右上角*/
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
/*绘制鼠标*/
void init_mouse_cursor8(char *mouse, char bc_color)
{
    static char cursor[16][16] = {"**************..",
                                  "*OOOOOOOOOOO*...",
                                  "*OOOOOOOOOO*....",
                                  "*OOOOOOOOO*.....",
                                  "*OOOOOOOO*......",
                                  "*OOOOOOO*.......",
                                  "*OOOOOOO* ......",
                                  "*OOOOOOOO*......",
                                  "*OOOO**OOO*.....",
                                  "*OOO*..*OOO*....",
                                  "*OO*....*OOO*...",
                                  "*O*......*OOO*..",
                                  "**........*OOO*.",
                                  "*..........*OOO*",
                                  "............*OO*",
                                  ".............***"};
    int x, y;
    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            if (cursor[y][x] == '*')
            {
                *(mouse + y * 16 + x) = COL8_000084;
            }
            if (cursor[y][x] == 'O')
            {
                *(mouse + y * 16 + x) = COL8_00ffff;
            }
            if (cursor[y][x] == '.')
            {
                *(mouse + y * 16 + x) = bc_color;
            }
        }
    }
    return;
}
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
    int x, y;
    for (y = 0; y < pysize; y++) /*这里的坐标从0开始,到pysize结束,是屏幕的 ys*/
    {
        for (x = 0; x < pxsize; x++)
        {
            *(vram + (y + py0) * vxsize + (x + px0)) = *(buf + y * bxsize + x);
        }
    }
    return;
}
void putfonts_asc_sht(struct SHEET *sht, int x, int y, int color, int bg_color, char *s, int l)
/*x,y显示坐标位置,c for 字符颜色,b for 背景颜色 ,s for string, l for length of string*/
{
    boxfill8(sht->buf, sht->bxsize, bg_color, x, y, x + l * 8 - 1, y + 15);
    putfonts8_asc(sht->buf, sht->bxsize, color, x, y, s);
    sheet_refresh(sht, x, y, x + 1 * 8, y + 16);
    return;
}
