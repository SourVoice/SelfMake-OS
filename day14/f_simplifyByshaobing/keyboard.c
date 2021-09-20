#include "bootpack.h"
struct FIFO32 *keyfifo;
int keydata0;

void wait_KBC_sendready(void)
{ /*ʹ���̿��Ƶ�·����׼��*/
    for (;;)
    {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTRADY) == 0) /*CPU���豸����0x0064����ȡ����,��ȡ�ɹ��жϱ�ʶ*/
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
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); /*����̿��Ƶ�·������Ϣ(ģʽ�趨ָ��0x60)*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE); /*���̿���ģʽ��λ���*/
    return;
}
void inthandler21(int *esp)
{
    /*�������Լ��̵��ж�*/
    int data;
    io_out8(PIC0_OCW2, 0x61);   /*֪ͨPIC��IRQ-01�����Ѿ��������,PIC����IRQ1�ж�*/
    data = io_in8(PORT_KEYDAT); /*�ӱ��Ϊ0x0060���豸����8λ��Ϣ����������,���0x0060�豸Ϊ����*/

    fifo32_put(keyfifo, data + keydata0);
    return;
}