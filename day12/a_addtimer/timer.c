#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040 /*PIT��IRQ0����*/

struct TIMERCTL timerctl;
void init_pit(void)
{
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //��������д��0x2e9c,Ϊ�ж�Ƶ��
    timerctl.count = 0;
    return;
}
void inthandler20(int *esp)
{
    io_out8(PIC0_OCW2, 0x60); /*����IRQ-00�ź�֪ͨPIC*/
    timerctl.count++;         /*��ʱ���ж�ʱ,��������+1*/
    return;
}
