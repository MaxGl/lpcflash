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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"
#include "lpc.h"
#include "comm.h"
#include "uu.h"

extern int verbose;

int timeout = DEFAULT_TIMEOUT;
int connected = 0;
extern int freq;
struct lpc_device *lpcdev = NULL;

/* Map does not include Boot code */

static u_int32_t map_210x[] = {
	8192,	8192,	8192,	8192,	8192,	8192,	8192,	8192,
	8192,	8192,	8192,	8192,	8192,	8192,	8192
};

static u_int32_t map_211x[] = {
	8192,	8192,	8192,	8192,	8192,	8192,	8192,	8192,
	8192,	8192,	8192,	8192,	8192,	8192,	8192
};

static u_int32_t map_212x[] = {
	8192,	8192,	8192,	8192,	8192,	8192,	8192,	8192,
	65536,	65536,
	8192,	8192,	8192,	8192,	8192,	8192,	8192
};

static u_int32_t map_213x[] = {
	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,
	32768,	32768,	32768,	32768,	32768,	32768,	32768,	32768,
	32768,	32768,	32768,	32768,	32768,	32768,	4096,	4096,
	4096,	4096,	4096
};

static u_int32_t map_2364[] = {
	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,
	32768,	32768,	32768
};

static u_int32_t map_2366[] = {
	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,
	32768,	32768,	32768,	32768,	32768,	32768,	32768
};

static u_int32_t map_2368[] = {
	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,
	32768,	32768,	32768,	32768,	32768,	32768,	32768,	32768,
	32768,	32768,	32768,	32768,	32768,	32768,
	4096,	4096,	4096,	4096,	4096,	4096
};

static u_int32_t map_2378[] = {
	4096,	4096,	4096,	4096,	4096,	4096,	4096,	4096,
	32768,	32768,	32768,	32768,	32768,	32768,	32768,	32768,
	32768,	32768,	32768,	32768,	32768,	32768,
	4096,	4096,	4096,	4096,	4096,	4096
};

static struct lpc_device lpc[] = {
	{ 0x00313F31, 2103,  32,  8,  8, 4096, map_210x },
	{ 0xFFF0FF12, 2104, 128, 16, 15, 8192, map_210x },
	{ 0xFFF0FF22, 2105, 128, 32, 15, 8192, map_210x },
	{ 0xFFF0FF32, 2106, 128, 64, 15, 8192, map_210x },
	{ 0x0101FF12, 2114, 128, 16, 15, 8192, map_211x },
	{ 0x0201FF12, 2119, 128, 16, 15, 8192, map_211x },
	{ 0x0101FF13, 2124, 256, 16, 17, 8192, map_212x },
	{ 0x0201FF13, 2129, 256, 16, 17, 8192, map_212x },
	{ 0x0002FF01, 2131,  32,  8,  8, 4096, map_213x },
	{ 0x0002FF11, 2132,  64, 16,  9, 4096, map_213x },
	{ 0x0002FF12, 2134, 128, 16, 11, 4096, map_213x },
	{ 0x0002FF23, 2136, 256, 32, 15, 4096, map_213x },
	{ 0x0002FF25, 2138, 512, 32, 27, 4096, map_213x },
	{ 0x0402FF01, 2141,  32,  8,  8, 4096, map_213x },
	{ 0x0402FF11, 2142,  64, 16,  9, 4096, map_213x },
	{ 0x0402FF12, 2144, 128, 16, 11, 4096, map_213x },
	{ 0x0402FF23, 2146, 256, 40, 15, 4096, map_213x },
	{ 0x0402FF25, 2148, 512, 40, 27, 4096, map_213x },
	{ 0x0301FF13, 2194, 256, 16, 17, 8192, map_212x },
	{ 0x0301FF12, 2210,   0, 16,  0, 8192, map_211x }, /* ROMless? */
	{ 0x0401FF12, 2212, 128, 16, 15, 8192, map_211x },
	{ 0x0601FF13, 2214, 256, 16, 17, 8192, map_212x },
	/*{ 2290 has same id as 2210 },*/
	{ 0x0401FF13, 2292, 256, 16, 17, 8192, map_212x },
	{ 0x0501FF13, 2294, 256, 16, 17, 8192, map_212x },
	{ 0x0603FB02, 2364, 128, 34, 11, 4096, map_2364 },
	{ 0x0603FB23, 2366, 256, 34, 15, 4096, map_2366 },
	{ 0x0603FB25, 2368, 512, 34, 28, 4096, map_2368 },
	{ 0x0703FF25, 2378, 512, 34, 28, 4096, map_2378 },
	{ 0x1700FD25, 2378, 512, 34, 28, 4096, map_2378 }, /* Rev B */
	{ 0x0603FF35, 2468, 512, 98, 28, 4096, map_2368 },
	/*
	{ 0x        , 2478, 512, 98, 28, 4096, map_2368 },
	*/
	/******************************************************************/
	{ 0, 0, 0 },  /* Last id must be zero */
};

