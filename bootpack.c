void io_hit(void); /*��������,��Ҫ���ǽ�nasm_func.asm�͸�cԴ�ļ�����*/

void HariMain(void)
{
fin:
    io_hit(); /*ִ�л���е�ͣ������*/
    goto fin;
}