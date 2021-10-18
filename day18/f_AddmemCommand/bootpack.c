/* bootpack*/

#include "bootpack.h"
#include <stdio.h>
#define KEYCMD_LED 0xed

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act); /*窗口标题栏代码*/
void console_task(struct SHEET *sheet);
void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	struct FIFO32 fifo, keycmd; /*keycmd专门管理发送数据的顺序*/
	char s[40];
	int fifobuf[128], keycmd_buf[32];
	struct TIMER *timer;
	int mx, my, i, cursor_x, cursor_c;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons; /*[]优先级高*/
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	static char keytable0[0x80] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0, 0,
								   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0, 0, 'A', 'S',
								   'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V',
								   'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
								   0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								   0, 0, 0, 0x5c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x5c, 0, 0};
	static char keytable1[0x80] = {0, 0, '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0, 0, '}', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
								   0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								   0, 0, 0, '_', 0, 0, 0, 0, 0, 0, 0, 0, 0, '|', 0, 0};
	struct TASK *task_a, *task_cons;
	int key_to = 0; /*记录键盘输入应该发送到哪里*/
	int key_shift = 0;
	int key_leds = (binfo->leds >> 4) & 7; /*binfo->leds为从bios中取得的键盘状态,第6位为capslock状态*/
	int keycmd_wait = -1 /*表示向键盘控制器发送数据的状态;-1表示键盘控制器可以发送指令,否则处于等待状态,所发送的数据存于keycmd_wait*/;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */
	fifo32_init(&fifo, 128, fifobuf, 0);
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* 设定PIT和PIC1以及键盘为许可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a; /*task_a为初始任务,且在下面设置了休眠在这里使其唤醒*/
	task_run(task_a, 1, 0);

	/*sht_back*/
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 没有透明色 */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/*sht_cons*/
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* 无透明色 */
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_cons->tss.eip = (int)&console_task;
	task_cons->tss.es = 2 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *)(task_cons->tss.esp + 4)) = (int)sht_cons;
	task_run(task_cons, 2, 2);

	/*sht_win*/
	sht_win = sheet_alloc(shtctl);
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); /* 没有透明色 */
	make_window8(buf_win, 144, 52, "window", 1); /*160-144-128(-16)*/
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	/*sht_mouse*/
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); /* 透明色号99 */
	init_mouse_cursor8(buf_mouse, 99);				/* 背景色号99 */
	mx = (binfo->scrnx - 16) / 2;					/* 按显示在画面中央来计算坐标 */
	my = (binfo->scrny - 28 - 16) / 2;

	/*图层显示*/
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_cons, 32, 4);
	sheet_slide(sht_win, 64, 56);
	sheet_slide(sht_mouse, mx, my);

	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons, 2);
	sheet_updown(sht_win, 3);
	sheet_updown(sht_mouse, 4);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	/*避免和当前键盘状态冲突*/
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	for (;;)
	{
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) /*keycmd中有数据,且keycmd_wait=-1(表示可以控制器可以接收数据)*/
		{
			keycmd_wait = fifo32_get(&keycmd); /*取得向键盘控制器发送的数据*/
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0)
		{
			task_sleep(task_a); /*fifo为空时,执行task_a的休眠*/
			io_sti();
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) /* 键盘数据*/
			{
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 0x80 + 256) /*将按键编码转换为字符编码*/
				{

					if (key_shift == 0) /*因为每次只输入传送数据i一次,所以提前进行shift响应,转换对应的字符编码*/
					{
						s[0] = keytable0[i - 256];
					}
					else
					{
						s[0] = keytable1[i - 256];
					}
				}
				else /*未识别的置为0*/
				{
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') /*输入为英文字母时判断大小写*/
				{
					/*判断小写*/
					if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)) /*caps松开且shift松开||大写打开且shift按下*/
					{
						s[0] += 0x20; /*大写转小写*/
					}
				}
				if (s[0] != 0) /*一般字符输入处理*/
				{
					if (key_to == 0) /*key_to==0数据发送到任务A的缓冲区*/
					{
						if (cursor_x < 128) /*字符插入后光标后移*/
						{
							s[1] = 0; /*s[0]为字符,s[0]代表结束符*/
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 2);
							cursor_x += 8;
						}
					}
					else /*数据发送到console*/
					{
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) /* 退格键 */
				{
					if (key_to == 0) /*key_to==0数据发送到任务A的缓冲区*/
					{
						if (cursor_x > 8)
						{
							/* 用空格键把光标消去后，后移1次光标 */
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					}
					else /*数据发送到console*/
					{
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if (i == 256 + 0x0f) /*Tab键*/
				{
					if (key_to == 0) /*当前为win窗口时tab按下*/
					{
						key_to = 1;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;																		  /*不显示光标*/
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43); /*sht_win框中光标小时*/
						fifo32_put(&task_cons->fifo, 2);													  /*在console时向fifo存入2,表示光标开启*/
					}
					else /*key_to==1时表示数据发送到console窗口,注意这里按下了tab键*/
					{
						key_to = 0;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;			 /*win显示光标*/
						fifo32_put(&task_cons->fifo, 3); /*不在console时向fifo存入3,表示光标关闭*/
					}
					sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				if (i == 256 + 0x2a) /*左shift按下*/
				{
					key_shift |= 1; /*0 | 1 = 1, 2 | 1 =3 (左右shift同时按下)*/
				}
				if (i == 256 + 0x36) /*右shift按下*/
				{
					key_shift |= 2; /*0 | 2 = 2, 1 | 2 = 3 (左右shift同时按下)*/
				}
				if (i == 256 + 0xaa) /*左shift抬起*/
				{
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) /*左shift抬起*/
				{
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a) /*capslock*/
				{
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) /*numLock*/
				{
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) /*scrollLock*/
				{
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x1c) /*回车键*/
				{
					if (key_to != 0) /*发送数据至console*/
					{
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if (i == 256 + 0xfa) /*键盘成功接收到数据*/
				{
					keycmd_wait = -1; /*恢复为-1继续发送下一个数据*/
				}
				if (i == 256 + 0xfe) /*键盘未成功接收到数据*/
				{
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				/* 光标再显示 */
				if (cursor_c >= 0)
				{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
			else if (512 <= i && i <= 767) /* 鼠标数据*/
			{
				if (mouse_decode(&mdec, i - 512) != 0) /* 已经收集了3字节的数据，所以显示出来 */
				{
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0)
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
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					/* 移动光标 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
					{
						mx = 0;
					}
					if (my < 0)
					{
						my = 0;
					}
					if (mx > binfo->scrnx - 1)
					{
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1)
					{
						my = binfo->scrny - 1;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my); /* 包含sheet_refresh含sheet_refresh */
					if ((mdec.btn & 0x01) != 0)		/* 按下左键、移动sht_win */
					{
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			}
			else if (i <= 1) /* 光标用定时器*/
			{
				if (i != 0)
				{
					timer_init(timer, &fifo, 0);
					if (cursor_c >= 0) /*当上方tab后cursor_c置-1后任务为console,所以判断是否大于零进行闪烁*/
					{
						cursor_c = COL8_000000;
					}
				}
				else
				{
					timer_init(timer, &fifo, 1);
					if (cursor_c >= 0)
					{
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if (cursor_c >= 0)
				{
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{

	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
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
	char c, tc, tbc; /*background color*/
	if (act != 0)	 /*参数act为1时,颜色不变*/
	{
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	}
	else /*否则变成灰色*/
	{
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize, 24, 4, tc, title);

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
				c = COL8_C6C6C6;
			}
			else
			{
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void console_task(struct SHEET *sheet)
{
	struct TIMER *timer;
	struct TASK *task = task_now();

	int i, fifobuf[128], cursor_x = 16, cursor_c = -1, cursor_y = 28; /*cursor错出一个字符长度*/
	char s[2];
	int x, y;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

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
			if (i <= 1) /* 光标用定时器*/
			{
				if (i != 0)
				{
					timer_init(timer, &task->fifo, 0); /* 下面设定0 */
					if (cursor_c >= 0)
					{
						cursor_c = COL8_FFFFFF;
					}
				}
				else
				{
					timer_init(timer, &task->fifo, 1); /* 下面设定1 */
					if (cursor_c >= 0)
					{
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			if (i == 2) /*光标ON */
			{
				cursor_c = COL8_FFFFFF;
			}
			if (i == 3) /*光标OFF */
			{
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
				cursor_c = -1;
			}
			if (256 <= i && i <= 511) /*A任务发送到fifo的键盘数据*/
			{
				if (i == 256 + 8) /* 退格键 */
				{
					if (cursor_x > 16) /* 用空格键把光标消去后，后移1次光标 */
					{
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				}
				else if (i == 10 + 256) /*回车键*/
				{
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					if (cursor_y < 28 + 112) /*用空格将光标擦除*/
					{
						cursor_y += 16;
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
					/*下一行的提示符*/
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				}
				else /*一般字符*/
				{
					if (cursor_x < 240)
					{
						s[0] = i - 256;
						s[1] = 0;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
				/*重新显示光标*/
				if (cursor_c >= 0) /*光标开启*/
				{
					boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
				}
				sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
			}
		}
	}
}
