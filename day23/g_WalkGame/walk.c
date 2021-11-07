
int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_initmalloc(void);
char *api_malloc(int size);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
int api_getkey(int mode);
void api_closewin(int win);
void api_end(void);

void HariMain(void)
{
    char *buf;
    int win, data, x, y;
    api_initmalloc();
    buf = api_malloc(160 * 100);
    win = api_openwin(buf, 160, 100, -1, "walk");
    x = 76;
    y = 56;
    api_putstrwin(win, x, y, 3, 1, ",");
    for (;;)
    {
        data = api_getkey(1);
        api_putstrwin(win, x, y, 0, 1, ",");
        if (data == '4' && x > 4)
            x -= 8;
        if (data == '6' && x < 148)
            x += 8;
        if (data == '8' && y < 24)
            y -= 8;
        if (data == '2' && y < 80)
            y += 8;
        if (data == 0x0a)
            break;
        api_putstrwin(win, x, y, 3, 1, ",");
    }
    api_closewin(win);
    api_end();
}
