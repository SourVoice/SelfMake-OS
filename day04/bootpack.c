void io_hlt(void); /*��������,��Ҫ���ǽ�nasm_func.asm�͸�cԴ�ļ�����*/
void io_cli(void); /**/
void io_out8(int port, int data);
int io_load_eflags(void); /*eflags��һ��16λ��ר�ŵļĴ���,��CF,OF,SF,��,�������Ƕ����жϱ�־IF*/
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
    int i;
    char *p; /*ָ�� Ĭ��pָ��һ���ֽ�,ָ������������ĸ��ֽڴ洢*/

    init_palette(); /*��ɫ��*/

    for (i = 0xa0000; i <= 0xaffff; i++)
    {
        p = (char *)i;
        *p = i & 0x0f; //�滻��func�еĺ���
        // write_mem8(i, i & 0x0f); /*���û��_write_mem8([i],)    i & 0x0f  ����λȫ����0,����ɾ��*/
    }
    for (;;)
        io_hlt(); /*ִ�л���е�ͣ������*/
}
/*��ʼ����ɫ��*/
void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00, /**/
        0xff, 0x00, 0x00, /**/
        0x00, 0xff, 0x00, /**/
        0xff, 0xff, 0x00, /**/
        0x00, 0x00, 0xff, /**/
        0xff, 0x00, 0xff, /**/
        0x00, 0xff, 0xff, /**/
        0xff, 0xff, 0xff, /**/
        0xc6, 0xc6, 0xc6, /**/
        0x84, 0x00, 0x00, /**/
        0x00, 0x84, 0x00, /**/
        0x84, 0x84, 0x00, /**/
        0x00, 0x00, 0x84, /**/
        0x84, 0x00, 0x84, /**/
        0x00, 0x84, 0x84, /**/
        0x84, 0x84, 0x84, /**/
    };
    set_palette(0, 15, table_rgb);
    return;
}
void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    /*�趨��ɫ��ǰ�����ж�,����ȶ����жϱ�־*/
    eflags = io_load_eflags(); /*��¼�ж���ɱ�־*/
    io_cli();                  /*�ж���ɱ�־Ϊ0,��ֹ�ж�(��ʵ����ִ���ж�)*/
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++)
    {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); /*�ָ��жϱ�־*/
    return;
}
