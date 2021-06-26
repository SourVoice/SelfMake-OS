void io_hit(void); /*函数调用,需要我们将nasm_func.asm和该c源文件连接*/

void HariMain(void)
{
fin:
    io_hit(); /*执行汇编中的停机函数*/
    goto fin;
}