#include "bootpack.h"

struct TIMERCTL timerctl;
struct FIFO8 timerfifo, timerfifo2, timerfifo3; /*时钟数据缓存区*/
void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //上面两行写入0x2e9c,为中断频率
    timerctl.count = 0;
    for (i = 0; i < MAX_TIMER; i++)
    {
        timerctl.timer[i].flags = 0; /*未使用*/
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
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    return;
}
void inthandler20(int *esp)
{
    int i;
    io_out8(PIC0_OCW2, 0x60); /*IRQ-00信号接受结束通知至PIC*/
    timerctl.count++;         /*定时器中断时,计数变量+1*/
    for (i = 0; i < MAX_TIMER; i++)
    {
        if (timerctl.timer[i].flags == TIMER_FLAGS_USING)
        {
            if (timerctl.timer[i].timeout <= timerctl.count) /*timeout作为给定时刻*/
            {
                timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
            }
        }
    }
    return;
}