void ucprintdevs()
{
	int i = 0;

	printf("\tSupported devices:\n");
	while (lpc[i].id) {
		if (!(i & 3))
			printf("\t");
		printf("\tLPC%d", lpc[i].part);
		if ((i & 3) == 3)
			printf("\n");
		i++;
	}
	printf("\n");
}

int ucisp(int freq)
{
	int retval;
	char obuf[MAX_IO_BUF];
	char ibuf[MAX_IO_BUF];
	int i;
	u_int32_t data;
	char *C;
	char *ok;

	VERBOSE(2, "Entering to ISP mode\n");
	retval = comm_setdtr(1);
	if (retval)
		return retval;
	retval = comm_setrts(1);
	if (retval)
		return retval;

	usleep(100000);
	for (i = 0; i < 16; i++)
		comm_read(ibuf, MAX_IO_BUF);

	usleep(100000);
	retval = comm_setdtr(0);
	if (retval)
		return retval;
	usleep(100000);
	retval = comm_setrts(0);
	if (retval)
		return retval;

	i = 16;
	while (i) {
		VERBOSE(3, "Autobaud sequence #%d\n", 17 - i);
		obuf[0] = '?';
		retval = comm_write(obuf, 1);
		if (retval < 0) {
			perror("Can not write to port");
			return retval;
		}
		/* Wait for 'Synchronized<CR><LF>' */
		if (ucwaitresp("Synchronized\r\n")) {
			break;
		}
		i--;
	}

	if (!i) {
		fprintf(stderr, "Can not enter to ISP mode.\n");
		return 1;
	}

	VERBOSE(2, "--> Synchronized\n");

	sprintf(obuf, "%s\r\n", "Synchronized");
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}

	if (!ucwaitresp(obuf)) {
		fprintf(stderr, "Can not enter to ISP mode.\n");
		return 1;
	}
	if (!ucwaitresp("OK\r\n")) {
		fprintf(stderr, "Can not enter to ISP mode.\n");
		return 1;
	}

	VERBOSE(2, "--> OK\n");
	VERBOSE(2, "Send frequency (%d kHz)\n", freq);

	sprintf(obuf, "%d\r\n", freq);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}
	if (!ucwaitresp(obuf)) {
		fprintf(stderr, "Can not enter to ISP mode.\n");
		return 1;
	}
	if (!ucwaitresp("OK\r\n")) {
		fprintf(stderr, "Can not enter to ISP mode.\n");
		return 1;
	}

	VERBOSE(2, "--> OK\n");

	VERBOSE(3, "Turning echo off...\n");
	sprintf(obuf, "A 0\r\n");
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}
	strcat(obuf, "0\r\n");
	if (!ucwaitresp(obuf)) {
		fprintf(stderr, "Can not turn off echo.\n");
		return 1;
	}

	connected = 1;

	VERBOSE(2, "Request Part ID.\n");
	sprintf(obuf, "J\r\n");
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}

	data = 1;
	if (!ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data)) {
		fprintf(stderr, "Can not get answer for command.\n");
		return 1;
	}
	VERBOSE(2, "Part ID = '%s'\n", &ibuf[data]);
	C = strstr(&ibuf[data], "\r\n");
	if (C == NULL) {
		fprintf(stderr, "Can not find Part ID.\n");
		return ERR_UNRECOGNIZED;
	} else {
		struct lpc_device *dev;
		*C = '\0';
		data = strtoul(&ibuf[data], &ok, 0);
		if (!(*ok == '\0')) {
			fprintf(stderr, "Invalid Part ID.\n");
			return ERR_UNRECOGNIZED;
		} else {
			dev = ucdetect(data);
			if (dev == NULL) {
				fprintf(stderr, "Can not determine CPU.\n");
				return ERR_UNRECOGNIZED;
			} else {
				VERBOSE(0, "LPC%d detected.\n", dev->part);
				lpcdev = dev;
			}
		}
	}

	VERBOSE(2, "Request Boot code version\n");
	sprintf(obuf, "K\r\n");
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}

	data = 1;
	if (!ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data)) {
		fprintf(stderr, "Can not get answer for command.\n");
		return 1;
	}
	i = data;
	data = 0;
	VERBOSE(0, "Boot code version = %s", ibuf[i] == '\r' ? "UNKNOWN" : "");
	while (ibuf[i] != '\r') {
		VERBOSE(0, "%c", ibuf[i++]);
		if ((data++))
			VERBOSE(0, ".");
	}
	VERBOSE(0, "\n");

	VERBOSE(2, "Done\n");
	return 0;
}

