#include "bootpack.h"
#include <stdio.h>
#include <string.h>

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
    struct TASK *task = task_now();
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

    int i, fifobuf[128];
    int *fat = (int *)memman_alloc_4k(memman, 4 * 2800);
    struct CONSOLE cons;
    char cmdline[30];
    cons.sht = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;
    *((int *)0x0fec) = (int)&cons;

    fifo32_init(&task->fifo, 128, fifobuf, task);
    cons.timer = timer_alloc();
    timer_init(cons.timer, &task->fifo, 1);
    timer_settime(cons.timer, 50);

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
                    timer_init(cons.timer, &task->fifo, 0); /*下次置0 */
                    if (cons.cur_c >= 0)
                    {
                        cons.cur_c = COL8_FFFFFF;
                    }
                }
                else
                {
                    timer_init(cons.timer, &task->fifo, 1); /*下次置1 */
                    if (cons.cur_c >= 0)
                    {
                        cons.cur_c = COL8_000000;
                    }
                }
                timer_settime(cons.timer, 50);
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
                        cons_putchar(&cons, ' ', 0);
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
    else if (cmdline[0] != 0)
    {
        if (cmd_app(cons, fat, cmdline) == 0)
        {
            cons_putstr0(cons, "wrong command.\n\n");
        }
    }
    return;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal)
{
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    char s[60];
    sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    cons_putstr0(cons, s);
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
                sprintf(s, "filename.ext %7d\n", finfo[x].size);
                for (y = 0; y < 8; y++)
                {
                    s[y] = finfo[x].name[y]; /*文件名*/
                }
                s[9] = finfo[x].ext[0];
                s[10] = finfo[x].ext[1];
                s[11] = finfo[x].ext[2];
                cons_putstr0(cons, s);
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
    char *p;
    if (finfo != 0) /*找到文件*/
    {
        p = (char *)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        cons_putstr1(cons, p, finfo->size);
        memman_free_4k(memman, (int)p, finfo->size);
    }
    else /*没找到文件*/
    {
        cons_putstr0(cons, "File not found.\n");
    }
    cons_newline(cons);
    return;
}
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    struct FILEINFO *finfo;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    char *p, *q, name[18];
    int i;
    int segsize, datsiz, esp, dathrb;

    struct SHTCTL *shtctl;
    struct SHEET *sht;
    struct TASK *task = task_now();

    for (i = 0; i < 13; i++)
    {
        if (cmdline[i] <= ' ')
        {
            break;
        }
        name[i] = cmdline[i];
    }
    name[i] = 0; /*暂将文件名的后面置位0*/

    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
    if (finfo == 0 && name[i - 1] != ' ') /*找不到文件则将文件名后面添加.hrb后重新寻找*/
    {
        name[i] = '.';
        name[i + 1] = 'H';
        name[i + 2] = 'R';
        name[i + 3] = 'B';

        name[i + 4] = 0;
        finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
    }

    if (finfo != 0) /*找到文件*/
    {
        p = (char *)memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) //判断文件开头的内容
        {
            segsize = *((int *)(p + 0x0000)); //操作系统为程序请求的数据段大小
            esp = *((int *)(p + 0x000c));     //程序启动时的初始栈
            datsiz = *((int *)(p + 0x0010));  //数据大小位置
            dathrb = *((int *)(p + 0x0014));  //有效数据起始地址
            q = (char *)memman_alloc_4k(memman, segsize);

            *((int *)0xfe8) = (int)q; /*保存代码段地址,q为代码段基址（段起始地址）*/
            set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
            set_segmdesc(gdt + 1004, segsize - 1, (int)q, AR_DATA32_RW + 0x60);
            for (i = 0; i < datsiz; i++)
            {
                q[esp + i] = p[dathrb + i];
            }
            start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0)); /*esp设置为栈开始处*/
            shtctl = (struct SHTCTL *)*((int *)0x0fe4);
            for (i = 0; i < MAX_SHEETS; i++)
            {
                sht = &(shtctl->sheets0[i]);
                if ((sht->flags & 0x11) == 0x11 && sht->task == task) //任务遗留图层
                {                                                     /*找到应用程序残留窗口*/
                    sheet_free(sht);
                }
            }
            memman_free_4k(memman, (int)q, segsize);
        }
        else //找不到Hari标志报错
        {
            cons_putstr0(cons, ".hrb file format error.\n");
        }
        memman_free_4k(memman, (int)p, finfo->size);
        cons_newline(cons);
        return 1;
    }
    return 0; /*not find*/
}
void cons_putstr0(struct CONSOLE *cons, char *s)
{
    for (; *s != 0; s++)
    {
        cons_putchar(cons, *s, 1);
    }
    return;
}
void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
    int i;
    for (i = 0; i < l; i++)
    {
        cons_putchar(cons, s[i], 1);
    }
    return;
}
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
    int ds_base = *((int *)0xfe8);
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec); /*cons地址*/
    struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0x0fe4); //shtctl地址
    struct SHEET *sht;
    char s[12];
    int data;
    int *reg = &eax + 1; //改写pushad保存的值,reg for regester
    if (edx == 1)
    {
        cons_putchar(cons, eax & 0xff, 1);
    }
    else if (edx == 2)
    {
        cons_putstr0(cons, (char *)ebx + ds_base);
        sprintf(s, "%08X\n", ebx); //字符串api调用时显示EBX的值
        cons_putstr0(cons, s);
    }
    else if (edx == 3)
    {
        cons_putstr1(cons, (char *)ebx + ds_base, ecx);
    }
    else if (edx == 4)
    {
        return &(task->tss.esp0);
    }
    else if (edx == 5)
    {
        sht = sheet_alloc(shtctl);
        sht->task = task;
        sht->flags |= 0x10;                                      //为打开的窗口启用自动关闭窗口功能
        sheet_setbuf(sht, (char *)ebx + ds_base, esi, edi, eax); //ebx作为窗口缓冲区,esi:x方向大小;edi:y方向大小;eax:透明色
        make_window8((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
        sheet_slide(sht, 100, 50);
        sheet_updown(sht, 3); //高度大于task_a
        reg[7] = (int)sht;
    }
    else if (edx == 6)
    {
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *)ebp + ds_base);
        if ((ebx & 1) == 0)
            sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
    }
    else if (edx == 7)
    {
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
        if ((ebx & 1) == 0)
            sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
    }
    else if (edx == 8)
    {
        memman_init((struct MEMMAN *)(ebx + ds_base)); //ebx: memman的地址
        ecx &= 0xfffffff0;                             //需要释放的字节数(16字节为单位)
        memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
    }
    else if (edx == 9)
    {
        ecx = (ecx + 0x0f) & 0xfffffff0; //16字节为单位取整
        reg[7] = memman_alloc((struct MEMMAN *)(ebx + ds_base), ecx);
    }
    else if (edx == 10)
    {
        ecx = (ecx + 0x0f) & 0xfffffff0;
        memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
    }
    else if (edx == 11)
    {
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        sht->buf[sht->bxsize * edi + esi] = eax;
        if ((ebx & 1) == 0)
            sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
    }
    else if (edx == 12)
    {
        sht = (struct SHEET *)ebx;
        sheet_refresh(sht, eax, ecx, esi, edi);
    }
    else if (edx == 13)
    {
        sht = (struct SHEET *)(ebx & 0xfffffffe);
        hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
        if ((ebx & 1) == 0)
        {
            sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
        }
    }
    else if (edx == 14)
    {
        sheet_free((struct SHEET *)ebx);
    }
    else if (edx == 15)
    {
        for (;;)
        {
            io_cli();
            if (fifo32_status(&task->fifo) == 0) //fifo为空,任务休眠
            {
                if (eax != 0)
                {
                    task_sleep(task);
                }
                else
                {
                    io_sti();
                    reg[7] = -1;
                    return 0;
                }
            }
            data = fifo32_get(&task->fifo);
            io_sti();
            if (data <= 1)
            {
                timer_init(cons->timer, &task->fifo, 1); //cons->timer为专门控制光标闪烁的计时器
                timer_settime(cons->timer, 50);
            }
            if (data == 2) //光标on
            {
                cons->cur_c = COL8_FFFFFF; //黑色
            }
            if (data == 3) //光标off
            {
                cons->cur_c = -1;
            }
            if (256 <= data && data <= 511) //键盘数据
            {
                reg[7] = data - 256;
                return 0;
            }
        }
    }
    return 0; //返回0程序继续运行
}
void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
    int i, x, y, len, dx, dy;

    dx = x1 - x0;
    dy = y1 - y0;
    x = x0 << 10; //*1024
    y = y0 << 10;
    if (dx < 0)
    {
        dx = -dx;
    }
    if (dy < 0)
    {
        dy = -dy;
    }
    if (dx >= dy) //斜率小于1
    {
        len = dx + 1;
        if (x0 > x1)
            dx = -1024;
        else
            dx = 1024;
        if (y0 <= y1)
            dy = ((y1 - y0 + 1) << 10) / len;
        else
            dy = ((y1 - y0 - 1) << 10) / len;
    }
    else
    {
        len = dy + 1;
        if (y0 > y1)
            dy = -1024;
        else
            dy = 1024;
        if (x0 <= x1)
            dx = ((x1 - x0 + 1) << 10) / len;
        else
            dx = ((x1 - x0 - 1) << 10) / len;
    }
    for (i = 0; i < len; i++)
    {
        sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
        x += dx;
        y += dy;
    }
    return;
}
int *inthandler0c(int *esp)
{
    struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec);
    struct TASK *task = task_now();
    char s[30];
    cons_putstr0(cons, "\nINT 0C: \n Stack Exception.\n");
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    return &(task->tss.esp0);
}

int *inthandler0d(int *esp)
{
    struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec);
    struct TASK *task = task_now();
    char s[30];
    cons_putstr0(cons, "\nINT 0D: \n General Protected Exception.\n");
    sprintf(s, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, s);
    return &(task->tss.esp0);
}
