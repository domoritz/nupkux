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

#include <time.h>
#include <drivers/drivers.h>
#include <kernel/dts.h>
#include <task.h>

#define tick_rate 50

static int _ktimezone = 1;
static int _kdaylight_saving_time = 1; //It's the 27th of July

ULONG ticks = 0;

static void set_pic_timer(int freq)
{
	int divisor = 1193180 / freq;
	outportb(0x43, 0x36);
	outportb(0x40, divisor & 0xFF);
	outportb(0x40, (divisor >> 8) & 0xFF);
}

void timer_handler(registers *regs)
{
	ticks++;
	switch_task();
}

void setup_timer()
{
	set_pic_timer(tick_rate);
	register_interrupt_handler(IRQ0, &timer_handler);
}

static UCHAR DateBCD(UCHAR value, int is_bcd)
{
	int tmp;

	if (!is_bcd) return value;
	tmp = value % 0x10;
	value /= 0x10;
	if ((tmp > 9) || ((value % 0x10) > 9)) return -1;
	tmp += 10 * (value % 0x10);
	return tmp;
}

struct tm getrtctime()
{
	struct tm now;
	int is_bcd;

	outportb(0x70, 0x0B);
	is_bcd = !((inportb(0x71) & 0x04) >> 2);
	outportb(0x70, 0x00);
	now.tm_sec = DateBCD(inportb(0x71), is_bcd);
	outportb(0x70, 0x02);
	now.tm_min = DateBCD(inportb(0x71), is_bcd);
	outportb(0x70, 0x04);
	now.tm_hour = DateBCD(inportb(0x71), is_bcd);
	outportb(0x70, 0x07);
	now.tm_mday = DateBCD(inportb(0x71), is_bcd);
	outportb(0x70, 0x08);
	now.tm_mon = DateBCD(inportb(0x71), is_bcd);
	outportb(0x70, 0x09);
	now.tm_year = DateBCD(inportb(0x71), is_bcd);
	/*//In VirtualBox it's weird, on my laptop everything is fine, on my PC it sucks. Forget the day.
	outportb(0x70,0x06);
	now.tm_wday=inportb(0x71);
	//A stupid bugfix, i've got no idea why it doesn't work correctly in my virtual box:
	now.tm_wday+=2;
	if (now.tm_wday>6) now.tm_wday-=7;*/
	now.tm_wday = -1;

	now.tm_yday = (now.tm_mon - 1) * 30;
	if (now.tm_mon > 2) now.tm_yday -= 2;
	if (now.tm_mon < 7) now.tm_yday += (now.tm_mon) / 2;
	else if (now.tm_mon == 7) now.tm_yday += 3;
	else now.tm_yday += (now.tm_mon - 7) / 2 + 4;
	now.tm_yday += (now.tm_mday - 1);
	if (((!(now.tm_year % 4) && (now.tm_year % 100)) || (!(now.tm_year % 400))) && (now.tm_mon > 2)) now.tm_yday++;
	//Summertime - I know it's called daylight saving time, but our German word sounds nicer
	//by the way, every clock should be set to UTC
	/*outportb(0x70,0x0B);
	now.tm_isdst=(inportb(0x71) & 0x01);*/
	now.tm_isdst = _kdaylight_saving_time;
	return now;
}

time_t mktime(struct tm *timeptr)
{
	//Zeit seit 01.01.1970 berechnen
	time_t result = 0;
	int i;

	//Es wären die Tage zu bestimmen
	if (timeptr->tm_year >= 70) result = (timeptr->tm_year - 70) * 365;
	else result = (timeptr->tm_year + 30) * 365;
	//Schalttage => VERBESSERN
	i = timeptr->tm_year + 1899;
	if (i < 1969) i += 100;
	for (; i >= 1970; i--)
		if ((!(i % 4) && (i % 100)) || (!(i % 400))) result++;
	result += timeptr->tm_yday;
	//Sommerzeit am 06.04.1980 ab 3:00 Uhr
	//Am heutigen Tage ...
	result *= 24;
	result += timeptr->tm_hour;
	result *= 60;
	result += timeptr->tm_min;
	result *= 60;
	result += timeptr->tm_sec;
	return result;
}

time_t time(time_t *tp)
{
	struct tm now = getrtctime();
	time_t result;

	removetimezone(&now);
	result = mktime(&now);
	if (tp) *tp = result;
	return result;
}

void sleep(UINT msecs)
{
	ULONG tstart = ticks, tdiff = (tick_rate * msecs) / 1000;
	while (ticks - tdiff < tstart);
}

int removetimezone(struct tm *timeptr)
{
	timeptr->tm_hour -= _ktimezone;
	timeptr->tm_hour -= timeptr->tm_isdst;
	return _ktimezone + timeptr->tm_isdst;
}