int ucwaitresp(char *resp)
{
	int retval;
	int ok = 0;
	int pos;
	int i;
	struct timeval tv, tvc;
	int usesleep = 0;
	int tm = 10;
	char ibuf[MAX_IO_BUF];

	memset(ibuf, 0, sizeof(ibuf));
	pos = 0;
	VERBOSE(3, "Wait for response: '%s'\n", resp);
	retval = gettimeofday(&tv, NULL);
	if (retval < 0) {
		perror("Can not get current time");
		usesleep = 1;
		tm = 10;
	} else {
		tvc.tv_sec = tv.tv_sec;
		tvc.tv_usec = tv.tv_usec;
	}
	while ((((tvc.tv_sec * 1000000 + tvc.tv_usec) - (tv.tv_sec * 1000000 + tv.tv_usec)) < (timeout * 2)) && (pos <= strlen(resp)) && (tm)) {
		retval = comm_read(&ibuf[pos], 1);
		if (retval < 0) {
			perror("Can not read port");
			return 0;
		}
		if (retval) {
			VERBOSE(4, "%c", ibuf[pos]);
			pos += retval;
			if (!strcmp(ibuf, resp)) {
				ok = 1;
				break;
			}
		}
		if (usesleep) {
			usleep(timeout / 10);
			--tm;
		} else {
			retval = gettimeofday(&tvc, NULL);
			if (retval < 0) {
				perror("Can not get current time");
				usesleep = 1;
			}
		}
	}

	if (!strcmp(ibuf, resp))
		ok = 1;

	if (!ok) {
		if (pos) {
			if (pos < strlen(resp))
				fprintf(stderr, "Incomplete response: '%s'.\n", ibuf);
			else
				fprintf(stderr, "Unrecognized response: '%s'.\n", ibuf);
		} else {
			fprintf(stderr, "Microcontroller not responding.\n");
		}
	}

	return ok;
}

int ucwaitcmd(char *ibuf, size_t size, int *waitdata)
{
	int retval;
	int ok = 0;
	int pos;
	int i;
	struct timeval tv, tvc;
	int usesleep = 0;
	int tm = 10;
	char *wstat = NULL;
	int wdata = *waitdata;

	*waitdata = 0;
	pos = 0;
	memset(ibuf, 0, size);
	VERBOSE(3, "Wait for command response%s.\n", wdata ? " with data" : "");
	retval = gettimeofday(&tv, NULL);
	if (retval < 0) {
		perror("Can not get current time");
		usesleep = 1;
		tm = 10;
	} else {
		tvc.tv_sec = tv.tv_sec;
		tvc.tv_usec = tv.tv_usec;
	}
	while ((((tvc.tv_sec * 1000000 + tvc.tv_usec) - (tv.tv_sec * 1000000 + tv.tv_usec)) < (timeout * 2)) && (tm) && (pos < (size - 1))) {
		retval = comm_read(&ibuf[pos], 1);
		if (retval < 0) {
			perror("Can not read port");
			return 0;
		}
		if (retval) {
			if (verbose >= 4) {
				VERBOSE(4, "%c", &ibuf[pos]);
			}
			pos += retval;
			if (wstat == NULL) {
				wstat = strstr(ibuf, "\r\n");	/* Find status */
				if (wstat != NULL) {
					if(wdata) {
						wstat += 2;	/* Find value next*/
					} else {
						ok = 1;
						break;
					}
				}
			} else {
				if (strstr(wstat, "\r\n")) {
					ok = 1;
					*waitdata = (unsigned int)(wstat - ibuf);
					break;
				}
			}
		}
		if (usesleep) {
			usleep(timeout / 10);
			--tm;
		} else {
			retval = gettimeofday(&tvc, NULL);
			if (retval < 0) {
				perror("Can not get current time");
				usesleep = 1;
			}
		}
	}

	if (!ok)
		fprintf(stderr, "Microcontroller not responding to command.\n");

	return ok;
}

