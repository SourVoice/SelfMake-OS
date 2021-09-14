#include "bootpack.h"
struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void)
{ /*使键盘控制电路做好准备*/
    for (;;)
    {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTRADY) == 0) /*CPU从设备号码0x0064处读取数据,读取成功判断标识*/
        {
            break;
        }
    }
    return;
}
void init_keyboard(struct FIFO32 *fifo, int data0)
{

    keyfifo = fifo;
    keydata0 = data0;
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); /*向键盘控制电路传送信息(模式设定指令0x60)*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE); /*键盘控制模式切位鼠标*/
    return;
}
void inthandler21(int *esp)
{
    /*接受来自键盘的中断*/
    int data;
    io_out8(PIC0_OCW2, 0x61);   /*通知PIC的IRQ-01请求已经接受完毕,PIC监视IRQ1中断*/
    data = io_in8(PORT_KEYDAT); /*从编号为0x0060的设备输入8位信息代表按键编码,编号0x0060设备为键盘*/

    fifo32_put(keyfifo, data + keydata0);
    return;
}