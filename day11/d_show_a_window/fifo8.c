#include "bootpack.h"

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
{
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0; //写入位置
    fifo->q = 0; //读出位置
    return;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data)
{
    if (fifo->free == 0)
    {
        fifo->flags |= FLAFS_OVERRUN;
        return -1; /*溢出*/
    }
    fifo->buf[fifo->p] = data;
    fifo->p++;
    if (fifo->p == fifo->size)
    {
        fifo->p = 0;
    }
    fifo->free--;
    return 0;
}
int fifo8_get(struct FIFO8 *fifo)
{
    int data;
    if (fifo->free == fifo->size)
    {
        /*缓冲区空*/
        return -1;
    }
    data = fifo->buf[fifo->q];
    fifo->q++;
    if (fifo->q == fifo->size)
    {
        fifo->q = 0;
    }
    fifo->free++;
    return data;
}
int fifo8_status(struct FIFO8 *fifo)
{
    return fifo->size - fifo->free;
}
