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
    ctl->vram = vram; /*binfo����,������ÿ�ζ����ڴ��ȡ*/
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        ctl->sheets0[i].flags = 0; /*ͼ��δʹ��*/
    }
err:
    return ctl;
}
/*����һ���·����ͼ��*/
struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        if (ctl->sheets0[i].flags == 0) /*��ͼ��δʹ��*/
        {
            sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE; /*����ʹ��*/
            sht->height = -1;       /*����*/
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
/*�趨�װ�߶�*/
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
    int h, old = sht->height; /*�洢����ǰ�ĸ߶���Ϣ*/
    /*ָ���߶�ǿ�ƽ���top~-1֮��*/
    if (height > ctl->top + 1)
    {
        height = ctl->top + 1;
    }
    if (height < -1)
    {
        height = -1;
    }
    sht->height = height; /*�趨ͼ��߶�*/

    if (old > height) /*ͼ��heightС����ǰ*/
    {
        if (height >= 0)
        {
            for (h = old; h > height; h--) /*����old��height֮���ͼ��������*/
            {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht; /*ͼ��ȫ���ƺú�sht�ŵ��趨��height��*/
        }
        else /*height = -1 (����)*/
        {
            if (ctl->top > old)
            {
                for (h = old; h < ctl->top; h++)
                {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--; /*��ʾͼ�����,������ͼ��߶��½�*/
        }
        sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize); /*��ʾ�����Ų��õ�ͼ��*/
    }
    else if (old < height) /*�趨λ�ñ���ǰ��*/
    {
        if (old >= 0)
        {
            for (h = old; h < height; h++)
            {
                /*ͼ������*/
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else /*old = -1 (ԭ������״̬)*/
        {
            for (h = ctl->top; h >= height; h--)
            {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++;
        }
        sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize); /*��ʾ�����Ų��õ�ͼ��*/
    }
    return;
}
/*���ϵ������ͼ��*/
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    {
        sheet_refreshsub(ctl, sht->vram_y0 + bx0, sht->vram_y0 + by0, sht->vram_x0 + bx1, sht->vram_y0 + by1);
    }
    return;
}
/*���������ƶ�ͼ��*/
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vram_x0, int vram_y0)
{
    int old_vx0 = sht->vram_x0, old_vy0 = sht->vram_y0; /*��ס�ƶ�֮ǰ��λ��*/
    sht->vram_x0 = vram_x0;
    sht->vram_y0 = vram_y0;
    if (sht->height >= 0)
    {
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
        sheet_refreshsub(ctl, vram_x0, vram_y0, vram_x0 + sht->bxsize, vram_y0 + sht->bysize);
    }
    return;
}
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
    if (sht->height >= 0)
    {
        sheet_updown(ctl, sht, -1);
    }
    sht->flags = 0;
    return;
}
void sheet_refreshsub(struct SHTCTL *ctl, int vram_x0, int vram_y0, int vram_x1, int vram_y1)
{
    int h, bx, by, vx, vy;
    unsigned char *buf, c, *vram = ctl->vram;
    struct SHEET *sht;
    for (h = 0; h <= ctl->top; h++)
    {
        sht = ctl->sheets[h];
        buf = sht->buf;
        for (by = 0; by < sht->bysize; by++)
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
