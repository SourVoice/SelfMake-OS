#include "bootpack.h"
/*��ʼ��gdt,idt*/
struct TSS32 tss_a, tss_b;
void init_gdtidt(void)
{
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT; /*GDT���ڴ��ϵ�λ��*/
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;

    int i;
    /*GDT��ʼ��*/
    for (i = 0; i <= LIMIT_GDT / 8; i++) /*8192����*/
    {
        set_segmdesc(gdt + i, 0, 0, 0); /*ÿ��������,��ַ,����Ȩ�޶���Ϊ0*/
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);   /*��һ�δӻ�ַΪ0, ����Ϊ0xffffffff����CPU���ܹ����ȫ���ڴ�*/
    set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER); /*�ڶ���512kb,��ַ0x280000,��bootpack.hrbʹ��*/
    set_segmdesc(gdt + 3, 103, (int)&tss_a, AR_TSS32);             /*tss+a������gdt����,�γ�103�ֽ�*/
    set_segmdesc(gdt + 4, 103, (int)&tss_b, AR_TSS32);
    load_gdtr(LIMIT_GDT, ADR_GDT); /**/
    /*IDT��ʼ��*/
    for (i = 0; i <= LIMIT_IDT / 8; i++)
    {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);
    /*�����ж�*/
    set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 * 8, AR_INTGATE32); /*asm_inthandler20ע����жϼ�¼��(idt�ĵ�0x20��),�ж�ʱCPU����asm_inthandler20,2*8��ʾasm_inthandler20������һ��,����������жϵ�ע��*/
    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32); /*asm_inthandler21ע����жϼ�¼��(idt�ĵ�0x21��),�ж�ʱCPU����asm_inthandler21,2*8��ʾasm_inthandler21������һ��*/
    set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32); /*asm_inthandler2cע����жϼ�¼��(idt�ĵ�0x2c��),�ж�ʱCPU����asm_inthandler2c,2*8��ʾasm_inthandler2c������һ��*/

    return;
}
/*gdt�趨*/
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
/*idt�趨*/
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}
