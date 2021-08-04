#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo; /*表示定义来自外部(其他源文件)(编译太快这里会漏掉编译导致不能通过,可以小改动makefile)*/
extern struct FIFO8 mousefifo;
void HariMain(void)
{

    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    int xsize = binfo->scrnx, ysize = binfo->scrny;
    char *vram = binfo->vram;
    char s[40], keybuf[32], mousebuf[128]; /*mousebuf 消息中断缓存与下面有区别*/
    int mouse_x = 0, mouse_y = 0;

    struct MOUSE_DEC mdec; /*d代表decode,phase阶段,记录数据接受的阶段*/
    unsigned char data;
    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR; /*memman需要32KB大小用于存储内存空间可用分配信息，我们使用从0x003c0000号地址以后*/

    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse;
    unsigned char *buf_back, buf_mouse[256]; /*buf_mouse 图像内容*/

    init_gdtidt();
    init_pic();
    io_sti(); /*中断IF设为1*/

    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /*开放键盘中断*/
    io_out8(PIC1_IMR, 0xef); /*开放鼠标中断*/

    /*中断*/
    init_keyboard();     /*键盘接受至栈打开*/
    enable_mouse(&mdec); /*鼠标接受至栈打开*/

    /*内存分配*/
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    /*画面*/
    init_palette();
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 没有透明色 */
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);                   /* 透明色号99 */
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(buf_mouse, 99); /* 背景色号99 */
    sheet_slide(shtctl, sht_back, 0, 0);
    mouse_x = (binfo->scrnx - 16) / 2; /* 按显示在画面中央来计算坐标 */
    mouse_y = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(shtctl, sht_mouse, mouse_x, mouse_y);
    sheet_updown(shtctl, sht_back, 0);
    sheet_updown(shtctl, sht_mouse, 1);
    sprintf(s, "(%3d, %3d)", mouse_x, mouse_y);
    putfonts8_asc(buf_back, binfo->scrnx, COL8_ffffff, 0, 0, s);
    sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(buf_back, binfo->scrnx, COL8_ffffff, 0, 32, s);
    sheet_refresh(shtctl, sht_back, 0, 0, xsize, 48); /* 刷新文字 */

    for (;;)
    {
        io_cli();                                                   /*屏蔽中断(一次只执行一次中断处理)*/
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) /*检查缓冲区,为空直接进入停机*/
        {
            io_stihlt();
        }
        else
        {
            if (fifo8_status(&keyfifo) != 0)
            {

                data = fifo8_get(&keyfifo);
                io_sti();
                sprintf(s, "%02x", data);
                boxfill8(buf_back, xsize, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(buf_back, xsize, COL8_ffffff, 0, 16, s);
                sheet_refresh(shtctl, sht_back, 0, 0, xsize, 48);
            }
            else if (fifo8_status(&mousefifo) != 0)
            {
                data = fifo8_get(&mousefifo);
                io_sti();
                if (mouse_decode(&mdec, data) != 0)
                {
                    sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) /*最低位为1*/
                    {
                        s[1] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0)
                    {
                        s[3] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0)
                    {
                        s[2] = 'C';
                    }
                    boxfill8(buf_back, xsize, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(buf_back, xsize, COL8_ffffff, 32, 16, s);
                    sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);
                    /*移动指针*/
                    mouse_x += mdec.x;
                    mouse_y += mdec.y;
                    if (mouse_x < 0)
                    {
                        mouse_x = 0;
                    }
                    if (mouse_y < 0)
                    {
                        mouse_y = 0;
                    }
                    if (mouse_x > xsize - 16)
                    {
                        mouse_x = xsize - 16;
                    }
                    if (mouse_y > ysize - 16)
                    {
                        mouse_y = ysize - 16;
                    }
                    sprintf(s, "(%3d,%3d)", mouse_x, mouse_y);
                    boxfill8(buf_back, xsize, COL8_008484, 0, 0, 79, 15); /*隐藏坐标*/
                    putfonts8_asc(buf_back, xsize, COL8_ffffff, 0, 0, s); /*显示坐标*/
                    sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);        /*向buf_back写入信息时进行refresh*/
                    sheet_slide(shtctl, sht_mouse, mouse_x, mouse_y);     /*含refresh*/
                }
            }
        }
    }
}
