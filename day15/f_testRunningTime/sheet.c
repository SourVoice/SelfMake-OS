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
    if (ctl->map == 0) /*�ڴ����ʧ��*/
    {
        memman_free_4k(memman, (int)ctl, sizeof(struct SHTCTL));
        goto err;
    }
    ctl->vram = vram; /*binfo����,������ÿ�ζ����ڴ��ȡ*/
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        ctl->sheets0[i].flags = 0; /*ͼ��δʹ��*/
        ctl->sheets0[i].ctl = ctl; /*ÿ��sheet����һ����¼��*/
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

/*��¼mapͼ��*/
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
        sid = sht - ctl->sheets0; /* �������˼�������ĵ�ַ��Ϊͼ�����ʹ�� */
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
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;                   /*bx0,by0,bx1,by1��ʾ��Ҫ��������λ��*/
    unsigned char *buf, *vram = ctl->vram, *map = ctl->map, sid; /*sid for sheet-id*/
    struct SHEET *sht;
    /*refresh������Χ���л�������*/
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
    for (h = h0; h <= h1; h++) /*���Ա仯ͼ�㼰���ϵ�ͼ��(����map��дmapָ����h1)ˢ��*/
    {
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = sht - ctl->sheets0;
        /*��vx0~vy1����bx0~by1���е���*/
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
/*�趨�װ�߶�*/
void sheet_updown(struct SHEET *sht, int height)
{
    struct SHTCTL *ctl = sht->ctl;
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
            ctl->sheets[height] = sht;                                                                                             /*ͼ��ȫ���ƺú�sht�ŵ��趨��height��*/
            sheet_refreshmap(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height + 1); /*refershǰ����map*/
            sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height + 1, old);
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
            sheet_refreshmap(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, 0);
            sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, 0, old - 1); /*ͼ�㱻���أ����㿪ʼ����*/
        }
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
        sheet_refreshmap(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height);
        sheet_refreshsub(ctl, sht->vram_x0, sht->vram_y0, sht->vram_x0 + sht->bxsize, sht->vram_y0 + sht->bysize, height, height); /*��ʾ�����Ų��õ�ͼ��*/
    }
    return;
}
/*���ϵ������ͼ��*/
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    {
        sheet_refreshsub(sht->ctl, sht->vram_x0 + bx0, sht->vram_y0 + by0, sht->vram_x0 + bx1, sht->vram_y0 + by1, sht->height, sht->height); /*refresh������ͼ�����¹�ϵ����,���Բ�����map����(����map)*/
    }
    return;
}
/*���������ƶ�ͼ��*/
void sheet_slide(struct SHEET *sht, int vram_x0, int vram_y0)
{
    struct SHTCTL *ctl = sht->ctl;
    int old_vx0 = sht->vram_x0, old_vy0 = sht->vram_y0; /*��ס�ƶ�֮ǰ��λ��*/
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
