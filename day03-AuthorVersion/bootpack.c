void io_hlt(void); /*��������,��Ҫ���ǽ�nasm_func.asm�͸�cԴ�ļ�����*/
void write_mem8(int addr, int data);

void HariMain(void)
{
    int i;

    for (i = 0xa0000; i < 0xaffff; i++)
    {
        write_mem8(i, i & 0x0f); /*���û��_write_mem8([i],15)*/
    }
    for (;;)
        io_hlt(); /*ִ�л���е�ͣ������*/
}