#include "bootpack.h"

struct TIMERCTL timerctl;
struct FIFO8 timerfifo, timerfifo2, timerfifo3; /*ʱ�����ݻ�����*/
void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //��������д��0x2e9c,Ϊ�ж�Ƶ��
    timerctl.count = 0;
    for (i = 0; i < MAX_TIMER; i++)
    {
        timerctl.timer[i].flags = 0; /*δʹ��*/
    }
    return;
}
struct TIMER *timer_alloc(void)
{
    int i;
    for (i = 0; i < MAX_TIMER; i++)
    {
        if (timerctl.timer[i].flags == 0)
        {
            timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timer[i];
        }
    }
    return 0;
}
void timer_free(struct TIMER *timer)
{
    timer->flags = 0;
    return;
}
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
    timer->fifo = fifo;
    timer->data = data;
    return;
}
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
    timer->timeout = timeout;
    timer->flags = TIMER_FLAGS_USING;
    return;
}
void inthandler20(int *esp)
{
    int i;
    io_out8(PIC0_OCW2, 0x60); /*IRQ-00�źŽ��ܽ���֪ͨ��PIC*/
    timerctl.count++;         /*��ʱ���ж�ʱ,��������+1*/
    for (i = 0; i < MAX_TIMER; i++)
    {
        if (timerctl.timer[i].flags == TIMER_FLAGS_USING)
        {
            timerctl.timer[i].timeout--;
            if (timerctl.timer[i].timeout == 0)
            {

                timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
            }
        }
    }
    return;
}