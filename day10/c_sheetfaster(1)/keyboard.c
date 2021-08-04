#include "bootpack.h"
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
void init_keyboard(void)
{

    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); /*����̿��Ƶ�·������Ϣ(ģʽ�趨ָ��0x60)*/
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE); /*���̿���ģʽ��λ���*/
    return;
}
