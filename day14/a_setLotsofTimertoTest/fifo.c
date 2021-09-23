#include "bootpack.h"

void fifo32_init(struct FIFO32 *fifo, int size, int *buf)
{
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0; //写入位置
    fifo->q = 0; //读出位置
    return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
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
int fifo32_get(struct FIFO32 *fifo)
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
int fifo32_status(struct FIFO32 *fifo)
{
    return fifo->size - fifo->free;
}
