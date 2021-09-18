#include "bootpack.h"

struct TIMERCTL timerctl;
void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //上面两行写入0x2e9c,为中断频率
    timerctl.count = 0;
    timerctl.next = 0xffffffff;
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
    int eflags, i, j;
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    eflags = io_load_eflags();
    io_cli(); /*关闭中断*/
    for (i = 0; i < timerctl.using; i++)
    {
        /*搜索注册位置*/
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
    io_out8(PIC0_OCW2, 0x60);           /*IRQ-00信号接受结束通知至PIC*/
    timerctl.count++;                   /*定时器中断时,计数变量+1*/
    if (timerctl.next > timerctl.count) /*仅判断当前中断和下一个中断*/
    {
        return;
    }
    // timerctl.next = 0xffffffff;
    for (i = 0; i < timerctl.using; i++)
    {
        if (timerctl.timers[i]->timeout > timerctl.count) /*timeout作为给定时刻*/
        {
            break;
        }
        timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
        fifo32_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
    }
    timerctl.using -= i;
    for (j = 0; j < timerctl.using; j++)
    {
        /*循环移位*/
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
