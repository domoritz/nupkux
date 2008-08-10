/*
 *  Copyright (C) 2008 Sven Köhler
 *
 *  This file is part of Nupkux.
 *
 *  Nupkux is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nupkux is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Nupkux.  If not, see <http://www.gnu.org/licenses/>.
 */

//                       Based on code from Bran's and JamesM's kernel development tutorials.

#include <kernel/dts.h>
#include <lib/memory.h>
#include <kernel/ktextio.h>

#define IDT_SET_GATE_ISR(nr)	idt_set_gate(nr,(UINT)isr##nr,0x08,0x8E);
#define IDT_SET_GATE_IRQ(nr)	idt_set_gate(IRQ##nr,(UINT)irq##nr,0x08,0x8E);

extern void gdt_flush(UINT);
extern void idt_flush(UINT);
extern void tss_flush();
static void init_gdt();
static void init_idt();
static void gdt_set_gate(int,UINT,UINT,UCHAR,UCHAR);
static void idt_set_gate(UCHAR,UINT,USHORT,UCHAR);
static void write_tss(int,UINT,UINT);

tss_entry tss_ent;

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void isr128();

gdt_entry gdt_entries[5];
gdt_pointer gdt_ptr;
idt_entry idt_entries[256];
idt_pointer idt_ptr;
isr_t interrupt_handlers[256];

void setup_dts()
{

	init_gdt();
	init_idt();
	memset(&interrupt_handlers,0,sizeof(isr_t)*256);
}

static void init_gdt()
{
	gdt_ptr.limit=(sizeof(gdt_entry)*6)-1;
	gdt_ptr.base=(UINT)&gdt_entries;
	gdt_set_gate(0,0,0,0,0);
	gdt_set_gate(1,0,0xFFFFFFFF,0x9A,0xCF);
	gdt_set_gate(2,0,0xFFFFFFFF,0x92,0xCF);
	gdt_set_gate(3,0,0xFFFFFFFF,0xFA,0xCF);
	gdt_set_gate(4,0,0xFFFFFFFF,0xF2,0xCF);
	write_tss(5,0x10,0x0);
	gdt_flush((UINT)&gdt_ptr);
	tss_flush();
}


static void gdt_set_gate(int num, UINT base, UINT limit, UCHAR access, UCHAR gran)
{
	gdt_entries[num].base_low    =(base & 0xFFFF);
	gdt_entries[num].base_middle =(base >> 16) & 0xFF;
	gdt_entries[num].base_high   =(base >> 24) & 0xFF;
	gdt_entries[num].limit_low   =(limit & 0xFFFF);
	gdt_entries[num].granularity =(limit >> 16) & 0x0F;
	gdt_entries[num].granularity|=gran & 0xF0;
	gdt_entries[num].access      =access;
}

