void io_hlt(void); /*函数调用,需要我们将nasm_func.asm和该c源文件连接*/

void HariMain(void)
{
    int i;
    char *p; /*指针 默认p指向一个字节,指针变量本身用四个字节存储*/
    for (i = 0xa0000; i <= 0xaffff; i++)
    {
        p = (char *)i;
        *p = i & 0x0f; //替换了func中的函数
        // write_mem8(i, i & 0x0f); /*调用汇编_write_mem8([i],)    i & 0x0f  高四位全部置0,现已删除*/
    }
    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
}
