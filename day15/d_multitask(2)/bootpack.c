#include <stdio.h>
#include "bootpack.h"
/*extern+������ʽ��ʾ���������ⲿ(����Դ�ļ�)(����̫�������©�����뵼�²���ͨ��,����С�Ķ�makefile)*/
/*bootpack.c*/
void make_window(unsigned char *buf, int xsize, int ysize, char *title);          /*��ʱ���ƴ���*/
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int color); /*�����������뱳��*/
void task_b_main(void);                                                           /*b����ִ������*/
struct TSS32                                                                      /*task status segment(TSS)����״̬��,ͬ���ڴ��һ��*/
{
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3; /*������������������Ϣ*/
    int eip, eax, ecx, edx, ebx, esp, ebp, esi, edi;    /*32λ�Ĵ���,����eflags*/
    int eflags;
    int es, cs, ss, ds, fs, gs; /*16λ�Ĵ���*/
    int ldtr, iomap;
};
void HariMain(void)
{
    struct FIFO32 fifo;
    static char keytable[0x54] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0, 0,
                                  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0, 0, 'A', 'S',
                                  'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C',
                                  'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
                                  '2', '3', '0', '.'};

    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    int xsize = binfo->scrnx, ysize = binfo->scrny;
    char *vram = binfo->vram;
    char s[40];
    int fifobuf[128]; /*mousebuf ��Ϣ�жϻ���������������*/
    int task_b_esp;   /*����b��ջ*/
    struct TIMER *timer, *timer2, *timer3;
    struct TIMER *timer_ts;
    int mouse_x = 0, mouse_y = 0;
    int cursor_x, cursor_c; /*cursor_x�����ʾλ�ñ���,û����һ���ַ��ñ�������8,cursor_c��ʾ�����ɫ,ÿ0.5s�仯һ��*/
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    struct TSS32 tss_a, tss_b;

    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR; /*memman��Ҫ32KB��С���ڴ洢�ڴ�ռ���÷�����Ϣ������ʹ�ô�0x003c0000�ŵ�ַ�Ժ�*/
    struct MOUSE_DEC mdec;                                /*d����decode,phase�׶�,��¼���ݽ��ܵĽ׶�*/
    int data;
    unsigned int memtotal;

    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse, *sht_win;
    unsigned char *buf_back, buf_mouse[256], *buf_win; /*buf_mouse ͼ������*/

    init_gdtidt();

    init_pic();
    io_sti(); /*�ж�IF��Ϊ1��������CPU�ж�*/

    /*��������ʼ��*/
    fifo32_init(&fifo, 128, fifobuf);

    init_pit(); /*��ʱ������ж�*/
    /*�ж�*/
    init_keyboard(&fifo, 256);       /*���̽�����ջ��*/
    enable_mouse(&fifo, 512, &mdec); /*��������ջ��*/

    io_out8(PIC0_IMR, 0xf8); /*���ż����ж�(���Ķ˿ں�ʹPIC1��PIT�ͼ��̾�Ϊ���)*/
    io_out8(PIC1_IMR, 0xef); /*��������ж�*/

    /*ʱ���ж�����*/
    timer_ts = timer_alloc();
    timer_init(timer_ts, &fifo, 2);
    timer_settime(timer_ts, 2);

    timer = timer_alloc();
    timer_init(timer, &fifo, 10);
    timer_settime(timer, 1000);
    timer2 = timer_alloc();
    timer_init(timer2, &fifo, 3);
    timer_settime(timer2, 300);
    timer3 = timer_alloc();
    timer_init(timer3, &fifo, 1);
    timer_settime(timer3, 50);

    /*�ڴ����*/
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    /*����*/
    init_palette();
    shtctl = shtctl_init(memman, vram, binfo->scrnx, binfo->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    sht_win = sheet_alloc(shtctl);
    buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* û��͸��ɫ */
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);                   /* ͸��ɫ��99 */
    sheet_setbuf(sht_win, buf_win, 160, 52, -1);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(buf_mouse, 99); /* ����ɫ��99 */
    make_window(buf_win, 160, 52, "window");
    putfonts8_asc(buf_win, 160, COL8_000000, 24, 44, "This OS!");
    sheet_slide(sht_back, 0, 0);
    mouse_x = (binfo->scrnx - 16) / 2; /* ����ʾ�ڻ����������������� */
    mouse_y = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(sht_mouse, mouse_x, mouse_y);
    sheet_slide(sht_win, 80, 72);
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 1);
    sheet_updown(sht_mouse, 2);
    sprintf(s, "(%3d, %3d)", mouse_x, mouse_y);
    putfonts_asc_sht(sht_back, 0, 0, COL8_ffffff, COL8_008484, s, 10);
    sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts_asc_sht(sht_back, 0, 32, COL8_ffffff, COL8_008484, s, 40); /* ˢ������ */

    cursor_x = 8;
    cursor_c = COL8_ffffff;
    make_textbox8(sht_win, 8, 28, 144, 16, COL8_ffffff);

    tss_a.ldtr = 0;
    tss_a.iomap = 0x40000000;
    tss_b.ldtr = 0;
    tss_b.iomap = 0x40000000;

    set_segmdesc(gdt + 3, 103, (int)&tss_a, AR_TSS32); /*tss+a������gdt����,�γ�103�ֽ�*/
    set_segmdesc(gdt + 4, 103, (int)&tss_b, AR_TSS32);
    task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8; /*����b��ջ�Ϸ���64kb*/
    load_tr(3 * 8);                                                  /*a����(gdt 3��)*/

    tss_b.eip = (int)&task_b_main; /*eip�Ĵ����м�¼��Ҫ��ת����Ŀ�ʼλ��,����ֱ�Ӹ���task_b_main�ĵ�ַ*/
    tss_b.eflags = 0x00000202;     /* IF = 1; */
    tss_b.eax = 0;
    tss_b.ecx = 0;
    tss_b.edx = 0;
    tss_b.ebx = 0;
    tss_b.esp = task_b_esp;
    tss_b.ebp = 0;
    tss_b.esi = 0;
    tss_b.edi = 0;
    tss_b.es = 1 * 8;
    tss_b.cs = 2 * 8;
    tss_b.ss = 1 * 8;
    tss_b.ds = 1 * 8;
    tss_b.fs = 1 * 8;
    tss_b.gs = 1 * 8;

    for (;;)
    {
        io_cli();                      /*�����ж�(һ��ִֻ��һ���жϴ���)*/
        if (fifo32_status(&fifo) == 0) /*��黺����,Ϊ��ֱ�ӽ���ͣ��*/
        {
            io_stihlt();
        }
        else
        {
            data = fifo32_get(&fifo);
            io_sti();
            if (data == 2)
            {
                farjmp(0, 4 * 8);
                timer_settime(timer_ts, 2);
            }
            else if (256 <= data && data <= 511)
            {
                sprintf(s, "%02x", data - 256);
                putfonts_asc_sht(sht_back, 0, 16, COL8_ffffff, COL8_008484, s, 2);
                if (data < 256 + 0x54)
                {
                    if (keytable[data - 256] != 0 && cursor_x < 144)
                    {
                        s[0] = keytable[data - 256];
                        s[1] = 0;
                        putfonts_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_ffffff, s, 1);
                        cursor_x += 8;
                    }
                    if (data == 256 + 0x0e && cursor_x > 8) /*backspace��,*/
                    {
                        /*�ÿո���ѹ����ȥ��,�������ι��*/
                        putfonts_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_ffffff, " ", 1);
                        cursor_x -= 8;
                    }
                    /*���*/
                    boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                    sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
                }
            }
            else if (512 <= data && data <= 767)
            {
                if (mouse_decode(&mdec, data - 512) != 0)
                {
                    sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) /*���λΪ1*/
                    {
                        s[1] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0)
                    {
                        s[3] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0)
                    {
                        s[2] = 'C';
                    }
                    putfonts_asc_sht(sht_back, 32, 16, COL8_ffffff, COL8_008484, s, 15);
                    /*�ƶ�ָ��*/
                    mouse_x += mdec.x;
                    mouse_y += mdec.y;
                    if (mouse_x < 0)
                    {
                        mouse_x = 0;
                    }
                    if (mouse_y < 0)
                    {
                        mouse_y = 0;
                    }
                    if (mouse_x > xsize - 1) /*ʹ�����������,����refreshsubд��ͼ��ʱ��Ȼ��������,��Ҫ��ˢ���»������������*/
                    {
                        mouse_x = xsize - 1;
                    }
                    if (mouse_y > ysize - 1)
                    {
                        mouse_y = ysize - 1;
                    }
                    sprintf(s, "(%3d,%3d)", mouse_x, mouse_y);
                    putfonts_asc_sht(sht_back, 0, 0, COL8_ffffff, COL8_008484, s, 10);
                    sheet_slide(sht_mouse, mouse_x, mouse_y); /*��refresh*/
                    if ((mdec.btn & 0x01) != 0)               /*�������,�ƶ�sht_win*/
                    {
                        sheet_slide(sht_win, mouse_x - 80, mouse_y - 8);
                    }
                }
            }
            else if (data == 10)
            {
                putfonts_asc_sht(sht_back, 0, 64, COL8_ffffff, COL8_008484, "10[sec]", 7);
            }
            else if (data == 3)
            {
                putfonts_asc_sht(sht_back, 0, 80, COL8_ffffff, COL8_008484, "3[sec]", 6);
            }
            else if (data <= 1)
            {
                if (data != 0)
                {
                    timer_init(timer3, &fifo, 0);
                    cursor_c = COL8_000000;
                }
                else
                {
                    timer_init(timer, &fifo, 1);
                    cursor_c = COL8_ffffff;
                }
                boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
                sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
            }
        }
    }
}
void make_window(unsigned char *buf, int xsize, int ysize, char *title)
{
    static char closebtn[14][16] =
        {"OOOOOOOOOOOOOOO@",
         "OQQQQQQQQQQQQQ$@",
         "OQQQQQQQQQQQQQ$@",
         "OQQQ@@QQQQ@@QQ$@",
         "OQQQQ@@QQ@@QQQ$@",
         "OQQQQQ@@@@QQQQ$@",
         "OQQQQQQ@@QQQQQ$@",
         "OQQQQQ@@@@QQQQ$@",
         "OQQQQ@@QQ@@QQQ$@",
         "OQQQ@@QQQQ@@QQ$@",
         "OQQQQQQQQQQQQQ$@",
         "OQQQQQQQQQQQQQ$@",
         "O$$$$$$$$$$$$$$@",
         "@@@@@@@@@@@@@@@@"};
    int x, y;
    char c;
    boxfill8(buf, xsize, COL8_c6c6c6, 0, 0, xsize - 1, 0);
    boxfill8(buf, xsize, COL8_ffffff, 1, 1, xsize - 2, 1);
    boxfill8(buf, xsize, COL8_c6c6c6, 0, 0, 0, ysize - 1);
    boxfill8(buf, xsize, COL8_ffffff, 1, 1, 1, ysize - 2);
    boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_c6c6c6, 2, 2, xsize - 3, ysize - 3);
    boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20);
    boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
    putfonts8_asc(buf, xsize, COL8_ffffff, 24, 4, title);
    for (y = 0; y < 14; y++)
    {
        for (x = 0; x < 16; x++)
        {
            c = closebtn[y][x];
            if (c == '@')
            {
                c = COL8_000000;
            }
            else if (c == '$')
            {
                c = COL8_848484;
            }
            else if (c == 'Q')
            {
                c = COL8_c6c6c6;
            }
            else
            {
                c = COL8_ffffff;
            }
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    return;
}
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int color)
{
    int x1 = x0 + sx, y1 = y0 + sy;
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_ffffff, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_ffffff, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
    boxfill8(sht->buf, sht->bxsize, COL8_c6c6c6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, COL8_c6c6c6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
    boxfill8(sht->buf, sht->bxsize, color,
             x0 - 1, y0 - 1, x1 + 0, y1 + 0);
    return;
}
void task_b_main(void)
{
    struct FIFO32 fifo;
    struct TIMER *timer_ts;
    int data, fifobuf[128];

    fifo32_init(&fifo, 128, fifobuf);
    timer_ts = timer_alloc();
    timer_init(timer_ts, &fifo, 1); /*timer��1��¼*/
    timer_settime(timer_ts, 2);

    for (;;)
    {
        io_cli();
        if (fifo32_status(&fifo) == 0) /*������ȡ����ֵ��ֹ*/
        {
            io_stihlt();
        }
        else
        {
            data = fifo32_get(&fifo); /*��fifo�ж���1*/
            io_sti();
            if (data == 1)
            {
                farjmp(0, 3 * 8);
                timer_settime(timer_ts, 1);
            }
        }
    }
}
