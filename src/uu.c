/***************************************************************************
 *   Copyright (C) 2006 by Yuri Ovcharenko                                 *
 *   amwsoft@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>

#include "defs.h"
#include "uu.h"

extern int verbose;

int uudecode(char *in, char *out)
{
	int retval = 0;
	int i, j;
	char c;
	char buf[4];

	if (in[0] == 0x60)
		retval = 0;
	else
		retval = in[0] - 0x20;
	if (retval < 1)
		return 0;
	i = 1;
	j = 0;
	while (j < retval) {
		buf[0] = in[i + 0] == 0x60 ? 0 : in[i + 0] - 0x20;
		buf[1] = in[i + 1] == 0x60 ? 0 : in[i + 1] - 0x20;
		buf[2] = in[i + 2] == 0x60 ? 0 : in[i + 2] - 0x20;
		buf[3] = in[i + 3] == 0x60 ? 0 : in[i + 3] - 0x20;

		out[j + 0] = (buf[0] << 2) | ((buf[1] >> 4) & 0x3);
		out[j + 1] = (buf[1] << 4) | ((buf[2] >> 2) & 0xF);
		out[j + 2] = (buf[2] << 6) | ((buf[3]) & 0x3F);

		i += 4;
		j += 3;
	}
	VERBOSE(3, "UU decoded %d bytes.\n", retval);
	return retval;
}

int uuencode(unsigned char *in, size_t len, unsigned char *out)
{
	int retval;
	int i, j;
	u_int32_t buf;
	size_t count = len;

	if (!len)
		return 0;
	if (len > 45)
		return 0;

	while (count % 3) count++;
	out[0] = len;
	i = 0;
	j = 1;
	buf = 0;
	while (i < count) {
		buf <<= 8;
		if (i < len)
			buf |= in[i];
		if ((i % 3) == 2) {
			out[j++] = (buf >> 18) & 0x3F;
			out[j++] = (buf >> 12) & 0x3F;
			out[j++] = (buf >>  6) & 0x3F;
			out[j++] =  buf        & 0x3F;
			buf = 0;
		}
		i++;
	}
	retval = j;
	for (j = 0; j < retval; j++)
		out[j] = out[j] == 0 ? 0x60 : out[j] + 0x20;

	VERBOSE(3, "UU encoded %d bytes.\n", retval);
	return retval;
}

/* End of file */
