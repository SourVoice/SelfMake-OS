#include "bootpack.h"
#include <stdio.h>
#include <string.h>

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
    struct TIMER *timer;
    struct TASK *task = task_now();
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

    int i, fifobuf[128];
    int *fat = (int *)memman_alloc_4k(memman, 4 * 2800);
    char s[30];
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

    /*显示提示符*/
    cons_putchar(&cons, '>', 1);

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
            if (i <= 1) /*光标用定时器*/
            {
                if (i != 0)
                {
                    timer_init(timer, &task->fifo, 0); /*下次置0 */
                    if (cons.cur_c >= 0)
                    {
                        cons.cur_c = COL8_FFFFFF;
                    }
                }
                else
                {
                    timer_init(timer, &task->fifo, 1); /*下次置1 */
                    if (cons.cur_c >= 0)
                    {
                        cons.cur_c = COL8_000000;
                    }
                }
                timer_settime(timer, 50);
            }
            if (i == 2) /*光标ON */
            {
                cons.cur_c = COL8_FFFFFF;
            }
            if (i == 3) /*光标OFF */
            {
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
                cons.cur_c = -1;
            }
            if (256 <= i && i <= 511) /*键盘数据（通过任务A） */
            {
                if (i == 8 + 256) /*退格键*/
                {
                    if (cons.cur_x > 16)
                    {
                        /*用空白擦除光标后将光标前移一位*/
                        cons_putchar(&cons, '  ', 0);
                        cons.cur_x -= 8;
                    }
                }
                else if (i == 10 + 256) /*回车键*/
                {
                    /*用空格将光标擦除*/
                    cons_putchar(&cons, ' ', 0);
                    cmdline[cons.cur_x / 8 - 2] = 0;
                    cons_newline(&cons);
                    /*执行命令*/
                    cons_runcmd(cmdline, &cons, fat, memtotal);
                    /*显示提示符*/
                    cons_putchar(&cons, '>', 1);
                }
                else /*一般字符*/
                {
                    if (cons.cur_x < 240)
                    {
                        /*显示一个字符之后将光标后移一位 */
                        cmdline[cons.cur_x / 8 - 2] = i - 256; /*输入内容存入cmdline*/
                        cons_putchar(&cons, i - 256, i);
                    }
                }
            }
            /*重新显示光标*/
            if (cons.cur_c >= 0)
            {
                boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
            }
            sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 16);
        }
    }
}
void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
    char s[2];
    s[0] = chr;
    s[1] = 0;
    if (s[0] == 0x09) /*制表符*/
    {
        for (;;)
        {
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
            cons->cur_x += 8;
            if (cons->cur_x == 8 + 248)
            {
                cons_newline(cons);
            }
            if (((cons->cur_x) & 0x1f) == 0)
            {
                break;
            }
        }
    }
    else if (s[0] == 0x0a) /*换行*/
    {
        cons_newline(cons);
    }
    else if (s[0] == 0x0d) /*回车*/
    {
    }
    else /*一般字符*/
    {
        putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
        if (move != 0) /*光标后移*/
        {
            cons->cur_x += 8;
            if (cons->cur_x == 8 + 240)
            {
                cons_newline(cons);
            }
        }
    }
    return;
}
void cons_newline(struct CONSOLE *cons)
{
    int x, y;
    struct SHEET *sheet = cons->sht;
    if (cons->cur_y < 28 + 112) /*换行*/
    {
        cons->cur_y += 16;
    }
    else /*滚动*/
    {
        for (y = 28; y < 28 + 112; y++) /*下一行给上一行*/
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
        cmd_cls(cons);
    }
    else if (strcmp(cmdline, "dir") == 0)
    {
        cmd_dir(cons);
    }
    else if (strncmp(cmdline, "type ", 5) == 0)
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
    for (x = 0; x < 224; x++) /*最多224个文件信息*/
    {
        if (finfo[x].name[0] == 0x00) /*文件名第一个字节为0x00表示不包含任何信息*/
        {
            break;
        }
        if (finfo[x].name[0] != 0xe5) /*文件名第一个字节未0xe5表示已被删除*/
        {
            if ((finfo[x].type & 0x18) == 0)
            {
                sprintf(s, "filename.ext %7d", finfo[x].size);
                for (y = 0; y < 8; y++)
                {
                    s[y] = finfo[x].name[y]; /*文件名*/
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
    struct FILEINFO *finfo = file_search(cmdline + 5, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);

    int i;
    char *p;
    if (finfo != 0) /*找到文件*/
    {
        p = (char *)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        for (i = 0; i < finfo->size; i++)
        {
            cons_putchar(cons, p[i], 1);
        }
        memman_free_4k(memman, (int)p, finfo->size);
    }
    else /*没找到文件*/
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
    struct FILEINFO *finfo = file_search("HLT.HRB", (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 244);
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    char *p;
    if (finfo != 0)
    {
        p = (char *)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER);
        farjmp(0, 1003 * 8);
        memman_free_4k(memman, (int)p, finfo->size);
    }
    else /*没有找到文件*/
    {
        putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "not find", 8);
        cons_newline(cons);
    }
    cons_newline(cons);
    return;
}
