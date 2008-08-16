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

#include <kernel.h>
#include <time.h>
#include <fs/devfs.h>

#define RAND_MAX	2147483647

static ULONG urandom_state = 0, urandom_seed = 0;

void srand(UINT seed)
{
	if (seed==1 && urandom_seed) urandom_state=urandom_seed;
	else {
		urandom_state=seed;
		urandom_seed=seed;
	}
}

long double sqrt(long double a) //Heron
{
	double x=1,x_=0;
	while (x!=x_) {
		x_=x;
		x=(x_+a/x_)*0.5;
	}
	return x;
}

int rand(void)
{
	if (!urandom_state) srand(time(0));
	long double tmp=sqrt((long double)urandom_state);
	tmp*=10;
	tmp-=(UINT)tmp;
	urandom_state=(int)(RAND_MAX*tmp);
	return urandom_state;
}

static UINT drv_urandom_read(fs_node *node, off_t offset, size_t size, UCHAR *buffer)
{
	size_t i;
	i=size;
	while (i--)
		buffer[i]=(UCHAR)(rand()%0x100);
	return size;
}

extern UINT drv_null_write(fs_node *node, off_t offset, size_t size, UCHAR *buffer);
static node_operations urandom_ops = {0,&drv_urandom_read,&drv_null_write,0,0,0};

void setup_urandom_file(fs_node *devfs)
{
	srand(time(0));
	devfs_register_device(devfs,"urandom",0666,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&urandom_ops);
}
