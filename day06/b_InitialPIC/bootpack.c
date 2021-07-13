#include <stdio.h>
#include "bootpack.h"
void HariMain(void)
{
    char s[40];
    char mouse[256]; /*256字节的指针*/

    char *vram;
    int xsize, ysize;
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram; /*这里注意,vram也是一个指针变量,指向0x0ff8*/

    init_gdtidt();
    init_pic();
    io_sti(); /*中断IF设为1*/

    init_palette();
    init_screen(vram, xsize, ysize);

    /*鼠标*/
    init_mouse_cursor8(mouse, COL8_000000);
    putblock8_8(vram, xsize, 16, 16, xsize / 2, ysize / 2, mouse, 16);

    /*变量显示*/
    sprintf(s, "scrnx = %d,scrny= %d", xsize, ysize);   /*新添内容,调用sprintf*/
    putfonts8_asc(vram, xsize, COL8_00ffff, 16, 64, s); /*s用于记录字符串的地址*/

    io_out8(PIC0_IMR, 0xf9); /*开放PIC1和键盘中断(11111001),即写入PIC寄存器*/
    io_out8(PIC1_IMR, 0xef); /*开放鼠标中断(11101111)*/

    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
}