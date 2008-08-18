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

#include <fs/fs.h>
#include <lib/string.h>
#include <task.h>

fs_node *namei(const char *filename)
{
	//TODO: Permissioncheck
	//Maybe a IS_DIR-Makro?
	char sname[NODE_NAME_LEN],*token,*ptr;
	fs_node *node;

	if (!filename) return 0;
	if (!current_task || !(current_task->pwd)) node=get_root_fs_node();
		else node=current_task->pwd;
	if (filename[0]=='/') {
		node=get_root_fs_node();
		filename++;
	}
	if (!filename) return 0;
	strcpy(sname,filename);
	token=strtok_save(sname,"/",&ptr);
	while (token) {
		if (!strcmp(token,"..")) {
			if (node==get_root_fs_node()) {
				token=strtok_save(0,"/",&ptr);
				continue;
			}
			if (node==node->mi->root && node->mi->parent_dir) {
				node=node->mi->parent_dir;
				token=strtok_save(0,"/",&ptr);
				continue;
			}
		}
		node=resolve_node(finddir_fs(node,token));
		if (!node) return 0;
		token=strtok_save(0,"/",&ptr);
	}
	return node;
}
