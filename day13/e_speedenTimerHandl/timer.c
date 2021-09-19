#include "bootpack.h"

struct TIMERCTL timerctl;
void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //上面两行写入0x2e9c,为中断频率
    timerctl.count = 0;
    timerctl.next_time = 0xffffffff;
    timerctl.using = 0;
    for (i = 0; i < MAX_TIMER; i++)
    {
        timerctl.timers0[i].flags = 0; /*未使用*/
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
    struct TIMER *t, *s; /*t 代表对timers[]遍历时，保存的前一个timer,s 代表前一个(顺序表改链表)*/
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    eflags = io_load_eflags();
    io_cli(); /*关闭中断*/
    if (timerctl.using == 1)
    {
        /*处于运行状态的定时器仅一个*/
        timerctl.t0 = timer;
        timer->next = 0;
        timerctl.next_time = timer->timeout;
        io_store_eflags(eflags);
        return;
    }
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
        if (t == 0)
        {
            break;
        }
        if (timer->timeout <= t->timeout)
        {
            /*插入到s与t中间*/
            s->next = timer;
            timer->next = timer;
            timer->next = t;
            io_store_eflags(eflags);
            return;
        }
    }
    /*插入最后位置*/
    s->next = timer;
    timer->next = 0;
    io_store_eflags(eflags);
    return;
}
void inthandler20(int *esp)
{
    int i;
    struct TIMER *timer;
    io_out8(PIC0_OCW2, 0x60);                /*IRQ-00信号接受结束通知至PIC*/
    timerctl.count++;                        /*定时器中断时,计数变量+1*/
    if (timerctl.next_time > timerctl.count) /*仅判断当前中断和下一个中断*/
    {
        return;
    }
    timer = timerctl.t0; /*首先将最前面的地址赋给timer*/
    for (i = 0; i < timerctl.using; i++)
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
