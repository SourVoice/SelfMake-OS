#include "bootpack.h"
#include <stdio.h>
#include <string.h>
struct CONSOLE
{
    struct SHEET *sht;
    int cur_x, cur_y, cur_c;
};

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
    struct TIMER *timer;
    struct TASK *task = task_now();
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

    int i, fifobuf[128];
    int *fat = (int *)memman_alloc_4k(memman, 4 * 2800);
    char s[30], *p;
    int x = 0, y = 0;
    struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    struct CONSOLE cons;
    char cmdline[30];
    cons.sht = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;

    fifo32_init(&task->fifo, 128, fifobuf, task);
    timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);

    file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

    /*��ʾ��ʾ��*/
    putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

    for (;;)
    {
        io_cli();
        if (fifo32_status(&task->fifo) == 0)
        {
            task_sleep(task);
            io_sti();
        }
        else
        {
            i = fifo32_get(&task->fifo);
            io_sti();
            if (i <= 1) /*����ö�ʱ��*/
            {
                if (i != 0)
                {
                    timer_init(timer, &task->fifo, 0); /*�´���0 */
                    if (cursor_c >= 0)
                    {
                        cursor_c = COL8_FFFFFF;
                    }
                }
                else
                {
                    timer_init(timer, &task->fifo, 1); /*�´���1 */
                    if (cursor_c >= 0)
                    {
                        cursor_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }
            if (i == 2) /*���ON */
            {
                cursor_c = COL8_FFFFFF;
            }
            if (i == 3) /*���OFF */
            {
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
                cursor_c = -1;
            }
            if (256 <= i && i <= 511) /*�������ݣ�ͨ������A�� */
            {
                if (i == 8 + 256) /*�˸��*/
                {
                    if (cursor_x > 16)
                    {
                        /*�ÿհײ������󽫹��ǰ��һλ*/
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                        cursor_x -= 8;
                    }
                }
                else if (i == 10 + 256) /*�س���*/
                {
                    /*�ÿո񽫹�����*/
                    putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                    cmdline[cursor_x / 8 - 2] = 0;
                    cursor_y = cons_newline(cursor_y, sheet);
                    /*ִ������*/
                    if (strcmp(cmdline, "mem") == 0) /* mem����*/
                    {
                        mem_command(memman, memtotal, s, &cursor_y, sheet);
                    }
                    else if (strcmp(cmdline, "cls") == 0) /* cls����*/
                    {
                        cls_command(sheet, &cursor_y, x, y);
                    }
                    else if (strcmp(cmdline, "dir") == 0 || strcmp(cmdline, "ls") == 0) /* dir���� */
                    {
                    }
                    else if (strncmp(cmdline, "type ", 5) == 0) /*type����(���Ƚ�ǰ����ַ�)*/
                    {
                    }
                    else if (strcmp(cmdline, "hlt") == 0)
                    {
                    }
                    else if (cmdline[0] != 0) /*�������Ҳ���ǿ��� */
                    {
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    }
                    /*��ʾ��ʾ��*/
                    putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
                    cursor_x = 16;
                }
                else /*һ���ַ�*/
                {
                    if (cursor_x < 240)
                    {
                        /*��ʾһ���ַ�֮�󽫹�����һλ */
                        s[0] = i - 256;
                        s[1] = 0;
                        cmdline[cursor_x / 8 - 2] = i - 256; /*�������ݴ���cmdline*/
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                        cursor_x += 8;
                    }
                }
            }
            /*������ʾ���*/
            if (cursor_c >= 0)
            {
                boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
            }
            sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
        }
    }
}
void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
    char s[2];
    s[0] = chr;
    s[1] = 0;
    if (s[0] == 0x09) /*�Ʊ��*/
    {
        for (;;)
        {
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
            cons->cur_x += 8;
            if (cons->cur_x == 8 + 248)
            {
                cons_newline(cons);
            }
        }
    }
}
int cons_newline(struct CONSOLE *cons)
{
    int x, y;
    struct SHEET *sheet = cons->sht;
    if (cons->cur_y < 28 + 112) /*����*/
    {
        cons->cur_y += 16;
    }
    else /*����*/
    {
        for (y = 28; y < 28 + 112; y++) /*��һ�и���һ��*/
        {
            for (x = 8; x < 8 + 240; x++)
            {
                sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
            }
        }
        for (y = 28 + 112; y < 28 + 128; y++)
        {
            for (x = 8; x < 8 + 240; x++)
            {
                sheet->buf[x + y * sheet->bxsize] = COL8_000000;
            }
        }
        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    }
    cons->cur_x = 8;
    return;
}
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
    if (strcmp(cmdline, "mem") == 0)
    {
        cmd_mem(cons, memtotal);
    }
    else if (strcmp(cmdline, "cls") == 0)
    {
        cmd_cls(cons, memtotal);
    }
    else if (strcmp(cmdline, "dir") == 0)
    {
        cmd_dir(cons);
    }
    else if (strncmp(cmdline, "type", 5) == 0)
    {
        cmd_type(cons, fat, cmdline);
    }
    else if (strcmp(cmdline, "hlt") == 0)
    {
        cmd_hlt(cons, fat);
    }
    else if (cmdline[0] != 0)
    {
        putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Wrong Command.", 14);
        cons_newline(cons);
        cons_newline(cons);
    }
    return;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    char s[30];
    sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
    putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
    cons_newline(cons);
    sprintf(s, "free %dKB", memman_total(memman) / 1024);
    putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
    cons_newline(cons);
    cons_newline(cons);
    return;
}

