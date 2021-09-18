#include "bootpack.h"

struct TIMERCTL timerctl;
void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //��������д��0x2e9c,Ϊ�ж�Ƶ��
    timerctl.count = 0;
    timerctl.next = 0xffffffff;
    timerctl.using = 0;
    for (i = 0; i < MAX_TIMER; i++)
    {
        timerctl.timers0[i].flags = 0; /*δʹ��*/
    }
    return;
}
struct TIMER *timer_alloc(void)
{
    int i;
    for (i = 0; i < MAX_TIMER; i++)
    {
        if (timerctl.timers0[i].flags == 0)
        {
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];
        }
    }
    return 0;
}
void timer_free(struct TIMER *timer)
{
    timer->flags = 0;
    return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
    timer->fifo = fifo;
    timer->data = data;
    return;
}
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
    int eflags, i, j;
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    eflags = io_load_eflags();
    io_cli(); /*�ر��ж�*/
    for (i = 0; i < timerctl.using; i++)
    {
        /*����ע��λ��*/
        if (timerctl.timers[i]->timeout >= timer->timeout)
        {
            break;
        }
    }
    for (j = timerctl.using; j > i; j--)
    {
        timerctl.timers[j] = timerctl.timers[j - i];
    }
    timerctl.using ++;
    timerctl.timers[i] = timer;
    timerctl.next = timerctl.timers[0]->timeout;
    io_store_eflags(eflags);
    return;
}
void inthandler20(int *esp)
{
    int i, j;
    io_out8(PIC0_OCW2, 0x60);           /*IRQ-00�źŽ��ܽ���֪ͨ��PIC*/
    timerctl.count++;                   /*��ʱ���ж�ʱ,��������+1*/
    if (timerctl.next > timerctl.count) /*���жϵ�ǰ�жϺ���һ���ж�*/
    {
        return;
    }
    // timerctl.next = 0xffffffff;
    for (i = 0; i < timerctl.using; i++)
    {
        if (timerctl.timers[i]->timeout > timerctl.count) /*timeout��Ϊ����ʱ��*/
        {
            break;
        }
        timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
    }
    timerctl.using -= i;
    for (j = 0; j < timerctl.using; j++)
    {
        /*ѭ����λ*/
        timerctl.timers[j] = timerctl.timers[i + j];
    }
    if (timerctl.using > 0)
    {
        timerctl.next = timerctl.timers[0]->timeout;
    }
    else
    {
        timerctl.next = 0xffffffff;
    }
    return;
}
