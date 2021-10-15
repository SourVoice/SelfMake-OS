#include "bootpack.h"

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000 /*缓存禁用*/

unsigned int memtest(unsigned int start, unsigned int end)
{
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    /* 确认CPU是386还是486以上的 */
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0)
    {
        /* 如果是386，即使设定AC=1，AC的值还会自动回到0 */
        flg486 = 1;
    }

    eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
    io_store_eflags(eflg);

    if (flg486 != 0)
    {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486 != 0)
    {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; /* 允许缓存 */
        store_cr0(cr0);
    }

    return i;
}

void memman_init(struct MEMMAN *man)
{
    man->frees = 0;    /* 可用信息数目 */
    man->maxfrees = 0; /* 用于观察可用状况：frees的最大值 */
    man->lostsize = 0; /* 释放失败的内存的大小总和 */
    man->losts = 0;    /* 释放失败次数 */
    return;
}

unsigned int memman_total(struct MEMMAN *man)
{
    unsigned int i, t = 0; /*t for memory abaiable*/
    for (i = 0; i < man->frees; i++)
    {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
    unsigned int i, a;
    for (i = 0; i < man->frees; i++)
    {
        if (man->free[i].size >= size) /*free[i].addr处有size可用*/
        {
            a = man->free[i].addr;
            man->free[i].addr += size;  /*可用空间地址向后推进size*/
            man->free[i].size -= size;  /*可用空间大小减少size*/
            if (man->free[i].size == 0) /*free[i]处空间已满*/
            {
                man->frees--; /*可用空余空间数减一*/
                for (; i < man->frees; i++)
                {
                    man->free[i] = man->free[i + 1];
                }
            }
            return a;
        }
    }
    return 0; /*没有可用空间*/
}
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i, j;
    for (i = 0; i < man->frees; i++)
    {
        if (man->free[i].addr > addr)
        {
            break;
        }
    }
    if (i > 0)
    {
        if (man->free[i - 1].addr + man->free[i - 1].size == addr)
        {
            man->free[i - 1].size += size; /*与前面可用内存合并*/
            if (i < man->frees)
            {
                /*后面的合并*/
                if (addr + size == man->free[i].addr)
                {
                    man->free[i - 1].size += man->free[i].size;
                    man->frees--; /*删去man->free[i]*/
                    for (; i < man->frees; i++)
                    {
                        /*同FIFO至零操作*/
                        man->free[i] = man->free[i + 1];
                    }
                }
            }
            return 0;
        }
    }
    /*不能与前面的可用空间进行归纳的内存*/
    if (i < man->frees)
    {
        if (addr + size == man->free[i].addr)
        {
            /*可用与后面的归纳到一起*/
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;
        }
    }
    /*既不能与前面归纳也不能与后面合并*/
    if (man->frees < MEMMAN_FREES)
    {
        /*free[i]之后的向后移动*/
        for (j = man->frees; j > i; j--)
        {
            man->free[j] = man->free[j - 1];
        }
        man->frees++;
        if (man->maxfrees < man->frees)
        {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    man->losts++; /*不能向后移动*/
    man->lostsize += size;
    return -1;
}
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000; /*分配时不足4kb补上(尾四位取0,加上0xfff)*/
    a = memman_alloc(man, size);
    return a;
}
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i = memman_free(man, addr, size);
    return i;
}
