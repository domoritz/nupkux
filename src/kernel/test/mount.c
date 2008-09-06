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

#include "mount.h"
#include <mm.h>
#include <errno.h>

extern filesystem_t *vfs_get_fs(const char *name);

static vfsmount *vfs_mounts = 0;

int d_mount(vfsmount *mnt)
{
	if (!mnt) return -EINVAL;
	if (mnt->next) return -EBUSY;
	mnt->next=vfs_mounts;
	vfs_mounts=mnt;
	return 0;
}

int d_umount(vfsmount *mnt)
{
	return 0;
}

int sys_mount(	const char *source, const char *target,
		const char *filesystemtype, unsigned long mountflags,
		void *data)
{
	filesystem_t *type=vfs_get_fs(filesystemtype);
	if (!type) return -EINVAL;
	vfsmount *mnt=malloc(sizeof(vfsmount));
	super_block *sb=malloc(sizeof(super_block));
	mnt->devname=source;
	mnt->dirname=target;
	mnt->flags=mountflags;
	mnt->next=0;
	sb->flags=mountflags;
	sb->mi=mnt;
	sb->type=type;
	mnt->sb=type->read_super(sb,data,0);
	d_mount(mnt);
	return 0;
}
