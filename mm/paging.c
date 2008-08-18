/*
 *  Copyright (C) 2007,2008 Sven Köhler
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

#include <paging.h>
#include <lib/memory.h>
#include <kernel/ktextio.h>
#include <kernel/dts.h>
#include <task.h>

page_directory *current_directory, *kernel_directory;

static UINT framecount = 0;
static UINT *framemap;

extern UINT kmalloc_pos;
extern UINT _kmalloc_pa(UINT sz, UINT *phys);
extern heap *kheap;
extern void clone_page(UINT src, UINT dest);

#define MAP_MEMORY(start,end,flags) for (i=start;i<=end;i+=FRAME_SIZE) \
		make_page(i,flags,kernel_directory,1)

static void set_page_directory(page_directory *PAGE_DIR)
{
	current_directory=PAGE_DIR;
	asm volatile (	"cli\n\t"
			"movl %%eax,%%cr3\n\t"
			"movl %%cr0,%%eax\n\t"
			"orl  $0x80000000,%%eax\n\t"
			"movl %%eax,%%cr0\n\t"
			"sti"::"a"(PAGE_DIR->physPos));
}

static UINT first_frame(void)
{
	UINT i,j;

	for (i=0;i<framecount/32;i++)
		if (framemap[i]!=0xFFFFFFFF)
			for (j=0;j<32;j++)
				if (!(framemap[i]&(1<<j)))
					return i*32+j;
	return 0xFFFFFFFF;
}

static void alloc_frame(page *apage, UINT flags)
{
	UINT number = first_frame();

	if (apage->frame) return;
	apage->frame=number;
	apage->flags=flags;
	framemap[number/32]|=(1<<(number%32));
}

static void free_frame(page *apage)
{
	UINT number=apage->frame;

	apage->frame=0;
	if (number) framemap[number/32]&=~(1<<(number%32));
}

static page_table *make_table(UINT index, UINT flags, page_directory *directory)
{
	page_table *res=(page_table *)_kmalloc_pa(sizeof(page_table),&(directory->physTabs[index]));

	directory->physTabs[index]|=flags;
	memset(res,0,sizeof(page_table));
	directory->tables[index]=res;
	return res;
}

page *make_page(UINT address, UINT flags, page_directory *directory, int alloc)
{
	UINT index = address/FRAME_SIZE;
	UINT tab = index/1024;

	if (!directory->physTabs[tab]) make_table(tab,flags,directory);
	if (alloc) alloc_frame(&(directory->tables[tab]->entries[index%1024]),flags);
	return &(directory->tables[tab]->entries[index%1024]);
}

page *free_page(UINT address, page_directory *directory)
{
	UINT index = address/FRAME_SIZE;
	UINT tab = index/1024;

	if (!directory->physTabs[tab]) return 0;
	free_frame(&(directory->tables[tab]->entries[index%1024]));
	return &(directory->tables[tab]->entries[index%1024]);
}

static page_table* clone_table(page_table* src, UINT* physAddr)
{
	UINT i = 1024;
	page_table *table = (page_table*)_kmalloc_pa(sizeof(page_table),physAddr);

	memset(table,0,sizeof(page_table));
	while (i--) {
		if (!src->entries[i].frame) continue;
		alloc_frame(&table->entries[i],src->entries[i].flags);
		clone_page(src->entries[i].frame*FRAME_SIZE,table->entries[i].frame*FRAME_SIZE);
	}
	return table;
}

page_directory* clone_directory(page_directory* src)
{
	UINT phys,i = 1024;
	page_directory *dir = (page_directory *)_kmalloc_pa(sizeof(page_directory),&phys);

	memset(dir,0,sizeof(page_directory));
	dir->physPos=phys;//+(UINT)dir->physTabs-(UINT)dir;
	while (i--) {
		if (!src->tables[i]) continue;
		if (kernel_directory->tables[i]==src->tables[i]) {
			dir->tables[i]=src->tables[i];
			dir->physTabs[i]=src->physTabs[i];
		} else {
			dir->tables[i]=clone_table(src->tables[i],&phys);
			dir->physTabs[i]=phys | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USERMODE;
		}
	}
	return dir;
}

static void free_table(page_table *table)
{
	UINT i=1024,number;
	while (i--) {
		number=table->entries[i].frame;
		//We cannot free page, because we are in this page_directory (Remind cli()!)
		//So we will only "set free" the frame
		if (number) framemap[number/32]&=~(1<<(number%32));
	}
	free(table);
}

void free_directory(page_directory *dir)
{
	UINT i=1024;
	while (i--) {
		if (!dir->tables[i]) continue;
		if (kernel_directory->tables[i]!=dir->tables[i])
			free_table(dir->tables[i]);
	}
	free(dir);
}

page *get_page(UINT address, int make, page_directory *directory)
{
	UINT index = address/FRAME_SIZE;
	UINT tab = index/1024;

	if (directory->physTabs[tab])
		return &(directory->tables[tab]->entries[index%1024]);
	else if (make)
		return make_page(address,PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USERMODE,directory,0);
	return 0;
}

void page_fault_handler(registers *regs)
{
	UINT faultaddr;

	asm volatile ("mov %%cr2,%%eax":"=a"(faultaddr));

	printf("\nPagefault at 0x%X: %s%s%s%s\n",faultaddr,(!(regs->err_code&1))?"present ":"",(regs->err_code&2)?"read-only ":"",(regs->err_code&4)?"user-mode ":"",(regs->err_code&8)?"reserved ":"");
	abort_current_process();
}

void setup_paging()
{
	UINT i = 0;

	framecount=WORKING_MEMEND/FRAME_SIZE;
	framemap=(UINT *)_kmalloc(framecount/8);  // sizeof(UINT)*fc/32
	memset(framemap,0,framecount/8);
	kernel_directory=(page_directory *)_kmalloc_pa(sizeof(page_directory),&i);
	memset(kernel_directory,0,sizeof(page_directory));
	kernel_directory->physPos=(UINT)kernel_directory->physTabs;
	MAP_MEMORY(0,WORKING_MEMSTART,PAGE_FLAG_READONLY | PAGE_FLAG_USERMODE | PAGE_FLAG_PRESENT); //Kernel & initrd
	MAP_MEMORY(WORKING_MEMSTART,WORKING_MEMSTART+IPC_MEMSIZE,PAGE_FLAG_WRITE | PAGE_FLAG_USERMODE | PAGE_FLAG_PRESENT); //IPC
	MAP_MEMORY(WORKING_MEMSTART+IPC_MEMSIZE,kmalloc_pos+FRAME_SIZE,PAGE_FLAG_READONLY | PAGE_FLAG_USERMODE | PAGE_FLAG_PRESENT); //Pre-Heap
	i=MM_KHEAP_START+kmalloc_pos;
	ASSERT_ALIGN(i);
	MAP_MEMORY(i,ALIGN_UP(kmalloc_pos)+MM_KHEAP_START+MM_KHEAP_SIZE,PAGE_FLAG_USERMODE | PAGE_FLAG_READONLY | PAGE_FLAG_PRESENT); //Heap
	register_interrupt_handler(14,page_fault_handler);
	set_page_directory(kernel_directory);
	kheap=create_heap(MM_KHEAP_START+kmalloc_pos,MM_KHEAP_START+MM_KHEAP_SIZE+kmalloc_pos,WORKING_MEMEND,PAGE_FLAG_USERMODE | PAGE_FLAG_READONLY | PAGE_FLAG_PRESENT);
	current_directory=clone_directory(kernel_directory);
	set_page_directory(current_directory);
}
