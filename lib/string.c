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

#include <lib/string.h>

static char *strtok_save_ptr;

int strcmp(const char *s1, const char *s2)
{	
	while ((*s1) && (*s2)) {
		if (*s1<*s2) return -1;
		if (*s1>*s2) return 1;
		s1++;
		s2++;
	}
	if ((!*s1) && (*s2)) return -1;
	if ((*s1) && (!*s2)) return 1;
	return 0;
}

size_t strlen(const char *str)
{
	size_t res = 0;
	
	while (*(str++)) res++;
	return res;
}

char *strchr(const char *str, char chr)
{
	while ((*str) && (*str!=chr)) str++;
	if (!(*str)) return 0;
	return (char *)((UINT)str);
}

char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;
	while (*src) *(tmp++)=*(src++);
	*tmp=0;
	return dest;
}

char *strncpy(char *dest, const char *src, size_t num)
{
	char *tmp = dest;
	while (*src && num--) *(tmp++)=*(src++);
	while (num--) *(tmp++)=0;
	
	return dest;
}

char *strtok(char *s, const char *delim)
{
	char tmp;
	while (*delim) {
		strtok_save_ptr=strchr(s,tmp);
	}
	return s;
}
