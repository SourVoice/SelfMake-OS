#include <stdio.h>
#include "bootpack.h"
/*extern+声明形式表示定义来自外部(其他源文件)(编译太快这里会漏掉编译导致不能通过,可以小改动makefile)*/
struct FIFO32 fifo;

/*bootpack.c*/
void make_window(unsigned char *buf, int xsize, int ysize, char *title); /*暂时绘制窗口*/
void set490(struct FIFO32 *fifo, int mode);                              /*追加490个定时器*/
void HariMain(void)
{

    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    int xsize = binfo->scrnx, ysize = binfo->scrny;
    char *vram = binfo->vram;
    char s[40];
    int fifobuf[128]; /*mousebuf 消息中断缓存与下面有区别*/
    struct TIMER *timer, *timer2, *timer3;
    int mouse_x = 0, mouse_y = 0, count = 0;

    struct MOUSE_DEC mdec; /*d代表decode,phase阶段,记录数据接受的阶段*/
    int data;
    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR; /*memman需要32KB大小用于存储内存空间可用分配信息，我们使用从0x003c0000号地址以后*/

    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse, *sht_win;
    unsigned char *buf_back, buf_mouse[256], *buf_win; /*buf_mouse 图像内容*/

    init_gdtidt();
    init_pic();
    io_sti(); /*中断IF设为1，即开放CPU中断*/

    /*缓冲区初始化*/
    fifo32_init(&fifo, 128, fifobuf);

    init_pit(); /*计时器间隔中断*/
    /*中断*/
    init_keyboard(&fifo, 256);       /*键盘接受至栈打开*/
    enable_mouse(&fifo, 512, &mdec); /*鼠标接受至栈打开*/

    io_out8(PIC0_IMR, 0xf8); /*开放键盘中断(更改端口号使PIC1和PIT和键盘均为许可)*/
    io_out8(PIC1_IMR, 0xef); /*开放鼠标中断*/

    /*时钟中断设置*/
    set490(&fifo, 1);

    timer = timer_alloc();
    timer_init(timer, &fifo, 10);
    timer_settime(timer, 1000);
    timer2 = timer_alloc();
    timer_init(timer2, &fifo, 3);
    timer_settime(timer2, 300);
    timer3 = timer_alloc();
    timer_init(timer3, &fifo, 1);
    timer_settime(timer3, 50);

    /*内存分配*/
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    /*画面*/
    init_palette();
    shtctl = shtctl_init(memman, vram, binfo->scrnx, binfo->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    sht_win = sheet_alloc(shtctl);
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 68);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 没有透明色 */
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);                   /* 透明色号99 */
    sheet_setbuf(sht_win, buf_win, 160, 68, -1);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(buf_mouse, 99); /* 背景色号99 */
    make_window(buf_win, 160, 68, "window");
    putfonts8_asc(buf_win, 160, COL8_000000, 24, 44, "This OS!");
    sheet_slide(sht_back, 0, 0);
    mouse_x = (binfo->scrnx - 16) / 2; /* 按显示在画面中央来计算坐标 */
    mouse_y = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(sht_mouse, mouse_x, mouse_y);
    sheet_slide(sht_win, 80, 72);
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 1);
    sheet_updown(sht_mouse, 2);
    sprintf(s, "(%3d, %3d)", mouse_x, mouse_y);
    putfonts8_asc(buf_back, binfo->scrnx, 0, 0, 0, s);
    sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(buf_back, binfo->scrnx, COL8_ffffff, 0, 32, s);
    sheet_refresh(sht_back, 0, 0, xsize, 48); /* 刷新文字 */

    for (;;)
    {
        count++;
        io_cli();                      /*屏蔽中断(一次只执行一次中断处理)*/
        if (fifo32_status(&fifo) == 0) /*检查缓冲区,为空直接进入停机*/
        {
            io_sti(); /*shihlt改为了sti*/
        }
        else
        {
            data = fifo32_get(&fifo);
            io_sti();
            if (256 <= data && data <= 511)
            {
                sprintf(s, "%02x", data - 256);
                putfonts_asc_sht(sht_back, 0, 16, COL8_ffffff, COL8_008484, s, 2);
            }
            else if (512 <= data && data <= 767)
            {
                if (mouse_decode(&mdec, data - 512) != 0)
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
                    putfonts_asc_sht(sht_back, 32, 16, COL8_ffffff, COL8_008484, s, 15);
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
                    if (mouse_x > xsize - 1) /*使鼠标出界后隐藏,但在refreshsub写入图层时仍然存在问题,需要不刷新新画面以外的内容*/
                    {
                        mouse_x = xsize - 1;
                    }
                    if (mouse_y > ysize - 1)
                    {
                        mouse_y = ysize - 1;
                    }
                    sprintf(s, "(%3d,%3d)", mouse_x, mouse_y);
                    putfonts_asc_sht(sht_back, 0, 0, COL8_ffffff, COL8_008484, s, 10);
                    sheet_slide(sht_mouse, mouse_x, mouse_y); /*含refresh*/
                }
            }
            else if (data == 10)
            {
                putfonts_asc_sht(sht_back, 0, 64, COL8_ffffff, COL8_008484, "10[sec]", 7);
                sprintf(s, "%010d", count);
                putfonts_asc_sht(sht_win, 40, 28, COL8_000000, COL8_c6c6c6, s, 10);
            }
            else if (data == 3)
            {
                putfonts_asc_sht(sht_back, 0, 80, COL8_ffffff, COL8_008484, "3[sec]", 6);
                count = 0;
            }
            else if (data == 1) /*光标用定时器*/
            {
                timer_init(timer3, &fifo, 0);
                boxfill8(buf_back, xsize, COL8_ffffff, 8, 96, 15, 111);
                timer_settime(timer3, 50);
                sheet_refresh(sht_back, 8, 96, 16, 112);
            }
            else if (data == 0) /*光标用定时器*/
            {
                timer_init(timer3, &fifo, 1);
                boxfill8(buf_back, xsize, COL8_008484, 8, 96, 15, 111);
                timer_settime(timer3, 50);
                sheet_refresh(sht_back, 8, 96, 16, 112);
            }
        }
    }
}
void make_window(unsigned char *buf, int xsize, int ysize, char *title)
{
    static char closebtn[14][16] =
        {"OOOOOOOOOOOOOOO@",
         "OQQQQQQQQQQQQQ$@",
         "OQQQQQQQQQQQQQ$@",
         "OQQQ@@QQQQ@@QQ$@",
         "OQQQQ@@QQ@@QQQ$@",
         "OQQQQQ@@@@QQQQ$@",
         "OQQQQQQ@@QQQQQ$@",
         "OQQQQQ@@@@QQQQ$@",
         "OQQQQ@@QQ@@QQQ$@",
         "OQQQ@@QQQQ@@QQ$@",
         "OQQQQQQQQQQQQQ$@",
         "OQQQQQQQQQQQQQ$@",
         "O$$$$$$$$$$$$$$@",
         "@@@@@@@@@@@@@@@@"};
    int x, y;
    char c;
    boxfill8(buf, xsize, COL8_c6c6c6, 0, 0, xsize - 1, 0);
    boxfill8(buf, xsize, COL8_ffffff, 1, 1, xsize - 2, 1);
    boxfill8(buf, xsize, COL8_c6c6c6, 0, 0, 0, ysize - 1);
    boxfill8(buf, xsize, COL8_ffffff, 1, 1, 1, ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_c6c6c6, 2, 2, xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20);
    boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
    putfonts8_asc(buf, xsize, COL8_ffffff, 24, 4, title);
    for (y = 0; y < 14; y++)
    {
        for (x = 0; x < 16; x++)
        {
            c = closebtn[y][x];
            if (c == '@')
            {
                c = COL8_000000;
            }
            else if (c == '$')
            {
                c = COL8_848484;
            }
            else if (c == 'Q')
            {
                c = COL8_c6c6c6;
            }
            else
            {
                c = COL8_ffffff;
            }
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    return;
}
void set490(struct FIFO32 *fifo, int mode)
{
    int i;
    struct TIMER *timer;
    if (mode != 0)
    {

        for (i = 0; i < 490; i++)
        {

            timer = timer_alloc();
            timer_init(timer, fifo, 1024 + i);
            timer_settime(timer, 100 * 60 * 60 * 24 * 50 + i * 100);
        }
    }
    return;
}
