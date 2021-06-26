void io_hlt(void); /*函数调用,需要我们将nasm_func.asm和该c源文件连接*/

int main(int argc, char **argv)
{
read:
    io_hlt(); /*执行汇编中的停机函数*/
    goto read;
    return 0;
}