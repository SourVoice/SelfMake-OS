#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo; /*��ʾ���������ⲿ(����Դ�ļ�)(����̫�������©�����뵼�²���ͨ��,����С�Ķ�makefile)*/
extern struct FIFO8 mousefifo;
void HariMain(void)
{

    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    int xsize = binfo->scrnx, ysize = binfo->scrny;
    char *vram = binfo->vram;
    char s[40], mouse[256], keybuf[32], mousebuf[128];
    int mouse_x = 0, mouse_y = 0;

    struct MOUSE_DEC mdec; /*d����decode,phase�׶�,��¼���ݽ��ܵĽ׶�*/
    unsigned char data;
    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR; /*memman��Ҫ32KB��С���ڴ洢�ڴ�ռ���÷�����Ϣ������ʹ�ô�0x003c0000�ŵ�ַ�Ժ�*/

    init_gdtidt();
    init_pic();
    io_sti(); /*�ж�IF��Ϊ1*/

    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /*���ż����ж�*/
    io_out8(PIC1_IMR, 0xef); /*��������ж�*/

    /*�ж�*/
    init_keyboard();     /*���̽�����ջ��*/
    enable_mouse(&mdec); /*��������ջ��*/

    /*�ڴ����*/
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette();
    init_screen(vram, xsize, ysize);

    /*���*/
    init_mouse_cursor8(mouse, COL8_000000);
    // putblock8_8(vram, xsize, 16, 16, (xsize - 16) / 2, (ysize - 28 - 16) / 2, mouse, 16);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_ffffff, s);

    sprintf(s, "memory %dMB free: %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(vram, xsize, COL8_ffffff, 0, 32, s);

    for (;;)
    {
        io_cli();                                                   /*�����ж�(һ��ִֻ��һ���жϴ���)*/
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) /*��黺����,Ϊ��ֱ�ӽ���ͣ��*/
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
                    if ((mdec.btn & 0x01) != 0) /*���λΪ1*/
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

                    /*�ƶ�ָ��*/
                    boxfill8(vram, xsize, COL8_008484, mouse_x, mouse_y, mouse_x + 15, mouse_y + 15); /*�������*/

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
                    boxfill8(vram, xsize, COL8_008484, 0, 0, 79, 15);              /*��������*/
                    putfonts8_asc(vram, xsize, COL8_ffffff, 0, 0, s);              /*��ʾ����*/
                    putblock8_8(vram, xsize, 16, 16, mouse_x, mouse_y, mouse, 16); /*�������*/
                }
            }
        }
    }
}