struct lpc_device *ucdetect(u_int32_t id)
{
	int i = 0;
	struct lpc_device *dev = NULL;
	while (lpc[i].id) {
		if (lpc[i].id == id) {
			dev = &lpc[i];
			break;
		}
		i++;
	}
	return dev;
}

int ucread(u_int32_t addr, char *buf, size_t len)
{
	int retval;
	char obuf[MAX_IO_BUF];
	char ibuf[MAX_IO_BUF];
	u_int32_t data = 0;
	int i;
	int count;
	int size;
	unsigned char c;
	char *out = buf;
	int k;
	int line;
	u_int32_t crc, rcrc;

	if (!connected) {
		retval = ucisp(freq);
		if (retval) {
			return ERR_CONNECT;
		}
	}
	if (addr & 3)
		return ERR_ALIGN;
	if (len & 3)
		return ERR_ALIGN;
	if (!len)
		return ERR_ALIGN;






	VERBOSE(2, "Remap to FLASH.\n");
	*((u_int32_t*)obuf) = 0x00000001;
	retval = ucwrite(0xE01FC040U, obuf, 4, 1);
	if (retval) {
		fprintf(stderr, "Can not remap: Invalid response: %s\n", &ibuf[data]);
		return ERR_INCOMPLETE;
	}

	VERBOSE(0, "Read memory.\n");
	sprintf(obuf, "R %d %d\r\n", addr, len);
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}

	data = 1;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}

	VERBOSE(3, "Receiving data\n");
	if (ibuf[data] == '0') {
		count = len;
		line = 0;
		crc = 0;
		while (count) {
			i = 0;
			memset(ibuf, 0, sizeof(ibuf));
			while (1) {
				retval = comm_read(&c, 1);
				if (retval < 0) {
					perror("Can not read data");
					return retval;
				}
				if (retval > 0) {
					ibuf[i] = c;
					VERBOSE(3, c < 0x20 ? " 0x%02X " : "%c", c);
				}
				if (ibuf[i] == '\n')
					break;
				i += retval;
			}
			line++;
			if (line == 21) {
				line = 0;
				ibuf[i] = 0;
				rcrc = strtoul(ibuf, NULL, 0);
				VERBOSE(1, "\nCHECKSUM: %lu : %lu\n", crc, rcrc);
				if (crc != rcrc) {
					fprintf(stderr, "\nChecksum error!\n");
					return ERR_INCOMPLETE;
				}
				crc = 0;
				sprintf(obuf, "OK\r\n");
				retval = comm_write(obuf, strlen(obuf));
				if (retval < 0) {
					perror("Can not write to port");
					return retval;
				}
			} else {
				retval = uudecode(ibuf, out);
				if (verbose >= 3) {
					printf("\n");
					for (k = 0; k < retval; k++)
						VERBOSE(3, " %02X", (unsigned char)out[k]);
					printf("\n");
				}
				for (k = 0; k < retval; k++)
					crc += (unsigned char)out[k];
				count -= retval;
				out += retval;
				VERBOSE(0, "\rRead %d / %d bytes ", len - count, len);
			}
		}
	} else {
		fprintf(stderr, "Error response: '%s'\n", ibuf);
		return ERR_COMMAND;
	}

	if ((line) && (line != 21)) {
		data = 0;
		retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
		if (!retval) {
			fprintf(stderr, "Error response: '%s'.\n", ibuf);
			return retval;
		}
		VERBOSE(3, "%s", ibuf);
		rcrc = strtoul(ibuf, NULL, 0);
		VERBOSE(1, "\nCHECKSUM: %lu : %lu\n", crc, rcrc);
		if (crc != rcrc) {
			fprintf(stderr, "\nChecksum error!\n");
			return ERR_INCOMPLETE;
		}
		sprintf(obuf, "OK\r\n");
		retval = comm_write(obuf, strlen(obuf));
		if (retval < 0) {
			perror("Can not write to port");
			return retval;
		}
	}

	VERBOSE(0, "\nDone\n");
	return 0;
}

