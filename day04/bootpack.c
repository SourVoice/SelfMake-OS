void io_hlt(void); /*��������,��Ҫ���ǽ�nasm_func.asm�͸�cԴ�ļ�����*/

void HariMain(void)
{
    int i;
    char *p; /*ָ�� Ĭ��pָ��һ���ֽ�,ָ������������ĸ��ֽڴ洢*/
    for (i = 0xa0000; i <= 0xaffff; i++)
    {
        p = (char *)i;
        *p = i & 0x0f; //�滻��func�еĺ���
        // write_mem8(i, i & 0x0f); /*���û��_write_mem8([i],)    i & 0x0f  ����λȫ����0,����ɾ��*/
    }
    for (;;)
        io_hlt(); /*ִ�л���е�ͣ������*/
}