void cmd_cls(struct CONSOLE *cons)
{
    int x, y;
    struct SHEET *sheet = cons->sht;
    for (y = 28; y < 28 + 128; y++)
    {
        for (x = 8; x < 8 + 240; x++)
        {
            sheet->buf[x + y * sheet->bxsize] = COL8_000000;
        }
    }
    sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    cons->cur_y = 28;
    return;
}

void cmd_dir(struct CONSOLE *cons)
{
    struct SHEET *sheet = cons->sht;
    struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
    int x, y;
    char s[30];
    for (x = 0; x < 224; x++) /*���224���ļ���Ϣ*/
    {
        if (finfo[x].name[0] == 0x00) /*�ļ�����һ���ֽ�Ϊ0x00��ʾ�������κ���Ϣ*/
        {
            break;
        }
        if (finfo[x].name[0] != 0xe5) /*�ļ�����һ���ֽ�δ0xe5��ʾ�ѱ�ɾ��*/
        {
            if ((finfo[x].type & 0x18) == 0)
            {
                sprintf(s, "filename.ext %7d", finfo[x].size);
                for (y = 0; y < 8; y++)
                {
                    s[y] = finfo[x].name[y]; /*�ļ���*/
                }
                s[9] = finfo[x].ext[0];
                s[10] = finfo[x].ext[1];
                s[11] = finfo[x].ext[2];
                putfonts8_asc_sht(sheet, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
                cons_newline(cons);
            }
        }
    }
    cons_newline(cons);
    return;
}
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline)
{

    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);

    int x, y, i;
    char *p, s[30];
    if (finfo != 0) /*�ҵ��ļ�*/
    {
        p = (char *)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        for (i = 0; i < finfo->size; i++)
        {
            cons_putchar(cons, p[i], 1);
        }
        memman_alloc_4k(memman, (int)p, finfo->size);
    }
    else /*û�ҵ��ļ�*/
    {
        putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
        cons_newline(cons);
    }
    cons_newline(cons);
    return;
}
void cmd_hlt(struct CONSOLE *cons, int *fat)
{
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    struct FILEINFO *finfo = file_search();
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    char *p;
    if (finfo != 0)
    {
        p = (char *)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        set_gatedesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER);
        farjmp(0, 1003 * 8);
        memman_alloc_4k(memman, (int)p, finfo->size);
    }
    else /*û���ҵ��ļ�*/
    {
        putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "not find", 8);
        cons_newline(cons);
    }
    cons_newline(cons);
}
