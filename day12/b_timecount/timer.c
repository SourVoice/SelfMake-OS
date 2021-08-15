#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040 /*PIT和IRQ0相连*/

struct TIMERCTL timerctl;
void init_pit(void)
{
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e); //上面两行写入0x2e9c,为中断频率
    timerctl.count = 0;
    return;
}
void inthandler20(int *esp)
{
    io_out8(PIC0_OCW2, 0x60); /*接收IRQ-00信号通知PIC*/
    timerctl.count++;         /*定时器中断时,计数变量+1*/
    return;
}
