
#include "bootpack.h"
unsigned int memtest(unsigned int start, unsigned int end)
{
    char flag486 = 0;
    unsigned int eflg, cr0, i;
    /*���CPUΪ386����486(���޸��ٻ��������)*/
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /*AC-bit = 1*/
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) /*486ϵ�д�����EFLAGs�Ĵ�����18λΪAC��־*/
    {
        flag486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT; /*AC-bit = 0*/
    io_store_eflags(eflg);
    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0); /*�������*/
    }
    i = memtest_sub(start, end);
    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0); /*���û���*/
    }
    return i;
}
void memman_init(struct MEMMAN *man)
{
    man->frees = 0;
    man->maxfrees = 0;
    man->losts = 0;
    man->lostsize = 0;
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
} //audio_enjoyment
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
    unsigned int i, a;
    for (i = 0; i < man->frees; i++)
    {
        if (man->free[i].size >= size) /*free[i].addr����size����*/
        {
            a = man->free[i].addr;
            man->free[i].size -= size;  /*���ÿռ��С����size*/
            man->free[i].addr += size;  /*���ÿռ��ַ����ƽ�size*/
            if (man->free[i].size == 0) /*free[i]���ռ�����*/
            {
                man->frees--; /*���ÿ���ռ�����һ*/
                for (; i < man->frees; i++)
                {
                    man->free[i] = man->free[i + 1];
                }
            }
            return a;
        }
    }
    return 0; /*û�п��ÿռ�*/
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
            man->free[i - 1].size += size; /*��ǰ������ڴ�ϲ�*/
            if (i < man->frees)
            {
                /*����ĺϲ�*/
                if (addr + size == man->free[i].addr)
                {
                    man->free[i - 1].size += man->free[i].size;
                    man->frees--; /*ɾȥman->free[i]*/
                    for (; i < man->frees; i++)
                    {
                        /*ͬFIFO�������*/
                        man->free[i] = man->free[i + 1];
                    }
                }
            }
            return 0;
        }
    }
    /*������ǰ��Ŀ��ÿռ���й��ɵ��ڴ�*/
    if (i < man->frees)
    {
        if (addr + size == man->free[i].addr)
        {
            /*���������Ĺ��ɵ�һ��*/
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;
        }
    }
    /*�Ȳ�����ǰ�����Ҳ���������ϲ�*/
    if (man->frees < MEMMAN_FREES)
    {
        /*free[i]֮�������ƶ�*/
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
    man->losts++; /*��������ƶ�*/
    man->lostsize += size;
    return -1;
}
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000; /*����ʱ����4kb����(β��λȡ0,����0xfff)*/
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
