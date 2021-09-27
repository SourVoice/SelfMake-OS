#include "bootpack.h"

struct TIMERCTL timerctl;
void init_pit(void)
{
    int i;
    struct TIMER *t;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //上面两行写入0x2e9c,为中断频率
    timerctl.count = 0;
    for (i = 0; i < MAX_TIMER; i++)
    {
        timerctl.timers0[i].flags = 0; /*未使用*/
    }
    t = timer_alloc();
    t->timeout = 0xffffffff; /*这里t设定为最后一个计时器*/
    t->flags = TIMER_FLAGS_USING;
    t->next = 0;
    timerctl.t0 = t; /*初始化中仅有一个哨兵，所以在最前面*/
    timerctl.next_time = 0xffffffff;
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
    struct TIMER *t, *s; /*t 代表对timers[]遍历时，保存的前一个timer,s 代表前一个(顺序表改链表)*/
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    eflags = io_load_eflags();
    io_cli(); /*关闭中断*/
    t = timerctl.t0;
    if (timer->timeout <= t->timeout)
    {
        /*插入到最前面的情况*/
        timerctl.t0 = timer;
        timer->next = t;
        timerctl.next_time = timer->timeout;
        io_store_eflags(eflags);
        return;
    }
    for (;;)
    {
        /*搜索插入位置*/
        s = t;
        t = t->next;
        if (timer->timeout <= t->timeout)
        {
            /*插入到s与t中间*/
            s->next = timer;
            timer->next = t;
            io_store_eflags(eflags);
            return;
        }
    }
}
void inthandler20(int *esp)
{
    struct TIMER *timer;
    io_out8(PIC0_OCW2, 0x60);                /*IRQ-00信号接受结束通知至PIC*/
    timerctl.count++;                        /*定时器中断时,计数变量+1*/
    if (timerctl.next_time > timerctl.count) /*仅判断当前中断和下一个中断*/
    {
        return;
    }
    timer = timerctl.t0; /*首先将最前面的地址赋给timer*/
    for (;;)
    {
        if (timer->timeout > timerctl.count) /*timeout作为给定时刻*/
        {
            break;
        }
        /*超时*/
        timer->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timer->fifo, timer->data);
        timer = timer->next; /*下一个定时器的地址赋给timer*/
    }
    timerctl.t0 = timer;
    /*下一计时器的设定*/
    timerctl.next_time = timerctl.t0->timeout;

    return;
}
