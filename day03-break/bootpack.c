void io_hlt(void); /*��������,��Ҫ���ǽ�nasm_func.asm�͸�cԴ�ļ�����*/

int main(int argc, char **argv)
{
read:
    io_hlt(); /*ִ�л���е�ͣ������*/
    goto read;
    return 0;
}