#include "bootpack.h"
/*PIC(CPU��оƬ,�жϿ���оƬ)��ʼ��*/
void init_pic(void)
{
    /*PIC0,PIC1ָ��PIC�ʹ�PIC*/
    io_out8(PIC0_IMR, 0xff); /*��ֹ�����ж�*/
    io_out8(PIC1_IMR, 0xff); /*��ֹ�����ж�*/

    io_out8(PIC0_ICW1, 0x11);   /*���ش���ģʽ*/
    io_out8(PIC0_ICW2, 0x20);   /*IRQ0-7��INT20-27����,IRQΪ�ж�����(interrput request)*/
    io_out8(PIC0_ICW3, 1 << 2); /*PIC1��IRQ2����*/
    io_out8(PIC0_ICW4, 0x01);   /*�޻�����ģʽ*/

    io_out8(PIC1_ICW1, 0x11);  /*���ش���ģʽ*/
    io_out8(PIC1_ICW2, 0x28);  /*IRQ8-15��INT28-2f����*/
    io_out8(PIC1_ICW3, 2);     /*PIC1��IRQ2����*/
    io_out8I(PIC1_ICW4, 0x01); /*�޻�����ģʽ*/

    io_out8(PIC0_IMR, 0xfb); /*11111011 PIC1����ȫ����ֹ*/
    io_out8(PIC1_IMR, 0xff); /*11111111 ��ֹ�����ж�*/

    return;
}