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
/*�����Ļ����*/
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /*����: vram��ַ,��С,ɫ��,λ��x,λ��y,*/
/* ���ػ�������ʹ�ýṹ����Ϊ�˽�Լ�ռ��*/
/*0x0ff0��������*/
struct BOOTINFO
{
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};
//font����
static char font_A[16] = {
    0x00, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
    0x24, 0x7e, 0x42, 0x42, 0xe7, 0x00, 0x00}; /*A��ĸ�����ص���*/
void putfont8(char *vram, int xsize, char color, int x, int y, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, unsigned char color, int x, int y, unsigned char *str);
/*��ʼ�����*/
void init_mouse_cursor8(char *mouse, char bc_color);
/*���ƿ�(��buf�е���ɫ����vram)*/
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize,
                 int px0, int py0, char *buf, int bxsize); /*p for picture, vxsize = vram��ƫ��, pxsize,pysize = ͼ���С,px0,py0 = ͼ��λ��,bxsize = ͼ��ƫ��*/
/*GDTȫ�ֶκż�¼��*/
struct SegmentDescriptor
{
    short limit_low, base_low;
    char base_mid, access_right; /*�ڴ����ʼ��ַ�Ͷθ�������GDTR�Ĵ���,����üĴ�����д��Ҫ�õ���ຯ��*/
    short limit_high, base_high;
};
/*�жϼ�¼��*/
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
    init_palette(); /*��ɫ��*/

    char *vram;
    char s[40];
    char mouse[256]; /*256�ֽڵ�ָ��*/
    int xsize, ysize;
    struct BOOTINFO *binfo;
    binfo = (struct BOOTINFO *)0x0ff0; /*ָ��binfo����0x0ff0��ַ.*/
    xsize = binfo->scrnx;
    ysize = binfo->scrny;
    vram = binfo->vram; /*����ע��,vramҲ��һ��ָ�����,ָ��0x0ff8*/

    init_screen(vram, xsize, ysize); /*��Ļ���ַ�װ����*/
    putfonts8_asc(vram, xsize, COL8_840000, 20, 20, "what?");
    putfonts8_asc(vram, xsize, COL8_840000, 40, 40, "what?");
    putfonts8_asc(vram, xsize, COL8_840000, 60, 60, "what?");
    /*������ʾ*/
    sprintf(s, "scrnx = %d", xsize);                    /*��������,����sprintf*/
    putfonts8_asc(vram, xsize, COL8_00ffff, 16, 64, s); /*s���ڼ�¼�ַ����ĵ�ַ*/
    /*���*/
    init_mouse_cursor8(mouse, COL8_000000);
    putblock8_8(vram, xsize, 16, 16, 160, 100, mouse, 16);
    for (;;)
        io_hlt(); /*ִ�л���е�ͣ������*/
}
void init_screen(unsigned char *vram, int xsize, int ysize)
{
    boxfill8(vram, xsize, COL8_000000, 0, 0, xsize - 1, ysize - 29); /*����ɫ*/

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

    putfont8(vram, xsize, COL8_00ffff, 2, 2, font_A);   /*��������Ļ��ʾ������ʾ*/
    putfont8(vram, xsize, COL8_00ffff, 12, 12, font_A); /*��������Ļ��ʾ������ʾ*/
    putfont8(vram, xsize, COL8_00ffff, 22, 22, font_A); /*��������Ļ��ʾ������ʾ*/
    putfont8(vram, xsize, COL8_00ffff, 32, 32, font_A); /*��������Ļ��ʾ������ʾ*/
}

/*��ʼ����ɫ��*/
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
    /*�趨��ɫ��ǰ�����ж�,����ȶ����жϱ�־*/
    eflags = io_load_eflags(); /*��¼�ж���ɱ�־*/
    io_cli();                  /*�ж���ɱ�־Ϊ0,��ֹ�ж�(��ʵ����ִ���ж�)*/
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++)
    {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags); /*�ָ��жϱ�־*/
    return;
}
/*��ʾ�ַ���*/
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
/*��ʾ����*/
void putfont8(char *vram, int xsize, char color, int x, int y, char *font)
{
    int i;
    char d;
    char *p;
    for (i = 0; i < 16; i++) /*16��*/
    {
        p = vram + (y + i) * xsize + x; /*�е�ָ����*/
        d = font[i];
        if ((d & 0x80) != 0)
            *(p + 0) = color; /*��static font��Ӧ�Ļ�,����λ����ɫ*/
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
/*��ʼ����Ļ,�����ɫ*/
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++)
    {
        for (x = x0; x <= x1; x++)
        {
            vram[y * xsize + x] = c; /*ɫ�Ÿ���ָ��vram��*/
        }
    }
    return;
}
/*�������*/
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
    for (y = 0; y < pysize; y++) /*����������0��ʼ,��pysize����,����Ļ�� ys*/
    {
        for (x = 0; x < pxsize; x++)
        {
            *(vram + (y + py0) * vxsize + (x + px0)) = *(buf + y * bxsize + x);
        }
    }
    return;
}
/*��ʼ��gdt,idt*/
void init_gdtidt()
{
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)0x00270000; /*GDT���ڴ��ϵ�λ��*/
    struct GATEDescriptor *idt = (struct GATE_Descriptor *)0x0026f800;
    int i;
    /*GDT��ʼ��*/
    for (i = 0; i < 8192; i++) /*8192����*/
    {
        set_SegDesc(gdt + i, 0, 0, 0); /*ÿ��������,��ַ,����Ȩ�޶���Ϊ0*/
    }
    set_SegDesc(gdt + 1, 0xffffffff, 0x00000000, 0x4096); /*��һ�δӻ�ַΪ0, ����Ϊ0xffffffff����CPU���ܹ����ȫ���ڴ�*/
    set_SegDesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a); /*�ڶ���512kb,��ַ0x280000,��bootpack.hrbʹ��*/
    load_gdtr(0xffff, 0x00270000);                        /**/
    /*IDT��ʼ��*/
    for (i = 0;;)
    {
        set_GATEDesc(idt + i, 0, 0, 0);
    }
    load_gdtr(0x7ff, 0x0026f800);
    return;
}
/*gdt�趨*/
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
/*idt�趨*/
void set_GATEDesc(struct GATEDescriptor *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}