#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo; /*表示定义来自外部(其他源文件)(编译太快这里会漏掉编译导致不能通过,可以小改动makefile)*/

void HariMain(void)
{
    char s[40];
    char mouse[256];
    char keybuf[32];

    char *vram;
    int xsize, ysize;
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram;
    unsigned char i, j;

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

    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    fifo8_init(&keyfifo, 32, keybuf);
    for (;;)
    {
        io_cli();                        /*屏蔽中断(一次只执行一次中断处理)*/
        if (fifo8_status(&keyfifo) == 0) /*检查缓冲区,为空直接进入停机*/
        {
            io_stihlt();
        }
        else
        {
            i = fifo8_get(&keyfifo);
            io_sti();
            sprintf(s, "%02x", i);
            boxfill8(vram, xsize, COL8_008484, 0, 16, 15, 31);
            putfonts8_asc(vram, xsize, COL8_ffffff, 0, 16, s);
        }
    }
}
