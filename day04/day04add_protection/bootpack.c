void io_hlt(void); /*函数调用,需要我们将nasm_func.asm和该c源文件连接*/
void io_cli(void); /**/
void io_out8(int port, int data);
int io_load_eflags(void); /*eflags是一个16位的专门的寄存器,含CF,OF,SF,等,这里我们读入中断标志IF*/
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
    int i;
    char *p; /*指针 默认p指向一个字节,指针变量本身用四个字节存储*/

    init_palette(); /*调色板*/

    for (i = 0xa0000; i <= 0xaffff; i++)
    {
        p = (char *)i;
        *p = i & 0x0f; //替换了func中的函数
        // write_mem8(i, i & 0x0f); /*调用汇编_write_mem8([i],)    i & 0x0f  高四位全部置0,现已删除*/
    }
    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
}
/*初始化调色板*/
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
    /*设定调色板前首先中断,因此先读入中断标志*/
    eflags = io_load_eflags(); /*记录中断许可标志*/
    io_cli();                  /*中断许可标志为0,禁止中断(其实就是执行中断)*/
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++)
    {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); /*恢复中断标志*/
    return;
}