int ucwrite(u_int32_t addr, char *buf, size_t len, int level)
{
	int retval;
	char obuf[MAX_IO_BUF];
	char ibuf[MAX_IO_BUF];
	u_int32_t data = 0;
	int i;
	int count;
	int size;
	unsigned char c;
	char *out = buf;
	int k;
	int line;
	u_int32_t crc, rcrc;
	size_t block;
	int tryes;
	int max;
	int blocksize;

	if (!connected) {
		retval = ucisp(freq);
		if (retval) {
			return ERR_CONNECT;
		}
	}
	if (addr & 3)
		return ERR_ALIGN;
	if (len & 3)
		return ERR_ALIGN;
	if (!len)
		return ERR_ALIGN;

	VERBOSE(0, "Writing memory.\n");

	sprintf(obuf, "W %u %u\r\n", addr, len);
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}

	data = level ? 0 : 1;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}

	if (ibuf[data] == '0') {
		VERBOSE(3, "Sending data\n");
		count = len;
		line = 0;
		block = 0;
		tryes = 0;
		while (count) {
			max = count > 45 ? 45 : count;
			VERBOSE(2, "Encode %d bytes offset %d\n", max, block + line * 45);
			memset(obuf, 0, sizeof(obuf));
			size = uuencode(&buf[block + line * 45], max, obuf);
			obuf[size++] = '\r';
			obuf[size++] = '\n';
			VERBOSE(3, obuf);
			usleep(10000);
			retval = comm_write(obuf, strlen(obuf));
			if (retval < 0) {
				perror("Can not write to port");
				return retval;
			}
			retval = comm_read(ibuf, 16);
			if (retval < 0) {
				perror("Can not read port");
				return retval;
			}
			for (i = 0; i < retval; i++) {
				if ((ibuf[i] == XON) || (ibuf[i] == XOFF)) {
					VERBOSE(2, "\n%s\n", ibuf[i] == XON ? "XON" : "XOFF");
				}
			}
			line++;
			count -= max;
			VERBOSE(0, "\rWrite %d / %d bytes (try %d) ", len - count, len, tryes + 1);
			if ((line == 20) || (count == 0)) {
				crc = 0;
				blocksize = max == 45 ? line * 45 : ((line - 1) * 45) + max;
				for (i = block; i < block + blocksize; i++) {
					crc += (unsigned char)buf[i];
				}
				VERBOSE(1, "\nCHECKSUM for %d bytes: %d\n", blocksize, crc);
				sprintf(obuf, "%lu\r\n", crc);
				retval = comm_write(obuf, strlen(obuf));
				if (retval < 0) {
					perror("Can not write to port");
					return retval;
				}
				data = 0;
				if (!ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data)) {
					fprintf(stderr, "Can not get answer for command.\n");
					return ERR_TIMEOUT;
				}
				if (strcmp(ibuf, "OK\r\n") == 0) {
					block += blocksize;
					tryes = 0;
				} else {
					fprintf(stderr, "CHECKSUM not accepted! Resending block.\n");
					count = len - block;
					line = 0;
					tryes++;
					if (tryes == /*16*/256) {
						fprintf(stderr, "Maximum retry count reached!\n");
						return ERR_INCOMPLETE;
					}
				}
			}
		}
	} else {
		fprintf(stderr, "Error response: '%s'\n", ibuf);
		return ERR_COMMAND;
	}

	VERBOSE(0, "\nDone\n");
	return 0;
}

