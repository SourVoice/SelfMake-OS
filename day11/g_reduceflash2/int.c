#include "bootpack.h"
/*PIC(CPU��оƬ,�жϿ���оƬ)��ʼ��,ʹ��io_out8������PIC�ڲ��ļĴ���д������*/

struct FIFO8 keyfifo;
struct FIFO8 mousefifo;

void init_pic(void)
{

    /*PIC0,PIC1ָ��PIC�ʹ�PIC*/
    io_out8(PIC0_IMR, 0xff); /*��ֹ�����ж�*/
    io_out8(PIC1_IMR, 0xff); /*��ֹ�����ж�*/

    io_out8(PIC0_ICW1, 0x11);   /*���ش���ģʽ*/
    io_out8(PIC0_ICW2, 0x20);   /*IRQ0-7��INT20-27����,IRQΪ�ж�����(interrput request)*/
    io_out8(PIC0_ICW3, 1 << 2); /*PIC1��IRQ2����*/
    io_out8(PIC0_ICW4, 0x01);   /*�޻�����ģʽ*/

    io_out8(PIC1_ICW1, 0x11); /*���ش���ģʽ*/
    io_out8(PIC1_ICW2, 0x28); /*IRQ8-15��INT28-2f����*/
    io_out8(PIC1_ICW3, 2);    /*PIC1��IRQ2����*/
    io_out8(PIC1_ICW4, 0x01); /*�޻�����ģʽ*/

    io_out8(PIC0_IMR, 0xfb); /*11111011 PIC1����ȫ����ֹ*/
    io_out8(PIC1_IMR, 0xff); /*11111111 ��ֹ�����ж�*/

    return;
}

void inthandler21(int *esp)
{
    unsigned char data;
    io_out8(PIC0_OCW2, 0x61);   /*֪ͨPIC��IRQ-01�����Ѿ��������,PIC����IRQ1�ж�*/
    data = io_in8(PORT_KEYDAT); /*�ӱ��Ϊ0x0060���豸����8λ��Ϣ����������,���0x0060�豸Ϊ����*/

    fifo8_put(&keyfifo, data);
    return;
}

void inthandler2c(int *esp)
{
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64); /*֪ͨPIC1 IRQ-12�������*/
    io_out8(PIC0_OCW2, 0x62); /*֪ͨPIC0 IRQ-02�������*/
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo, data);
    return;
}
void inthandler27(int *esp)
/* PIC0�жϵĲ��������� */
/* ����ж���Athlon64X2��ͨ��оƬ���ṩ�ı�����ֻ��ִ��һ�� */
/* ����ж�ֻ�ǽ��գ���ִ���κβ��� */
/* Ϊʲô������
	��  ��Ϊ����жϿ����ǵ������������ġ�ֻ�Ǵ���һЩ��Ҫ�������*/
/*��ע��ΪԴע��*/
{
    io_out8(PIC0_OCW2, 0x67); /*����PIC��IRQ-07*/
    return;
}
