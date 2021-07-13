#include "bootpack.h"
/*初始化gdt,idt*/
void init_gdtidt(void)
{
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)ADR_GDT; /*GDT在内存上的位置*/
    struct GATEDescriptor *idt = (struct GATE_Descriptor *)ADR_IDT;
    int i;
    /*GDT初始化*/
    for (i = 0; i <= LIMIT_GDT / 8; i++) /*8192个段*/
    {
        set_SegDesc(gdt + i, 0, 0, 0); /*每个段上限,基址,访问权限都设为0*/
    }
    set_SegDesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);   /*第一段从基址为0, 上限为0xffffffff代表CPU所能管理的全部内存*/
    set_SegDesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER); /*第二段512kb,地址0x280000,供bootpack.hrb使用*/
    load_gdtr(LIMIT_GDT, ADR_GDT);                                /**/
    /*IDT初始化*/
    for (i = 0; i <= LIMIT_IDT / 8; i++)
    {
        set_GATEDesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);
    /*设置中断*/
    set_GATEDesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32); /*asm_inthandler21注册进中断记录表(idt的第0x21号),中断时CPU调用asm_inthandler21,2*8表示asm_inthandler21属于哪一段*/
    set_GATEDesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
    set_GATEDesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32); /*asm_inthandler2c注册进中断记录表(idt的第0x2c号),中断时CPU调用asm_inthandler2c,2*8表示asm_inthandler2c属于哪一段*/

    return;
}
/*gdt设定*/
void set_SegDesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar)
{
    if (limit > 0xfffff)
    {
        ar = ar | 0x8000;
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
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}
