void io_hit(void); /*��������,��Ҫ���ǽ�nasm_func.asm�͸�cԴ�ļ�����*/
void write_mem8(int addr, int data);

void HariMain(void)
{
    int i;

    for (i = 0xa0000; i < 0xaffff; i++)
    {
        write_mem8(i, 15); /*���û��_write_mem8([i],15)*/
    }
    for (;;)
        io_hit(); /*ִ�л���е�ͣ������*/
}