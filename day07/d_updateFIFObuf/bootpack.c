#include <stdio.h>
#include "bootpack.h"

extern struct KEYBUF keybuf; /*��ʾ���������ⲿ(����Դ�ļ�)(����̫�������©�����뵼�²���ͨ��,����С�Ķ�makefile)*/

void HariMain(void)
{
    char s[40];
    char mouse[256]; /*256�ֽڵ�ָ��*/

    char *vram;
    int xsize, ysize;
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram;
    unsigned char i, j;

    init_gdtidt();
    init_pic();
    io_sti(); /*�ж�IF��Ϊ1*/

    init_palette();
    init_screen(vram, xsize, ysize);

    /*���*/
    init_mouse_cursor8(mouse, COL8_000000);
    putblock8_8(vram, xsize, 16, 16, xsize / 2, ysize / 2, mouse, 16);

    /*������ʾ*/
    sprintf(s, "scrnx = %d,scrny= %d", xsize, ysize);   /*��������,����sprintf*/
    putfonts8_asc(vram, xsize, COL8_00ffff, 16, 64, s); /*s���ڼ�¼�ַ����ĵ�ַ*/

    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    for (;;)
    {
        io_cli();            /*�����ж�(һ��ִֻ��һ���жϴ���)*/
        if (keybuf.len == 0) /*��黺����,Ϊ��ֱ�ӽ���ͣ��*/
        {
            io_stihlt();
        }
        else
        {
            i = keybuf.data[0];
            keybuf.len--; /*���жϱ�����*/
            if (keybuf.next_r == 32)
            {
                keybuf.next_r = 0;
            }
            io_sti();
            sprintf(s, "%02x", i);
            boxfill8(vram, xsize, COL8_008484, 0, 16, 15, 31);
            putfonts8_asc(vram, xsize, COL8_ffffff, 0, 16, s);
        }
    }
}
