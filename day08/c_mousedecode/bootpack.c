#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo; /*表示定义来自外部(其他源文件)(编译太快这里会漏掉编译导致不能通过,可以小改动makefile)*/
extern struct FIFO8 mousefifo;
void HariMain(void)
{
    char s[40];
    char mouse[256];

    char *vram;
    int xsize, ysize;
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram;
    unsigned char data;

    init_gdtidt();
    init_pic();
    io_sti(); /*中断IF设为1*/

    init_palette();
    init_screen(vram, xsize, ysize);

    /*鼠标*/
    init_mouse_cursor8(mouse, COL8_000000);
    putblock8_8(vram, xsize, 16, 16, xsize / 2, ysize / 2, mouse, 16);

    /*变量显示*/
    sprintf(s, "scrnx = %d,scrny= %d", xsize, ysize); /*新添内容,调用sprintf*/
    putfonts8_asc(vram, xsize, COL8_00ffff, 0, 0, s); /*s用于记录字符串的地址*/

    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    /*中断*/
    char keybuf[32];
    char mousebuf[128];
    struct MOUSE_DEC mdec; /*d代表decode,phase阶段,记录数据接受的阶段*/

    fifo8_init(&keyfifo, 32, keybuf);
    init_keyboard(); /*键盘接受至栈打开*/

    fifo8_init(&mousefifo, 128, mousebuf);
    enable_mouse(&mdec); /*鼠标接受至栈打开*/
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
                boxfill8(vram, xsize, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(vram, xsize, COL8_ffffff, 0, 16, s);
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
                    boxfill8(vram, xsize, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(vram, xsize, COL8_ffffff, 32, 16, s);
                }
            }
        }
    }
}
void wait_KBC_sendready(void)
{ /*使键盘控制电路做好准备*/
    for (;;)
    {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTRADY) == 0) /*CPU从设备号码0x0064处读取数据,读取成功判断标识*/
        {
            break;
        }
    }
    return;
}
void init_keyboard(void)
{

    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); /*向键盘控制电路传送信息(模式设定指令0x60)*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE); /*键盘控制模式切位鼠标*/
    return;
}
void enable_mouse(struct MOUSE_DEC *mdec)
{
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE); /*同向键盘控制电路发送信息,0xd4会使下一个数据自动发松给鼠标*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE); /*KBC返回0xfa到cpu*/
    mdec->phase = 0;                       /*0xfa送过来时舍去这阶段*/
    return;
}
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
    if (mdec->phase == 0)
    {
        if (dat == 0xfa)
        {
            /*等待鼠标的0xfa阶段*/
            mdec->phase = 1;
        }
        return 0;
    }
    if (mdec->phase == 1)
    {
        if ((dat & 0xc8) == 0x08)
        {
            /*检验第一个字节正确性*/
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if (mdec->phase == 2)
    {
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if (mdec->phase == 3) /*仅第三阶段时将数据返回*/
    {
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if ((mdec->buf[0] & 0x10) != 0)
        {
            mdec->x |= 0xffffff00;
        }
        if ((mdec->buf[0] & 0x20) != 0)
        {
            mdec->y |= 0xffffff00;
        }
        mdec->y = -mdec->y; /*鼠标与屏幕方向的y相反*/
        return 1;
    }
    return -1;
}
