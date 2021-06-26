void io_hlt(void); /*函数调用,需要我们将nasm_func.asm和该c源文件连接*/
void write_mem8(int addr, int data);

void HariMain(void)
{
    int i;

    for (i = 0xa0000; i < 0xaffff; i++)
    {
        write_mem8(i, i & 0x0f); /*调用汇编_write_mem8([i],15)*/
    }
    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
}