#include "bootpack.h"

#define SHEET_USE 1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == 0)
    {
        goto err;
    }
    ctl->map = (unsigned char *)memman_alloc_4k(memman, xsize * ysize);
    if (ctl->map == 0) /*内存分配失败*/
    {
        memman_free_4k(memman, (int)ctl, sizeof(struct SHTCTL));
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

/*记录map图层*/
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, sid, *map = ctl->map;
    struct SHEET *sht;
    if (vx0 < 0)
    {
        vx0 = 0;
    }
    if (vy0 < 0)
    {
        vy0 = 0;
    }
    if (vx1 > ctl->xsize)
    {
        vx1 = ctl->xsize;
    }
    if (vy1 > ctl->ysize)
    {
        vy1 = ctl->ysize;
    }
    for (h = h0; h <= ctl->top; h++)
    {
        sht = ctl->sheets[h];
        sid = sht - ctl->sheets0; /* 将进行了减法计算的地址作为图层号码使用 */
        buf = sht->buf;
        bx0 = vx0 - sht->vram_x0;
        by0 = vy0 - sht->vram_y0;
        bx1 = vx1 - sht->vram_x0;
        by1 = vy1 - sht->vram_y0;
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
            for (bx = bx0; bx < bx1; bx++)
            {
                vx = sht->vram_x0 + bx;
                if (buf[by * sht->bxsize + bx] != sht->color_inv)
                {
                    map[vy * ctl->xsize + vx] = sid;
                }
            }
        }
    }
    return;
}

void sheet_refreshsub(struct SHTCTL *ctl, int vram_x0, int vram_y0, int vram_x1, int vram_y1, int h0, int h1)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;                   /*bx0,by0,bx1,by1表示将要调整到的位置*/
    unsigned char *buf, *vram = ctl->vram, *map = ctl->map, sid; /*sid for sheet-id*/
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
    for (h = h0; h <= h1; h++) /*仅对变化图层及以上的图层(按照map仅写map指定到h1)刷新*/
    {
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = sht - ctl->sheets0;
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
                if (map[vy * ctl->xsize + vx] == sid)
                {
                    vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
                }
            }
        }
    }
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
            ctl->sheets[height] = sht;                                                                                             /*图层全部移好后将sht放到设定的height处*/
            sheet_refreshmap(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height + 1); /*refersh前更新map*/
            sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height + 1, old);
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
            ctl->top--; /*显示图层减少,最上面图层高度下降*/
            sheet_refreshmap(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, 0);
            sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, 0, old - 1); /*图层被隐藏，从零开始绘制*/
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
        sheet_refreshmap(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height);
        sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height, height); /*显示重新排布好的图层*/
    }
    return;
}
/*从上到下描绘图层*/
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    {
        sheet_refreshsub(sht->ctl, sht->vram_x0 + bx0, sht->vram_y0 + by0, sht->vram_x0 + bx1, sht->vram_y0 + by1, sht->height, sht->height); /*refresh函数中图层上下关系不变,所以不进行map处理(更新map)*/
    }
    return;
}
/*上下左右移动图层*/
void sheet_slide(struct SHEET *sht, int vram_x0, int vram_y0)
{
    struct SHTCTL *ctl = sht->ctl;
    int old_vx0 = sht->vram_x0, old_vy0 = sht->vram_y0; /*记住移动之前的位置*/
    sht->vram_x0 = vram_x0;
    sht->vram_y0 = vram_y0;
    if (sht->height >= 0)
    {
        sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
        sheet_refreshmap(ctl, vram_x0, vram_y0, vram_x0 + sht->bxsize, vram_y0 + sht->bysize, sht->height);
        sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
        sheet_refreshsub(sht->ctl, vram_x0, vram_y0, vram_x0 + sht->bxsize, vram_y0 + sht->bysize, sht->height, sht->height);
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
