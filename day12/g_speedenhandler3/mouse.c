#include "bootpack.h"
void enable_mouse(struct MOUSE_DEC *mdec)
{
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE); /*ͬ����̿��Ƶ�·������Ϣ,0xd4��ʹ��һ�������Զ����ɸ����*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE); /*KBC����0xfa��cpu*/
    mdec->phase = 0;                       /*0xfa�͹���ʱ��ȥ��׶�*/
    return;
}
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
    if (mdec->phase == 0)
    {
        if (dat == 0xfa)
        {
            /*�ȴ�����0xfa�׶�*/
            mdec->phase = 1;
        }
        return 0;
    }
    if (mdec->phase == 1)
    {
        if ((dat & 0xc8) == 0x08)
        {
            /*�����һ���ֽ���ȷ��*/
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
    if (mdec->phase == 3) /*�������׶�ʱ�����ݷ���*/
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
        mdec->y = -mdec->y; /*�������Ļ�����y�෴*/
        return 1;
    }
    return -1;
}
