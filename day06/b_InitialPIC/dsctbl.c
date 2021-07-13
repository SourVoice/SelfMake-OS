#include "bootpack.h"
/*��ʼ��gdt,idt*/
void init_gdtidt(void)
{
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)ADR_GDT; /*GDT���ڴ��ϵ�λ��*/
    struct GATEDescriptor *idt = (struct GATE_Descriptor *)ADR_IDT;
    int i;
    /*GDT��ʼ��*/
    for (i = 0; i <= LIMIT_GDT / 8; i++) /*8192����*/
    {
        set_SegDesc(gdt + i, 0, 0, 0); /*ÿ��������,��ַ,����Ȩ�޶���Ϊ0*/
    }
    set_SegDesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);   /*��һ�δӻ�ַΪ0, ����Ϊ0xffffffff����CPU���ܹ����ȫ���ڴ�*/
    set_SegDesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER); /*�ڶ���512kb,��ַ0x280000,��bootpack.hrbʹ��*/
    load_gdtr(LIMIT_GDT, ADR_GDT);                                /**/
    /*IDT��ʼ��*/
    for (i = 0; i <= LIMIT_IDT / 8; i++)
    {
        set_GATEDesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);
    /*�����ж�*/
    set_GATEDesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32); /*asm_inthandler21ע����жϼ�¼��(idt�ĵ�0x21��),�ж�ʱCPU����asm_inthandler21,2*8��ʾasm_inthandler21������һ��*/
    set_GATEDesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
    set_GATEDesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32); /*asm_inthandler2cע����жϼ�¼��(idt�ĵ�0x2c��),�ж�ʱCPU����asm_inthandler2c,2*8��ʾasm_inthandler2c������һ��*/

    return;
}
/*gdt�趨*/
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
/*idt�趨*/
void set_GATEDesc(struct GATEDescriptor *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}
