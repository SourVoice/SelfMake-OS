#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo; /*��ʾ���������ⲿ(����Դ�ļ�)(����̫�������©�����뵼�²���ͨ��,����С�Ķ�makefile)*/
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
    io_sti(); /*�ж�IF��Ϊ1*/

    init_palette();
    init_screen(vram, xsize, ysize);

    /*���*/
    init_mouse_cursor8(mouse, COL8_000000);
    putblock8_8(vram, xsize, 16, 16, xsize / 2, ysize / 2, mouse, 16);

    /*������ʾ*/
    sprintf(s, "scrnx = %d,scrny= %d", xsize, ysize); /*��������,����sprintf*/
    putfonts8_asc(vram, xsize, COL8_00ffff, 0, 0, s); /*s���ڼ�¼�ַ����ĵ�ַ*/

    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    /*�ж�*/
    char keybuf[32];
    char mousebuf[128];
    unsigned char mouse_dbuf[3], mouse_phase; /*d����decode,phase�׶�,��¼���ݽ��ܵĽ׶�*/

    fifo8_init(&keyfifo, 32, keybuf);
    init_keyboard(); /*���̽�����ջ��*/

    fifo8_init(&mousefifo, 128, mousebuf);
    enable_mouse(); /*��������ջ��*/
    mouse_phase = 0;
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
                if (mouse_phase == 0)
                {
                    if (data == 0xfa) /*���������0xfa���*/
                    {
                        mouse_phase = 1;
                    }
                }
                else if (mouse_phase == 1)
                {
                    mouse_dbuf[0] = data; /*��¼�����ʽ*/
                    mouse_phase = 2;
                }
                else if (mouse_phase == 2)
                {
                    mouse_dbuf[1] = data; /*����*/
                    mouse_phase = 3;
                }
                else if (mouse_phase == 3)
                {
                    mouse_dbuf[2] = data; /*����*/
                    mouse_phase = 1;
                    sprintf(s, "%02x,%02x,%02x", mouse_dbuf[0], mouse_dbuf[1], mouse_dbuf[2]);
                    boxfill8(vram, xsize, COL8_008484, 32, 16, 32 + 8 * 8, 31);
                    putfonts8_asc(vram, xsize, COL8_ffffff, 32, 16, s);
                }
            }
        }
    }
}
void wait_KBC_sendready(void)
{ /*ʹ���̿��Ƶ�·����׼��*/
    for (;;)
    {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTRADY) == 0) /*CPU���豸����0x0064����ȡ����,��ȡ�ɹ��жϱ�ʶ*/
        {
            break;
        }
    }
    return;
}
void init_keyboard(void)
{

    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); /*����̿��Ƶ�·������Ϣ(ģʽ�趨ָ��0x60)*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE); /*���̿���ģʽ��λ���*/
    return;
}
void enable_mouse(void)
{
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE); /*ͬ����̿��Ƶ�·������Ϣ,0xd4��ʹ��һ�������Զ����ɸ����*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE); /*KBC����0xfa��cpu*/
    return;
}
