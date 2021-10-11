#include "bootpack.h"
/*初始化gdt,idt*/
struct TSS32 tss_a, tss_b;
void init_gdtidt(void)
{
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT; /*GDT在内存上的位置*/
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;

    int i;
    /*GDT初始化*/
    for (i = 0; i <= LIMIT_GDT / 8; i++) /*8192个段*/
    {
        set_segmdesc(gdt + i, 0, 0, 0); /*每个段上限,基址,访问权限都设为0*/
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);   /*第一段从基址为0, 上限为0xffffffff代表CPU所能管理的全部内存*/
    set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER); /*第二段512kb,地址0x280000,供bootpack.hrb使用*/
    set_segmdesc(gdt + 3, 103, (int)&tss_a, AR_TSS32);             /*tss+a定义在gdt三号,段长103字节*/
    set_segmdesc(gdt + 4, 103, (int)&tss_b, AR_TSS32);
    load_gdtr(LIMIT_GDT, ADR_GDT); /**/
    /*IDT初始化*/
    for (i = 0; i <= LIMIT_IDT / 8; i++)
    {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);
    /*设置中断*/
    set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 * 8, AR_INTGATE32); /*asm_inthandler20注册进中断记录表(idt的第0x20号),中断时CPU调用asm_inthandler20,2*8表示asm_inthandler20属于哪一段,间隔计数器中断的注册*/
    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32); /*asm_inthandler21注册进中断记录表(idt的第0x21号),中断时CPU调用asm_inthandler21,2*8表示asm_inthandler21属于哪一段*/
    set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32); /*asm_inthandler2c注册进中断记录表(idt的第0x2c号),中断时CPU调用asm_inthandler2c,2*8表示asm_inthandler2c属于哪一段*/

    return;
}
/*gdt设定*/
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
    if (limit > 0xfffff)
    {
        ar |= 0x8000; /* G_bit = 1 */
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
    return;
}
/*idt设定*/
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}
