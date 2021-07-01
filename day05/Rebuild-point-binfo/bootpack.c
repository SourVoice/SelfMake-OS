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
void io_hlt(void);
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void io_cli(void);
void io_out8(int port, int data);
void init_screen(unsigned char *vram, int xsize, int ysize);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1); /*����: vram��ַ,��С,ɫ��,λ��x,λ��y,*/

void HariMain(void)
{
    init_palette(); /*��ɫ��*/

    char *vram;
    int xsize, ysize;
    short *binfo_scrnx, *binfo_scrny;
    int *binfo_vram;
    binfo_scrnx = (short *)0x0ff4; /*����ָ����ڴ���ȡbinfo(asmhead)�ﶨ�����Ϣ*/
    binfo_scrny = (short *)0x0ff6;
    binfo_vram = (int *)0x0ff8;
    xsize = *binfo_scrnx;
    ysize = *binfo_scrny;
    vram = (char *)*binfo_vram;

    init_screen(vram, xsize, ysize); /*��Ļ���ַ�װ����*/
    for (;;)
        io_hlt(); /*ִ�л���е�ͣ������*/
}
void init_screen(unsigned char *vram, int xsize, int ysize)
{
    boxfill8(vram, xsize, COL8_ffffff, 20, 20, 120, 120);
    boxfill8(vram, xsize, COL8_ff0000, 40, 40, 140, 140);
    boxfill8(vram, xsize, COL8_848484, 60, 60, 160, 160);

    // boxfill8(vram, xsize, COL8_840084, 0, 0, xsize - 1, ysize - 29);

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
}

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