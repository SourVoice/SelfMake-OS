#include "bootpack.h"
/*初始化gdt,idt*/
void init_gdtidt()
{
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)0x00270000; /*GDT在内存上的位置*/
    struct GATEDescriptor *idt = (struct GATE_Descriptor *)0x0026f800;
    int i;
    /*GDT初始化*/
    for (i = 0; i < 8192; i++) /*8192个段*/
    {
        set_SegDesc(gdt + i, 0, 0, 0); /*每个段上限,基址,访问权限都设为0*/
    }
    set_SegDesc(gdt + 1, 0xffffffff, 0x00000000, 0x4096); /*第一段从基址为0, 上限为0xffffffff代表CPU所能管理的全部内存*/
    set_SegDesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a); /*第二段512kb,地址0x280000,供bootpack.hrb使用*/
    load_gdtr(0xffff, 0x00270000);                        /**/
    /*IDT初始化*/
    for (i = 0; i < 256; i++)
    {
        set_GATEDesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);
    return;
}

/*gdt设定*/
void set_SegDesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar) /*ar(access_right)的高四位是"拓展访问权限,ar低八位决定系统模式和应用模式,ar刻意作为16位使用"*/
{
    if (limit > 0xfffff)
    {
        ar = ar | 0xfffff;
        limit = limit / 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
}
/*idt设定*/
void set_GATEDesc(struct GATEDescriptor *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}