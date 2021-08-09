#include "bootpack.h"
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
