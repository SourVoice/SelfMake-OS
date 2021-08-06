#include "bootpack.h"
/*PIC(CPU外芯片,中断控制芯片)初始化,使用io_out8函数向PIC内部的寄存器写入内容*/

struct FIFO8 keyfifo;
struct FIFO8 mousefifo;

void init_pic(void)
{

    /*PIC0,PIC1指主PIC和从PIC*/
    io_out8(PIC0_IMR, 0xff); /*禁止所有中断*/
    io_out8(PIC1_IMR, 0xff); /*禁止所有中断*/

    io_out8(PIC0_ICW1, 0x11);   /*边沿触发模式*/
    io_out8(PIC0_ICW2, 0x20);   /*IRQ0-7由INT20-27接受,IRQ为中断请求(interrput request)*/
    io_out8(PIC0_ICW3, 1 << 2); /*PIC1由IRQ2连接*/
    io_out8(PIC0_ICW4, 0x01);   /*无缓冲区模式*/

    io_out8(PIC1_ICW1, 0x11); /*边沿触发模式*/
    io_out8(PIC1_ICW2, 0x28); /*IRQ8-15由INT28-2f接受*/
    io_out8(PIC1_ICW3, 2);    /*PIC1由IRQ2连接*/
    io_out8(PIC1_ICW4, 0x01); /*无缓冲区模式*/

    io_out8(PIC0_IMR, 0xfb); /*11111011 PIC1以外全部禁止*/
    io_out8(PIC1_IMR, 0xff); /*11111111 禁止所有中断*/

    return;
}

void inthandler21(int *esp)
{
    unsigned char data;
    io_out8(PIC0_OCW2, 0x61);   /*通知PIC的IRQ-01请求已经接受完毕,PIC监视IRQ1中断*/
    data = io_in8(PORT_KEYDAT); /*从编号为0x0060的设备输入8位信息代表按键编码,编号0x0060设备为键盘*/

    fifo8_put(&keyfifo, data);
    return;
}

void inthandler2c(int *esp)
{
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64); /*通知PIC1 IRQ-12受理完成*/
    io_out8(PIC0_OCW2, 0x62); /*通知PIC0 IRQ-02受理完成*/
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo, data);
    return;
}
void inthandler27(int *esp)
/* PIC0中断的不完整策略 */
/* 这个中断在Athlon64X2上通过芯片组提供的便利，只需执行一次 */
/* 这个中断只是接收，不执行任何操作 */
/* 为什么不处理？
	→  因为这个中断可能是电气噪声引发的、只是处理一些重要的情况。*/
/*该注释为源注释*/
{
    io_out8(PIC0_OCW2, 0x67); /*启用PIC的IRQ-07*/
    return;
}
