#include "bootpack.h"
/*��ʼ��gdt,idt*/
void init_gdtidt()
{
    struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)0x00270000; /*GDT���ڴ��ϵ�λ��*/
    struct GATEDescriptor *idt = (struct GATE_Descriptor *)0x0026f800;
    int i;
    /*GDT��ʼ��*/
    for (i = 0; i < 8192; i++) /*8192����*/
    {
        set_SegDesc(gdt + i, 0, 0, 0); /*ÿ��������,��ַ,����Ȩ�޶���Ϊ0*/
    }
    set_SegDesc(gdt + 1, 0xffffffff, 0x00000000, 0x4096); /*��һ�δӻ�ַΪ0, ����Ϊ0xffffffff����CPU���ܹ����ȫ���ڴ�*/
    set_SegDesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a); /*�ڶ���512kb,��ַ0x280000,��bootpack.hrbʹ��*/
    load_gdtr(0xffff, 0x00270000);                        /**/
    /*IDT��ʼ��*/
    for (i = 0; i < 256; i++)
    {
        set_GATEDesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);
    return;
}

/*gdt�趨*/
void set_SegDesc(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar) /*ar(access_right)�ĸ���λ��"��չ����Ȩ��,ar�Ͱ�λ����ϵͳģʽ��Ӧ��ģʽ,ar������Ϊ16λʹ��"*/
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
/*idt�趨*/
void set_GATEDesc(struct GATEDescriptor *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}