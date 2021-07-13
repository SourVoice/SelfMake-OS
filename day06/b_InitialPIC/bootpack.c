#include <stdio.h>
#include "bootpack.h"
void HariMain(void)
{
    char s[40];
    char mouse[256]; /*256�ֽڵ�ָ��*/

    char *vram;
    int xsize, ysize;
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram; /*����ע��,vramҲ��һ��ָ�����,ָ��0x0ff8*/

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

    io_out8(PIC0_IMR, 0xf9); /*����PIC1�ͼ����ж�(11111001),��д��PIC�Ĵ���*/
    io_out8(PIC1_IMR, 0xef); /*��������ж�(11101111)*/

    for (;;)
        io_hlt(); /*ִ�л���е�ͣ������*/
}