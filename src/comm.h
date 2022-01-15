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

#ifndef _COMM_H
#define _COMM_H

#include <termios.h>

#define DEFAULT_PORT	"/dev/tts/0"
#define DEFAULT_BAUD	B57600;

void comm_init(char *name, int baud);
void comm_clean();
int comm_open(char *name, int b);
void comm_close();
int comm_opened();
int comm_read(char *data, size_t len);
int comm_write(char *data, size_t len);
void comm_info();

static char *set_port_name(char *name);
static void ios_init(int b);

int comm_setrts(int rts);
int comm_setdtr(int dtr);

#endif /* _COMM_H */
