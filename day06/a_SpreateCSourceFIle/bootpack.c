#include <stdio.h>
#include "bootpack.h"

//font部分
static char font_A[16] = {
    0x00, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
    0x24, 0x7e, 0x42, 0x42, 0xe7, 0x00, 0x00}; /*A字母的像素点阵*/
void putfont8(char *vram, int xsize, char color, int x, int y, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, unsigned char color, int x, int y, unsigned char *str);
/*初始化鼠标*/
void init_mouse_cursor8(char *mouse, char bc_color);
/*绘制块(将buf中的颜色填入vram)*/
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
                 int px0, int py0, char *buf, int bxsize); /*p for picture, vxsize = vram的偏移, pxsize,pysize = 图像大小,px0,py0 = 图像位置,bxsize = 图像偏移*/
void HariMain(void)
{

    char *vram;
    char s[40];
    char mouse[256]; /*256字节的指针*/
    int xsize, ysize;
    struct BOOTINFO *binfo;
    binfo = (struct BOOTINFO *)0x0ff0; /*指针binfo放入0x0ff0地址.*/

    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram; /*这里注意,vram也是一个指针变量,指向0x0ff8*/

    putfont8(vram, xsize, COL8_00ffff, 2, 2, font_A);   /*文字在屏幕显示后再显示*/
    putfont8(vram, xsize, COL8_00ffff, 12, 12, font_A); /*文字在屏幕显示后再显示*/
    putfont8(vram, xsize, COL8_00ffff, 22, 22, font_A); /*文字在屏幕显示后再显示*/
    putfont8(vram, xsize, COL8_00ffff, 32, 32, font_A); /*文字在屏幕显示后再显示*/

    putfonts8_asc(vram, xsize, COL8_840000, 20, 20, "what?");
    putfonts8_asc(vram, xsize, COL8_840000, 40, 40, "what?");
    putfonts8_asc(vram, xsize, COL8_840000, 60, 60, "what?");
    /*变量显示*/
    sprintf(s, "scrnx = %d", xsize);                    /*新添内容,调用sprintf*/
    putfonts8_asc(vram, xsize, COL8_00ffff, 16, 64, s); /*s用于记录字符串的地址*/
    /*鼠标*/
    init_mouse_cursor8(mouse, COL8_000000);
    putblock8_8(vram, xsize, 16, 16, 160, 100, mouse, 16);
    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
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
        p = vram + (y + i) * xsize + x; /*切到指定行*/
        d = font[i];
        if ((d & 0x80) != 0)
            *(p + 0) = color; /*和static font对应的话,将该位置填色*/
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
/*初始化屏幕,填充颜色*/
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