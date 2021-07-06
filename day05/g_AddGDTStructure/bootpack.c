#include <stdio.h>

#define COL8_000000 0
#define COL8_ff0000 1
#define COL8_00ff00 2
#define COL8_ffff00 3
#define COL8_0000ff 4
#define COL8_ff00ff 5
#define COL8_00ffff 6
#define COL8_ffffff 7
#define COL8_c6c6c6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15
/**/
void io_hlt(void);
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void io_cli(void);
void io_out8(int port, int data);
/*填充屏幕像素*/
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /*参数: vram地址,大小,色号,位置x,位置y,*/
/* 严重怀疑这里使用结构体是为了节约空间的*/
/*0x0ff0附近数据*/
struct BOOTINFO
{
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};
//font部分
static char font_A[16] = {
    0x00, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
    0x24, 0x7e, 0x42, 0x42, 0xe7, 0x00, 0x00}; /*A字母的像素点阵*/
void putfont8(char *vram, int xsize, char color, int x, int y, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, unsigned char color, int x, int y, unsigned char *str);
/*初始化鼠标*/
void init_mouse_cursor8(char *mouse, char bc_color);
/*绘制块(将buf中的颜色填入vram)*/
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
                 int px0, int py0, char *buf, int bxsize); /*p for picture, vxsize = vram的偏移, pxsize,pysize = 图像大小,px0,py0 = 图像位置,bxsize = 图像偏移*/