static void init_idt()
{
	idt_ptr.limit=sizeof(idt_entry)*256-1;
	idt_ptr.base=(UINT)&idt_entries;
	memset(&idt_entries,0,sizeof(idt_entry)*256);
	outportb(0x20,0x11);
	outportb(0xA0,0x11);
	outportb(0x21,0x20);
	outportb(0xA1,0x28);
	outportb(0x21,0x04);
	outportb(0xA1,0x02);
	outportb(0x21,0x01);
	outportb(0xA1,0x01);
	outportb(0x21,0x0);
	outportb(0xA1,0x0);
	IDT_SET_GATE_ISR(0);
	IDT_SET_GATE_ISR(1);
	IDT_SET_GATE_ISR(2);
	IDT_SET_GATE_ISR(3);
	IDT_SET_GATE_ISR(4);
	IDT_SET_GATE_ISR(5);
	IDT_SET_GATE_ISR(6);
	IDT_SET_GATE_ISR(7);
	IDT_SET_GATE_ISR(8);
	IDT_SET_GATE_ISR(9);
	IDT_SET_GATE_ISR(10);
	IDT_SET_GATE_ISR(11);
	IDT_SET_GATE_ISR(12);
	IDT_SET_GATE_ISR(13);
	IDT_SET_GATE_ISR(14);
	IDT_SET_GATE_ISR(15);
	IDT_SET_GATE_ISR(16);
	IDT_SET_GATE_ISR(17);
	IDT_SET_GATE_ISR(18);
	IDT_SET_GATE_ISR(19);
	IDT_SET_GATE_ISR(20);
	IDT_SET_GATE_ISR(21);
	IDT_SET_GATE_ISR(22);
	IDT_SET_GATE_ISR(23);
	IDT_SET_GATE_ISR(24);
	IDT_SET_GATE_ISR(25);
	IDT_SET_GATE_ISR(26);
	IDT_SET_GATE_ISR(27);
	IDT_SET_GATE_ISR(28);
	IDT_SET_GATE_ISR(29);
	IDT_SET_GATE_ISR(30);
	IDT_SET_GATE_ISR(31);
	IDT_SET_GATE_IRQ(0);
	IDT_SET_GATE_IRQ(1);
	IDT_SET_GATE_IRQ(2);
	IDT_SET_GATE_IRQ(3);
	IDT_SET_GATE_IRQ(4);
	IDT_SET_GATE_IRQ(5);
	IDT_SET_GATE_IRQ(6);
	IDT_SET_GATE_IRQ(7);
	IDT_SET_GATE_IRQ(8);
	IDT_SET_GATE_IRQ(9);
	IDT_SET_GATE_IRQ(10);
	IDT_SET_GATE_IRQ(11);
	IDT_SET_GATE_IRQ(12);
	IDT_SET_GATE_IRQ(13);
	IDT_SET_GATE_IRQ(14);
	IDT_SET_GATE_IRQ(15);
	IDT_SET_GATE_ISR(128);
	idt_flush((UINT)&idt_ptr);
}

static void idt_set_gate(UCHAR num, UINT base, USHORT sel, UCHAR flags)
{
	idt_entries[num].base_lo=base&0xFFFF;
	idt_entries[num].base_hi=(base>>16)&0xFFFF;
	idt_entries[num].sel=sel;
	idt_entries[num].always0=0;
	idt_entries[num].flags=flags; //|0x60
}

static void write_tss(int num, UINT ss0, UINT esp0)
{
	UINT base=(UINT)&tss_ent;
	UINT limit=base+sizeof(tss_entry);
	gdt_set_gate(num,base,limit,0xE9,0x00);
	memset(&tss_ent,0,sizeof(tss_entry));
	tss_ent.ss0=ss0;
	tss_ent.esp0=esp0;
	tss_ent.cs=0x0b;
	tss_ent.ss=tss_ent.ds=tss_ent.es=tss_ent.fs=tss_ent.gs=0x13;
}

void set_kernel_stack(UINT stack)
{
	tss_ent.esp0=stack;
}

//Interrupt Service Routines and related stuff

char *exception_messages[] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",

	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",

	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",

	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void register_interrupt_handler(UCHAR n, isr_t handler)
{
	interrupt_handlers[n]=handler;
}

void isr_handler(registers regs)
{
	UCHAR int_no = regs.int_no&0xFF;
	if (interrupt_handlers[int_no]) {
		isr_t handler=interrupt_handlers[int_no];
		handler(&regs);
	} else {
		if (int_no<32) {
			printf("%s Exception. System Halted!\n",exception_messages[int_no]);
			cli();
			hlt();
		} else printf("unhandled interrupt: 0x%X\n",int_no);
	}
}

void irq_handler(registers regs)
{
    if (regs.int_no>=40) {
        outportb(0xA0,0x20);
    }
    outportb(0x20,0x20);
    if (interrupt_handlers[regs.int_no]) {
        isr_t handler=interrupt_handlers[regs.int_no];
        handler(&regs);
    }
}
