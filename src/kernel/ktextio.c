/*
 *  Copyright (C) 2007-2010 Sven Köhler
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

#include <kernel/ktextio.h>
#include <lib/stdarg.h>
#include <mm.h>
#include <kernel/dts.h>
#include <lib/string.h>
#include <drivers/tty.h>

extern devfs_handle *current_tty;

static int _ksetcursor(UCHAR x, UCHAR y);
static int _kout(devfs_handle *handle, off_t offset, size_t size, const char *output);
int (*ktexto)(devfs_handle *, off_t, size_t, const char*) = _kout;

tty_cursor _cursor_pos;

char *mem = (char *) VIDEO_MEM_ENTRY;

extern int vsprintf(char *buf, const char *fmt, va_list args);

int sys_putchar(char chr)
{
	_kputc(chr);
	return chr;
}

int printf(const char *fmt, ...)
{
	char str[STRLEN];
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = vsprintf(str, fmt, ap);
	ktexto(current_tty, 0, strlen(str), str);
	va_end(ap);
	return res;
}

int sprintf(char *str, const char *fmt, ...)
{
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = vsprintf(str, fmt, ap);
	va_end(ap);
	return res;
}

static int _ksetcursor(UCHAR x, UCHAR y)
{
	USHORT position;

	if (x == TXT_WIDTH) {
		if (y == TXT_HEIGHT - 1) return 0;
		y++;
		x = 0;
	}
	if (x > TXT_WIDTH) {
		if (!y) return 0;
		x = TXT_WIDTH - 1;
		y--;
	}
	if (y > TXT_HEIGHT) y = 0;
	if (y == TXT_HEIGHT) y--;
	position = (y * TXT_WIDTH) + x;
	outportb(0x3D4, 0x0F);
	outportb(0x3D5, (USHORT) (position & 0xFF));
	outportb(0x3D4, 0x0E);
	outportb(0x3D5, (USHORT) ((position >> 8) & 0xFF));
	_cursor_pos.x = x;
	_cursor_pos.y = y;
	return 0;
}

int _kclear()
{
	int i = 0;

	while (i < (TXT_WIDTH * TXT_HEIGHT * 2)) {
		mem[i++] = ' ';
		mem[i++] = TXT_COL_WHITE;
	}
	_ksetcursor(0, 0);
	return 0;
}

static int _kout(devfs_handle *dummy, off_t offset, size_t size, const char *output)
/* This method is only to be used on start up, before we can use ttys */
{
	UCHAR x = _cursor_pos.x, y = _cursor_pos.y;
	int i;

	while (size--) {
		switch (*output) {
		case '\n':
#ifdef NEWLINE_RETURN
			for (i = 2 * ((y + 1) * TXT_WIDTH); i < 2 * TXT_HEIGHT * TXT_WIDTH; i++)
				mem[i] = mem[i+2*x];
			x = 0;
#endif
			y++;
			break;
		case '\r':
			x = 0;
			break;
		case '\t':
			i = TAB_WIDTH - (x % TAB_WIDTH);
			x += i + 1;
			break;
		case '\b':
			if (!x) {
				if (!y) break;
				y--;
				x = TXT_WIDTH;
			}
			x--;
			for (i = 2 * (y * TXT_WIDTH + x); i < 2 * TXT_HEIGHT * TXT_WIDTH; i++)
				mem[i] = mem[i+2];
			break;
		case '\v':
			y++;
			break;
		case '\f':
			y++;
			break;
		default:
			mem[2*(y*TXT_WIDTH+x)] = *output;
			mem[2*(y*TXT_WIDTH+x)+1] = TXT_COL_WHITE;
			x++;
			break;
		}
		if (x >= TXT_WIDTH) {
			x -= TXT_WIDTH;
			y++;
		}
		if (y >= TXT_HEIGHT) {
			y = 0;
			while (y < TXT_HEIGHT - 1) {
				memcpy(mem + 2 * (y * TXT_WIDTH), mem + 2 * ((y + 1)*TXT_WIDTH), (UINT) 2 * TXT_WIDTH);
				y++;
			}
			for (y = 0; y < TXT_WIDTH; y++) {
				mem[2*((TXT_HEIGHT-1)*TXT_WIDTH+y)] = ' ';
				mem[2*((TXT_HEIGHT-1)*TXT_WIDTH+y)+1] = TXT_COL_WHITE;
			}
			y = TXT_HEIGHT - 1;
		}
		output++;
	}
	_ksetcursor(x, y);
	return 0;
}

int _kputc(const char chr)
{
	return ktexto(current_tty, 0, 1, (char *)&chr);
}