/*GDT全局段号记录表*/
struct SegmentDescriptor
{
    short limit_low, base_low;
    char base_mid, access_right; /*内存的起始地址和段个数存在GDTR寄存器,这里该寄存器的写入要用到汇编函数*/
    short limit_high, base_high;
};
/*中断记录表*/
struct GATEDescriptor
{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};
void init_gdtidt();
void set_SegDesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar);
void set_GATEDesc(struct GATE_Descriptor *gd, int offset, int selector, int ar);
void load_gdtr(int add);
void HariMain(void)
{
    init_palette(); /*调色板*/

    char *vram;
    char s[40];
    char mouse[256]; /*256字节的指针*/
    int xsize, ysize;
    struct BOOTINFO *binfo;
    binfo = (struct BOOTINFO *)0x0ff0; /*指针binfo放入0x0ff0地址.*/
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram; /*这里注意,vram也是一个指针变量,指向0x0ff8*/

    init_screen(vram, xsize, ysize); /*屏幕部分封装函数*/
    putfonts8_asc(vram, xsize, COL8_840000, 20, 20, "what?");
    putfonts8_asc(vram, xsize, COL8_840000, 40, 40, "what?");
    putfonts8_asc(vram, xsize, COL8_840000, 60, 60, "what?");
    /*变量显示*/
    sprintf(s, "scrnx = %d", xsize);                    /*新添内容,调用sprintf*/
    putfonts8_asc(vram, xsize, COL8_00ffff, 16, 64, s); /*s用于记录字符串的地址*/
    /*鼠标*/
    init_mouse_cursor8(mouse, COL8_000000);
    putblock8_8(vram, xsize, 16, 16, 160, 100, mouse, 16);
    for (;;)
        io_hlt(); /*执行汇编中的停机函数*/
}
void init_screen(unsigned char *vram, int xsize, int ysize)
{
    boxfill8(vram, xsize, COL8_000000, 0, 0, xsize - 1, ysize - 29); /*背景色*/

    boxfill8(vram, xsize, COL8_ffffff, 3, ysize - 24, 59, ysize - 24);
    boxfill8(vram, xsize, COL8_ffffff, 2, ysize - 24, 2, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 3, ysize - 4, 59, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 59, ysize - 23, 59, ysize - 5);
    boxfill8(vram, xsize, COL8_000000, 2, ysize - 3, 59, ysize - 3);
    boxfill8(vram, xsize, COL8_000000, 60, ysize - 24, 60, ysize - 3);

    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize - 3);
    boxfill8(vram, xsize, COL8_ffffff, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
    boxfill8(vram, xsize, COL8_ffffff, xsize - 3, ysize - 24, xsize - 3, ysize - 3);

    putfont8(vram, xsize, COL8_00ffff, 2, 2, font_A);   /*文字在屏幕显示后再显示*/
    putfont8(vram, xsize, COL8_00ffff, 12, 12, font_A); /*文字在屏幕显示后再显示*/
    putfont8(vram, xsize, COL8_00ffff, 22, 22, font_A); /*文字在屏幕显示后再显示*/
    putfont8(vram, xsize, COL8_00ffff, 32, 32, font_A); /*文字在屏幕显示后再显示*/
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
/*显示字符串*/
void putfonts8_asc(unsigned char *vram, int xsize, unsigned char color, int x, int y, unsigned char *str)
{
    extern char hankaku[4096];
    for (; *str != 0x00; str++)
    {
        putfont8(vram, xsize, color, x, y, hankaku + (*str) * 16);
        x += 8;
    }
    return;
}
/*显示字体*/
void putfont8(char *vram, int xsize, char color, int x, int y, char *font)
{
    int i;
    char d;
    char *p;
    for (i = 0; i < 16; i++) /*16行*/
    {
        p = vram + (y + i) * xsize + x; /*切到指定行*/
        d = font[i];
        if ((d & 0x80) != 0)
            *(p + 0) = color; /*和static font对应的话,将该位置填色*/
        if ((d & 0x40) != 0)
            *(p + 1) = color;
        if ((d & 0x20) != 0)
            *(p + 2) = color;
        if ((d & 0x10) != 0)
            *(p + 3) = color;
        if ((d & 0x08) != 0)
            *(p + 4) = color;
        if ((d & 0x04) != 0)
            *(p + 5) = color;
        if ((d & 0x02) != 0)
            *(p + 6) = color;
        if ((d & 0x01) != 0)
            *(p + 7) = color;
    }
    return;
}
/*初始化屏幕,填充颜色*/
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++)
    {
        for (x = x0; x <= x1; x++)
        {
            vram[y * xsize + x] = c; /*色号给到指定vram处*/
        }
    }
    return;
}
/*绘制鼠标*/
void init_mouse_cursor8(char *mouse, char bc_color)
{
    static char cursor[16][16] = {"**************..",
                                  "*OOOOOOOOOOO*...",
                                  "*OOOOOOOOOO*....",
                                  "*OOOOOOOOO*.....",
                                  "*OOOOOOOO*......",
                                  "*OOOOOOO*.......",
                                  "*OOOOOOO* ......",
                                  "*OOOOOOOO*......",
                                  "*OOOO**OOO*.....",
                                  "*OOO*..*OOO*....",
                                  "*OO*....*OOO*...",
                                  "*O*......*OOO*..",
                                  "**........*OOO*.",
                                  "*..........*OOO*",
                                  "............*OO*",
                                  ".............***"};
    int x, y;
    for (y = 0; y < 16; y++)
    {
        for (x = 0; x < 16; x++)
        {
            if (cursor[y][x] == '*')
            {
                *(mouse + y * 16 + x) = COL8_000084;
            }
            if (cursor[y][x] == 'O')
            {
                *(mouse + y * 16 + x) = COL8_00ffff;
            }
            if (cursor[y][x] == '.')
            {
                *(mouse + y * 16 + x) = bc_color;
            }
        }
    }
    return;
}
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
    int x, y;
    for (y = 0; y < pysize; y++) /*这里的坐标从0开始,到pysize结束,是屏幕的 ys*/
    {
        for (x = 0; x < pxsize; x++)
        {
            *(vram + (y + py0) * vxsize + (x + px0)) = *(buf + y * bxsize + x);
        }
    }
    return;
}
/*初始化gdt,idt*/
void init_gdtidt()
{
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)0x00270000; /*GDT在内存上的位置*/
    struct GATEDescriptor *idt = (struct GATE_Descriptor *)0x0026f800;
    int i;
    /*GDT初始化*/
    for (i = 0; i < 8192; i++) /*8192个段*/
    {
        set_SegDesc(gdt + i, 0, 0, 0); /*每个段上限,基址,访问权限都设为0*/
    }
    set_SegDesc(gdt + 1, 0xffffffff, 0x00000000, 0x4096); /*第一段从基址为0, 上限为0xffffffff代表CPU所能管理的全部内存*/
    set_SegDesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a); /*第二段512kb,地址0x280000,供bootpack.hrb使用*/
    load_gdtr(0xffff, 0x00270000);                        /**/
    /*IDT初始化*/
    for (i = 0;;)
    {
        set_GATEDesc(idt + i, 0, 0, 0);
    }
    load_gdtr(0x7ff, 0x0026f800);
    return;
}
/*gdt设定*/
void set_SegDesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar)
{
    if (limit > 0xfffff)
    {
        ar = ar | 0xfffff;
        limit = limit / 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
}
/*idt设定*/
void set_GATEDesc(struct GATEDescriptor *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}