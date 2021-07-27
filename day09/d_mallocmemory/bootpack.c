#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo; /*表示定义来自外部(其他源文件)(编译太快这里会漏掉编译导致不能通过,可以小改动makefile)*/
extern struct FIFO8 mousefifo;
void HariMain(void)
{

    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    int xsize = binfo->scrnx, ysize = binfo->scrny;
    char *vram = binfo->vram;
    char s[40], mouse[256], keybuf[32], mousebuf[128];
    int mouse_x = 0, mouse_y = 0;

    struct MOUSE_DEC mdec; /*d代表decode,phase阶段,记录数据接受的阶段*/
    unsigned char data;
    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR; /*memman需要32KB大小用于存储内存空间可用分配信息，我们使用从0x003c0000号地址以后*/

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

    init_palette();
    init_screen(vram, xsize, ysize);

    /*鼠标*/
    init_mouse_cursor8(mouse, COL8_000000);
    // putblock8_8(vram, xsize, 16, 16, (xsize - 16) / 2, (ysize - 28 - 16) / 2, mouse, 16);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_ffffff, s);

    sprintf(s, "memory %dMB free: %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(vram, xsize, COL8_ffffff, 0, 32, s);

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

                    /*移动指针*/
                    boxfill8(vram, xsize, COL8_008484, mouse_x, mouse_y, mouse_x + 15, mouse_y + 15); /*隐藏鼠标*/

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
                    boxfill8(vram, xsize, COL8_008484, 0, 0, 79, 15);              /*隐藏坐标*/
                    putfonts8_asc(vram, xsize, COL8_ffffff, 0, 0, s);              /*显示坐标*/
                    putblock8_8(vram, xsize, 16, 16, mouse_x, mouse_y, mouse, 16); /*绘制鼠标*/
                }
            }
        }
    }
}
unsigned int memtest(unsigned int start, unsigned int end)
{
    char flag486 = 0;
    unsigned int eflg, cr0, i;
    /*检查CPU为386还是486(有无高速缓存的区别)*/
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /*AC-bit = 1*/
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) /*486系列处理器EFLAGs寄存器第18位为AC标志*/
    {
        flag486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT; /*AC-bit = 0*/
    io_store_eflags(eflg);
    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0); /*缓存禁用*/
    }
    i = memtest_sub(start, end);
    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0); /*启用缓存*/
    }
    return i;
}
void memman_init(struct MEMMAN *man)
{
    man->frees = 0;
    man->maxfrees = 0;
    man->losts = 0;
    man->lostsize = 0;
    return;
}
unsigned int memman_total(struct MEMMAN *man)
{
    unsigned int i, t = 0; /*t for memory abaiable*/
    for (i = 0; i < man->frees; i++)
    {
        t += man->free[i].size;
    }
    return t;
} //audio_enjoyment
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
    unsigned int i, a;
    for (i = 0; i < man->frees; i++)
    {
        if (man->free[i].size >= size) /*free[i].addr处有size可用*/
        {
            a = man->free[i].size;
            man->free[i].size -= size;  /*可用空间大小减少size*/
            man->free[i].addr += size;  /*可用空间地址向后推进size*/
            if (man->free[i].size == 0) /*free[i]处空间已满*/
            {
                man->frees--; /*可用空余空间数减一*/
                for (; i < man->frees; i++)
                {
                    man->free[i] = man->free[i + 1];
                }
            }
            return a;
        }
    }
    return 0; /*没有可用空间*/
}
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i, j;
    for (i = 0; i < man->frees; i++)
    {
        if (man->free[i].addr > addr)
        {
            break;
        }
    }
    if (i > 0)
    {
        if (man->free[i - 1].addr + man->free[i - 1].size == addr)
        {
            man->free[i - 1].size += size; /*与前面可用内存合并*/
            if (i < man->frees)
            {
                /*后面的合并*/
                if (addr + size == man->free[i].addr)
                {
                    man->free[i - 1].size += man->free[i].size;
                    man->frees--; /*删去man->free[i]*/
                    for (; i < man->frees; i++)
                    {
                        /*同FIFO至零操作*/
                        man->free[i] = man->free[i + 1];
                    }
                }
            }
            return 0;
        }
    }
    /*不能与前面的可用空间进行归纳的内存*/
    if (i < man->frees)
    {
        if (addr + size == man->free[i].addr)
        {
            /*可用与后面的归纳到一起*/
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;
        }
    }
    /*既不能与前面归纳也不能与后面合并*/
    if (man->frees < MEMMAN_FREES)
    {
        /*free[i]之后的向后移动*/
        for (j = man->frees; j > i; j--)
        {
            man->free[j] = man->free[j - 1];
        }
        man->frees++;
        if (man->maxfrees < man->frees)
        {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    man->lostsize++;
    man->lostsize += size;
    return -1;
}
