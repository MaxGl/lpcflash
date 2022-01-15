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

#ifndef _LPC_H
#define _LPC_H

#define MAX_IO_BUF	256
#define DEFAULT_TIMEOUT	500000	/* 0.5 sec */
#define DEFAULT_FREQ	18432	/* 18432000 Hz */

#define ERR_TIMEOUT		(ERROR_UC + 0x00)
#define ERR_ALIGN		(ERROR_UC + 0x01)
#define ERR_INCOMPLETE		(ERROR_UC + 0x02)
#define ERR_UNRECOGNIZED	(ERROR_UC + 0x03)
#define ERR_CONNECT		(ERROR_UC + 0x04)
#define ERR_READY		(ERROR_UC + 0x05)
#define ERR_COMMAND		(ERROR_UC + 0x06)

#define OP_READ			1
#define OP_WRITE		2
#define OP_ERASE		3
#define OP_FLASH		4

#define XON			0x11
#define XOFF			0x13

struct lpc_device {
	u_int32_t id;
	u_int32_t part;
	u_int32_t flash_size;
	u_int32_t ram_size;
	u_int32_t sectors;
	u_int32_t copy_size;
	u_int32_t *sector_map;
};

void ucprintdevs();
int ucisp(int freq);
int ucwaitresp(char *resp);
int ucwaitcmd(char *ibuf, size_t size, int *waitdata);
struct lpc_device *ucdetect(u_int32_t id);
int ucread(u_int32_t addr, char *buf, size_t len);
int ucwrite(u_int32_t addr, char *buf, size_t len, int level);
int ucerase(u_int32_t start_sector, u_int32_t sector_count);
int ucflash(u_int32_t addr, char *buf, size_t len);
int ucgo(u_int32_t addr, char mode);

#endif /* _LPC_H */