int ucgo(u_int32_t addr, char mode)
{
	int retval;
	int data;
	char obuf[MAX_IO_BUF];
	char ibuf[MAX_IO_BUF];
	char *c;

	if (!connected) {
		retval = ucisp(freq);
		if (retval) {
			return ERR_CONNECT;
		}
	}
	if (addr & 3)
		return ERR_ALIGN;

	VERBOSE(0, "Executing GO command.\n");

	sprintf(obuf, "U 23130\r\n");
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}
	data = 0;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}
	if (ibuf[0] != '0') {
		c = strstr(ibuf, "\r");
		if (c != NULL)
			*c = 0;
		fprintf(stderr, "Can not Unlock: Invalid response: %s\n", ibuf);
		return ERR_INCOMPLETE;
	}

	sprintf(obuf, "G %d %c\r\n", addr, mode);
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}

	data = 0;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}
	if (ibuf[0] != '0') {
		c = strstr(ibuf, "\r");
		if (c != NULL)
			*c = 0;
		fprintf(stderr, "Can not GO: Invalid response: %s\n", ibuf);
		return ERR_INCOMPLETE;
	}
	VERBOSE(0, "Done.\n");
	return 0;
}

int ucerase(u_int32_t start_sector, u_int32_t sector_count)
{
	int retval;
	char obuf[MAX_IO_BUF];
	char ibuf[MAX_IO_BUF];
	int data;
	char *c;

	if (lpcdev == NULL) {
		fprintf(stderr, "CPU undefined!\n");
		return ERR_UNRECOGNIZED;
	}
	if (sector_count == 0) {
		fprintf(stderr, "Nothing to do!\n");
		return ERR_INCOMPLETE;
	}
	if ((start_sector + sector_count) >= lpcdev->sectors) {
		fprintf(stderr, "Requested region is exceeded CPU Flash!\n");
		return ERR_INCOMPLETE;
	}
	VERBOSE(0, "Erasing flash.\n");

	VERBOSE(2, "Unlock.\n");
	sprintf(obuf, "U 23130\r\n");
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}
	data = 1;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}
	if (ibuf[data] != '0') {
		c = strstr(&ibuf[data], "\r");
		if (c != NULL)
			*c = 0;
		fprintf(stderr, "Can not Unlock: Invalid response: %s\n", &ibuf[data]);
		return ERR_INCOMPLETE;
	}

	VERBOSE(2, "Prepare.\n");
	sprintf(obuf, "P %d %d\r\n", start_sector, start_sector + sector_count - 1);
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}
	data = 0;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}
	if (ibuf[data] != '0') {
		c = strstr(&ibuf[data], "\r");
		if (c != NULL)
			*c = 0;
		fprintf(stderr, "Can not prepare: '%s'\n", &ibuf[data]);
		return ERR_COMMAND;
	}

	VERBOSE(2, "Erasing.\n");
	sprintf(obuf, "E %d %d\r\n", start_sector, start_sector + sector_count - 1);
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}
	data = 0;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}
	if (ibuf[data] != '0') {
		c = strstr(&ibuf[data], "\r");
		if (c != NULL)
			*c = 0;
		fprintf(stderr, "Can not erase: '%s'\n", &ibuf[data]);
		return ERR_COMMAND;
	}

	VERBOSE(0, "Done.\n");
	return 0;
}

