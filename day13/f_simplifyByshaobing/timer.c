#include "bootpack.h"

struct TIMERCTL timerctl;
void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //��������д��0x2e9c,Ϊ�ж�Ƶ��
    timerctl.count = 0;
    timerctl.next_time = 0xffffffff;
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
    int eflags;
    struct TIMER *t, *s; /*t �����timers[]����ʱ�������ǰһ��timer,s ����ǰһ��(˳��������)*/
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    eflags = io_load_eflags();
    io_cli(); /*�ر��ж�*/
    if (timerctl.using == 1)
    {
        /*��������״̬�Ķ�ʱ����һ��*/
        timerctl.t0 = timer;
        timer->next = 0;
        timerctl.next_time = timer->timeout;
        io_store_eflags(eflags);
        return;
    }
    t = timerctl.t0;
    if (timer->timeout <= t->timeout)
    {
        /*���뵽��ǰ������*/
        timerctl.t0 = timer;
        timer->next = t;
        timerctl.next_time = timer->timeout;
        io_store_eflags(eflags);
        return;
    }
    for (;;)
    {
        /*��������λ��*/
        s = t;
        t = t->next;
        if (t == 0)
        {
            break;
        }
        if (timer->timeout <= t->timeout)
        {
            /*���뵽s��t�м�*/
            s->next = timer;
            timer->next = timer;
            timer->next = t;
            io_store_eflags(eflags);
            return;
        }
    }
    /*�������λ��*/
    s->next = timer;
    timer->next = 0;
    io_store_eflags(eflags);
    return;
}
void inthandler20(int *esp)
{
    int i;
    struct TIMER *timer;
    io_out8(PIC0_OCW2, 0x60);                /*IRQ-00�źŽ��ܽ���֪ͨ��PIC*/
    timerctl.count++;                        /*��ʱ���ж�ʱ,��������+1*/
    if (timerctl.next_time > timerctl.count) /*���жϵ�ǰ�жϺ���һ���ж�*/
    {
        return;
    }
    timer = timerctl.t0; /*���Ƚ���ǰ��ĵ�ַ����timer*/
    for (i = 0; i < timerctl.using; i++)
    {
        if (timer->timeout > timerctl.count) /*timeout��Ϊ����ʱ��*/
        {
            break;
        }
        /*��ʱ*/
        timer->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timer->fifo, timer->data);
        timer = timer->next; /*��һ����ʱ���ĵ�ַ����timer*/
    }
    timerctl.using -= i;

    timerctl.t0 = timer;

    if (timerctl.using > 0)
    {
        timerctl.next_time = timerctl.t0->timeout;
    }
    else
    {
        timerctl.next_time = 0xffffffff;
    }
    return;
}
