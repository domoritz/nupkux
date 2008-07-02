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

#include <multiboot.h>
#include <kernel.h>
#include <kernel/ktextio.h>
#include <kernel/nish.h>
#include <time.h>
#include <drivers/fdc.h>
#include <mm.h>
#include <fs/initrd.h>

char _kabort_func = 0;
int errno;
UINT initrd_location = 0;
ULONG memory_end = 0;
UINT __working_memstart = 0;
extern UINT kmalloc_pos;
extern UINT initrd_location;

void reboot()
{
	volatile UCHAR in = 0x02;

	printf("Will now reboot");
	while (in & 0x02)
		in=inportb(0x64);
	outportb(0x64,0xFE);
}

static void halt()
{	
	printf("Will now halt");
	printf("\n\nYou can turn off the computer.");
	cli();
	hlt();
}

int _kmain(multiboot_info_t* mbd, unsigned int magic) 
{
	int ret;
	fs_node *root;
	
	_kclear();
	if (mbd->flags&0x01) memory_end=mbd->mem_upper*1024;
		else memory_end=0x400000;
	printf("Nupkux loaded ...\nAmount of RAM: %d Bytes.\nSet up Descriptors ... ",memory_end);
	if (mbd->mods_count>0) {
		initrd_location = *((UINT*)mbd->mods_addr);
		__working_memstart=*(UINT*)(mbd->mods_addr+4);
	} else __working_memstart=(UINT) &kernel_end;
	kmalloc_pos=__working_memstart;
	gdt_install();
	idt_install();
	printf("Finished.\nInstall IRQ & ISRS ... ");
	isrs_install();
	irq_install();
	printf("Finished.\nStart Keyboard Controller ... ");
	input_setup();
	printf("Finished.\nEnable Interrupts ... ");
	sti();
	printf("Finished.\nEnable Paging and Memory Manager ... ");
	paging_setup();
	init_ktexto();
	printf("Finished.\nSetup VFS ... ");
	setup_vfs();
	printf("Finished.\nMount initrd read-only on root ... ");
	root=setup_initrd(initrd_location,get_root_fs_node());
	if (root) printf("Finished.\n");
		else printf("FAILED.\n");
	printf("Set up timer ... ");
	timer_install();
	printf("Finished.\nFloppydrive support  ... ");
	init_floppy();
	if (!floppy_drives) printf("No drives found.\n");
		else printf("%s found.\n",(floppy_drives & 0x0F)?"2 drives":"1 drive");
	printf("Booted up!\n");
	printf("nish returned with 0x%X.\n",ret=nish());
	printf("\nUnmount initrd (root) ... ");
	remove_initrd(root);
	printf("\nClose VFS ... ");
	close_vfs();
	printf("OK\n");
	switch (ret) {
	  case NISH_REBOOT: reboot();
			    return 0;	
			    break;
	  case NISH_HALT:   halt();
			    return 0;
			    break;
	  default: 	    printf("Stop system");
			    return 0;
			    break;
	}
	while (1);
	return 0;
}