int ucflash(u_int32_t addr, char *buf, size_t len)
{
	int retval;
	char obuf[MAX_IO_BUF];
	char ibuf[MAX_IO_BUF];
	char iobuf[512];
	int data;
	char *c;
	int start_sector;
	int end_sector;
	int tmp = verbose;
	int count;
	int max;
	int block;
	int i;
	u_int32_t signature = 0;
	u_int32_t *vectors = (u_int32_t *)buf;

	if (lpcdev == NULL) {
		fprintf(stderr, "CPU undefined!\n");
		return ERR_UNRECOGNIZED;
	}
	if (addr & 0x1FF) {
		fprintf(stderr, "Address not aligned!\n");
		return ERR_ALIGN;
	}
	if ((addr + len) > lpcdev->flash_size * 1024) {
		fprintf(stderr, "Requested region is exceeded CPU Flash!\n");
		return ERR_INCOMPLETE;
	}
	VERBOSE(0, "Programming flash.\n");

	if ((addr == 0) && (len < 32)) {
		fprintf(stderr, "Flash programming at address 0 and size less then 32 bytes may produce unpredictable result!\n");
	}

	if ((addr == 0) && (len >= 32)) {
		for (i = 0; i < 8; i++) {
			if (i != 5)
				signature += vectors[i];
		}
		vectors[5] = (u_int32_t)0 - signature;
		VERBOSE(2, "Boot signature: 0x%08X\n", vectors[5]);
	}

	data = 0;
	i = 0;
	start_sector = -1;
	end_sector = -1;
	while (i < lpcdev->sectors) {
		if ((addr >= data) && (addr < data + lpcdev->sector_map[i])) {
			start_sector = i;
			break;
		}
		data += lpcdev->sector_map[i];
		i++;
	}
	while (i < lpcdev->sectors) {
		data += lpcdev->sector_map[i];
		if (addr + len <= data) {
			end_sector = i;
			break;
		}
		i++;
	}
	if ((start_sector < 0) || (end_sector < 0)) {
		fprintf(stderr, "Can not determine sectors to be prepared!\n");
		return ERR_UNRECOGNIZED;
	}

	VERBOSE(2, "Unlock.\n");
	sprintf(obuf, "U 23130\r\n");
	VERBOSE(3, obuf);
	retval = comm_write(obuf, strlen(obuf));
	if (retval < 0) {
		perror("Can not write to port");
		return retval;
	}
	data = 1;
	retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
	if (!retval) {
		fprintf(stderr, "Error response: '%s'.\n", ibuf);
		return retval;
	}
	if (ibuf[data] != '0') {
		c = strstr(&ibuf[data], "\r");
		if (c != NULL)
			*c = 0;
		fprintf(stderr, "Can not Unlock: Invalid response: %s\n", &ibuf[data]);
		return ERR_INCOMPLETE;
	}

	count = len;
	block = 0;
	while (count) {
		max = count > 512 ? 512 : count;
		memset(iobuf, 0xFF, sizeof(iobuf));
		memcpy(iobuf, &buf[block], max);
		verbose = -1;
		retval = ucwrite(0x40000200, iobuf, max, 1);
		verbose = tmp;
		if (retval) {
			fprintf(stderr, "Error programming flash (%d).\n", retval);
			return retval;
		}
		VERBOSE(2, "Prepare.\n");

		sprintf(obuf, "P %d %d\r\n", start_sector, end_sector);
		VERBOSE(3, obuf);
		retval = comm_write(obuf, strlen(obuf));
		if (retval < 0) {
			perror("Can not write to port");
			return retval;
		}
		data = 0;
		retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
		if (!retval) {
			fprintf(stderr, "Error response: '%s'.\n", ibuf);
			return retval;
		}
		if (ibuf[data] != '0') {
			c = strstr(&ibuf[data], "\r");
			if (c != NULL)
				*c = 0;
			fprintf(stderr, "Can not prepare: '%s'\n", &ibuf[data]);
			return ERR_COMMAND;
		}

		VERBOSE(2, "Copy RAM to Flash.\n");
		sprintf(obuf, "C %d %d %d\r\n", addr + block, 0x40000200, 512);
		VERBOSE(3, "%s", obuf);
		retval = comm_write(obuf, strlen(obuf));
		if (retval < 0) {
			perror("Can not write to port");
			return retval;
		}
		data = 0;
		retval = ucwaitcmd(ibuf, sizeof(ibuf), (int *)&data);
		if (!retval) {
			fprintf(stderr, "Error response: '%s'.\n", ibuf);
			return retval;
		}
		if (ibuf[data] != '0') {
			c = strstr(&ibuf[data], "\r");
			if (c != NULL)
				*c = 0;
			fprintf(stderr, "Can not copy RAM to Flash: '%s'\n", &ibuf[data]);
			return ERR_COMMAND;
		}

		count -= max;
		block += max;
		VERBOSE(0, "\rProgrammed %d / %d bytes ", len - count, len);
	}

	VERBOSE(0, "\nDone.\n");
	return 0;
}


/* End of file */
