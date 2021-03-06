#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == 0)
    {
        goto err;
    }
    ctl->vram = vram; /*binfo内容,避免你每次都从内存读取*/
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        ctl->sheets0[i].flags = 0; /*图层未使用*/
        ctl->sheets0[i].ctl = ctl; /*每个sheet都赋一个记录表*/
    }
err:
    return ctl;
}
/*返回一个新分配的图层*/
struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        if (ctl->sheets0[i].flags == 0) /*该图层未使用*/
        {
            sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE; /*正在使用*/
            sht->height = -1;       /*隐藏*/
            return sht;
        }
    }
    return 0;
}
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int color_inv)
{
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->color_inv = color_inv;
    return;
}
/*设定底板高度*/
void sheet_updown(struct SHEET *sht, int height)
{
    struct SHTCTL *ctl = sht->ctl;
    int h, old = sht->height; /*存储设置前的高度信息*/
    /*指定高度强制介于top~-1之间*/
    if (height > ctl->top + 1)
    {
        height = ctl->top + 1;
    }
    if (height < -1)
    {
        height = -1;
    }
    sht->height = height; /*设定图层高度*/

    if (old > height) /*图层height小于以前*/
    {
        if (height >= 0)
        {
            for (h = old; h > height; h--) /*介于old到height之间的图层向上移*/
            {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht; /*图层全部移好后将sht放到设定的height处*/
            sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, sht->height + 1);
        }
        else /*height = -1 (隐藏)*/
        {
            if (ctl->top > old)
            {
                for (h = old; h < ctl->top; h++)
                {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--;                                                                                                   /*显示图层减少,最上面图层高度下降*/
            sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, 0); /*图层被隐藏，从零开始绘制*/
        }
    }
    else if (old < height) /*设定位置比以前高*/
    {
        if (old >= 0)
        {
            for (h = old; h < height; h++)
            {
                /*图层下移*/
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else /*old = -1 (原来隐藏状态)*/
        {
            for (h = ctl->top; h >= height; h--)
            {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++;
        }
        sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height); /*显示重新排布好的图层*/
    }
    return;
}
/*从上到下描绘图层*/
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    {
        sheet_refreshsub(sht->ctl, sht->vram_y0 + bx0, sht->vram_y0 + by0, sht->vram_x0 + bx1, sht->vram_y0 + by1, sht->height);
    }
    return;
}
void sheet_refreshsub(struct SHTCTL *ctl, int vram_x0, int vram_y0, int vram_x1, int vram_y1, int h0)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1; /*bx0,by0,bx1,by1表示将要调整到的位置*/
    unsigned char *buf, c, *vram = ctl->vram;
    struct SHEET *sht;
    /*refresh超出范围进行画面修正*/
    if (vram_x0 < 0)
    {
        vram_x0 = 0;
    }
    if (vram_y0 < 0)
    {
        vram_y0 = 0;
    }
    if (vram_x1 > ctl->xsize)
    {
        vram_x1 = ctl->xsize;
    }
    if (vram_y1 > ctl->ysize)
    {
        vram_y1 = ctl->ysize;
    }
    for (h = h0; h <= ctl->top; h++) /*仅对变化图层及以上的图层刷新*/
    {
        sht = ctl->sheets[h];
        buf = sht->buf;
        /*从vx0~vy1，对bx0~by1进行倒推*/
        bx0 = vram_x0 - sht->vram_x0;
        by0 = vram_y0 - sht->vram_y0;
        bx1 = vram_x1 - sht->vram_x0;
        by1 = vram_y1 - sht->vram_y0;
        if (bx0 < 0)
        {
            bx0 = 0;
        }
        if (by0 < 0)
        {
            by0 = 0;
        }
        if (bx1 > sht->bxsize)
        {
            bx1 = sht->bxsize;
        }
        if (by1 > sht->bysize)
        {
            by1 = sht->bysize;
        }
        for (by = by0; by < by1; by++)
        {
            vy = sht->vram_y0 + by;
            for (bx = 0; bx < sht->bxsize; bx++)
            {
                vx = sht->vram_x0 + bx;
                if (vram_x0 <= vx && vx < vram_x1 && vram_y0 <= vy && vy < vram_y1)
                {
                    c = buf[by * sht->bxsize + bx];
                    if (c != sht->bxsize + vx)
                    {
                        vram[vy * ctl->xsize + vx] = c;
                    }
                }
            }
        }
    }
    return;
}
/*上下左右移动图层*/
void sheet_slide(struct SHEET *sht, int vram_x0, int vram_y0)
{
    int old_vx0 = sht->vram_x0, old_vy0 = sht->vram_y0; /*记住移动之前的位置*/
    sht->vram_x0 = vram_x0;
    sht->vram_y0 = vram_y0;
    if (sht->height >= 0)
    {
        sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
        sheet_refreshsub(sht->ctl, vram_x0, vram_y0, vram_x0 + sht->bxsize, vram_y0 + sht->bysize, sht->height);
    }
    return;
}

void sheet_free(struct SHEET *sht)
{
    if (sht->height >= 0)
    {
        sheet_updown(sht, -1);
    }
    sht->flags = 0;
    return;
}
