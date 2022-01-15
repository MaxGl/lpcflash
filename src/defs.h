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

#ifndef _DEFS_H
#define _DEFS_H

#define PROJECT		"LPC-Flash"
#define PROJECT_VERSION	"0.2.4"

#define VERBOSE(level, fmt, arg...)			\
		{					\
			if (level <= verbose) {		\
				printf(fmt, ## arg);	\
				fflush(stdout);		\
			}				\
		}

#define ERROR_CMD	0x00
#define ERROR_UC	0x10
#define ERROR_IO	0x20
#define ERROR_BATCH	0x30

#define FREE(x)         if (x != NULL) {free(x); x = NULL;}

#endif /* _DEFS_H */
