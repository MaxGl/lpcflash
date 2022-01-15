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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"
#include "comm.h"
#include "lpc.h"

int verbose = 0;
int freq = DEFAULT_FREQ;

struct option long_opt [] = {
	{"interface", required_argument, 0, 'i'},
	{"baud", required_argument, 0, 'b'},
	{"freq", required_argument, 0, 'f'},

	{"read", required_argument, 0, 'r'},
	{"write", required_argument, 0, 'w'},
	{"erase", required_argument, 0, 'e'},
	{"programm", required_argument, 0, 'p'},

	{"go", required_argument, 0, 'g'},

	{"file", required_argument, 0, 'l'},

	{"verbose", no_argument, 0, 'v'},
	{"quiet", no_argument, 0, 'q'},
	{"version", no_argument, 0, 'V'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0},
};

void usage();

int main(int argc, char *argv[])
{
	int retval;
	int i;
	int opt;
	char *c = NULL;
	int opt_index;
	char *port = NULL;
	int baud = 0;
	char *ok;
	char *cmd = NULL;
	int operation = 0;
	u_int32_t addr;
	u_int32_t goaddr;
	char gomode;
	int goflag = 0;
	size_t size, msize;
	char *filename = NULL;
	int fd;
	size_t pos;
	size_t count;
	int n;
	struct stat st;

	while (1) {
		opt = getopt_long(argc, argv, "i:b:f:r:w:e:p:g:l:vqVh", long_opt, &opt_index);
		if (opt == -1)
			break;
		switch (opt) {
			case 0:
				printf("\ngetopt_long == 0\noption == '%s'\n", long_opt[opt_index].name);
				if (optarg)
					printf(" with arg '%s'", optarg);
				printf("\n");
				break;
			case 'i':
				port = optarg;
				break;
			case 'b':
				baud = strtol(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid baud rate: '%s'\n", optarg);
					return 254;
				}
				break;
			case 'f':
				freq = strtol(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid frequency: '%s'\n", optarg);
					return 254;
				}
				if ((freq < 10000) || (freq > 100000)){
					fprintf(stderr, "Invalid frequency: '%d kHz'\n", freq);
					return 254;
				}
				break;
			case 'r':
				if (operation) {
					fprintf(stderr, "Only one operation may be specified.\n");
					return 254;
				}
				c = strstr(optarg, ":");
				if (c == NULL) {
					fprintf(stderr, "Invalid argument format for -%c option.\n", opt);
					return 254;
				}
				*c = 0;
				c++;
				addr = strtoul(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid address: '%s'\n", optarg);
					return 254;
				}
				if (addr & 3) {
					fprintf(stderr, "Invalid address: '%s'\n", optarg);
					return 254;
				}
				size = strtoul(c, &ok, 0);
				c--;
				*c = ':';
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid size: '%s'\n", optarg);
					return 254;
				}
				if (size == 0) {
					fprintf(stderr, "ZERO size.\n");
					return 254;
				}
				if (size & 3) {
					fprintf(stderr, "Invalid size: '%s'\n", optarg);
					return 254;
				}
				VERBOSE(2, "Operation READ 0x%08X:%d.\n", addr, size);
				operation = OP_READ;
				break;
			case 'w':
				if (operation) {
					fprintf(stderr, "Only one operation may be specified.\n");
					return 254;
				}
				addr = strtoul(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid address: '%s'\n", optarg);
					return 254;
				}
				if (addr & 3) {
					fprintf(stderr, "Invalid address: '%s'\n", optarg);
					return 254;
				}
				VERBOSE(2, "Operation WRITE 0x%08X.\n", addr);
				operation = OP_WRITE;
				break;
			case 'e':
				if (operation) {
					fprintf(stderr, "Only one operation may be specified.\n");
					return 254;
				}
				c = strstr(optarg, ":");
				if (c == NULL) {
					fprintf(stderr, "Invalid argument format for -%c option.\n", opt);
					return 254;
				}
				*c = 0;
				addr = strtoul(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid sector number: '%s'\n", optarg);
					return 254;
				}
				*c++ = ':';
				size = strtoul(c, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid setor count: '%s'\n", c);
					return 254;
				}
				if (size == 0) {
					fprintf(stderr, "ZERO sector count.\n");
					return 254;
				}
				VERBOSE(2, "Operation ERASE %d:%d.\n", addr, size);
				operation = OP_ERASE;
				break;
			case 'p':
				if (operation) {
					fprintf(stderr, "Only one operation may be specified.\n");
					return 254;
				}
				addr = strtoul(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid address: '%s'\n", optarg);
					return 254;
				}
				if (addr & 0x1FF) {
					fprintf(stderr, "Invalid address: '%s'\n", optarg);
					return 254;
				}
				VERBOSE(2, "Operation PROGRAM 0x%08X.\n", addr);
				operation = OP_FLASH;
				break;
			case 'g':
				c = strstr(optarg, ":");
				if (c == NULL) {
					fprintf(stderr, "Invalid argument format for -%c option.\n", opt);
					return 254;
				}
				*c = 0;
				c++;
				goaddr = strtoul(optarg, &ok, 0);
				if (!(*ok == '\0')) {
					fprintf(stderr, "Invalid address for -%c option: '%s'\n", opt, optarg);
					return 254;
				}
				if (goaddr & 3) {
					fprintf(stderr, "Invalid address for -%c option: '%s'\n", opt, optarg);
					return 254;
				}
				gomode = *c;
				c--;
				*c = ':';
				switch (gomode) {
					case 'T':
					case 't':
						gomode = 'T';
						break;
					case 'A':
					case 'a':
						gomode = 'A';
						break;
					default:
						fprintf(stderr, "Invalid mode for -%c option: '%s'\n", opt, optarg);
						return 254;
						break;
				}
				goflag = 1;
				break;
			case 'l':
				filename = optarg;
				break;
			case 'v':
				++verbose;
				break;
			case 'q':
				verbose = -1;
				break;
			case 'V':
				printf(PROJECT " version " PROJECT_VERSION "\n");
				return 0;
				break;
			case 'h':
				usage();
				return 0;
				break;
			case '?':
				return 254;
				break;
		}
	}

	c = NULL;

	comm_init(port, baud);

	retval = ucisp(freq);
	if (retval) {
		comm_clean();
		return 2;
	}

	switch (operation) {
		/***************************************************/
		case OP_READ:
			c = malloc(size);
			if (c == NULL) {
				perror("Can not allocate memory");
			} else {
				retval = ucread(addr, c, size);
				if (!retval) {
					if (filename != NULL) {
						fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
						if (fd < 0) {
							perror("Can not create file");
						} else {
							retval = write(fd, c, size);
							if (retval < 0) {
								perror("Can not write data to file");
							}
							close(fd);
						}
					} else {
						for (i = 0; i < (size >> 2); i++) {
							printf("0x%08X\n", ((u_int32_t *)c)[i]);
						}
					}
				}
			}
			break;
		/***************************************************/
		case OP_WRITE:
			if (filename == NULL) {
				fprintf(stderr, "File to write is not specified.\n");
				return 254;
			}
			retval = stat(filename, &st);
			if (retval < 0) {
				perror("Can not stat file");
				return 254;
			}
			if (!S_ISREG(st.st_mode)) {
				fprintf(stderr, "%s is not a regular file.\n", filename);
				return 254;
			}
			size = st.st_size;
			if (size == 0) {
				fprintf(stderr, "Zero file size.\n");
				return 254;
			}
			while (size & 3)
				size++;
			fd = open(filename, O_RDONLY);
			if (fd < 0) {
				perror("Can not open file");
				return 254;
			}

			c = malloc(size);
			if (c == NULL) {
				perror("Can not allocate memory");
				return 254;
			} else {
				memset(c, 0, size);
				retval = read(fd, c, st.st_size);
				if (retval != st.st_size) {
					perror("Can not read data");
					close(fd);
				} else {
					close(fd);
					retval = ucwrite(addr, c, size, 0);
					if (retval) {
						fprintf(stderr, "Error writing to microcontroller.\n");
					}
				}
				FREE(c);
			}
			break;
		/***************************************************/
		case OP_ERASE:
			retval = ucerase(addr, size);
			if (retval) {
				fprintf(stderr, "Error erasing flash.\n");
			}
			break;
		/***************************************************/
		case OP_FLASH:
			if (filename == NULL) {
				fprintf(stderr, "File to program is not specified.\n");
				return 254;
			}
			retval = stat(filename, &st);
			if (retval < 0) {
				perror("Can not stat file");
				return 254;
			}
			if (!S_ISREG(st.st_mode)) {
				fprintf(stderr, "%s is not a regular file.\n", filename);
				return 254;
			}
			size = st.st_size;
			if (size == 0) {
				fprintf(stderr, "Zero file size.\n");
				return 254;
			}
			fd = open(filename, O_RDONLY);
			if (fd < 0) {
				perror("Can not open file");
				return 254;
			}

			msize = size;
			if (size & 3UL) {
				msize &= ~3UL;
				msize += 4;
			}
			c = malloc(msize);
			if (c == NULL) {
				perror("Can not allocate memory");
				return 254;
			} else {
				memset(c, 0xFF, size);
				retval = read(fd, c, size);
				if (retval != size) {
					perror("Can not read data");
					close(fd);
				} else {
					close(fd);
					retval = ucflash(addr, c, msize);
					if (retval) {
						fprintf(stderr, "Error programming.\n");
					}
				}
				FREE(c);
			}
			break;
	}

	if ((!retval) && (goflag)) {
		retval = ucgo(goaddr, gomode);
	}

	FREE(c);

	comm_clean();

	return retval;
}

void usage()
{
	/*          0         1         2         3         4         5         6         7       */
	/*      01234567890123456789012345678901234567890123456789012345678901234567890123456789  */
	printf("Usage: lpcflash [OPTIONS]\n");
	printf("OPTIONS:\n");
	printf("\n");

	printf("\t-i DEVICE\n\t--interface=DEVICE\n");
	printf("\t\tUse DEVICE to communicate with microcontroller (" DEFAULT_PORT ").\n");
	printf("\n");

	printf("\t-b BAUDRATE\n\t--baud=BAUDRATE\n");
	printf("\t\tCommunicate with microcontroller at the given BAUDRATE (57600).\n");
	printf("\n");

	printf("\t-f FREQUENCY\n\t--freq=FREQUENCY\n");
	printf("\t\tMicrocontroller running at FREQUENCY kHz (18432).\n");
	printf("\n");

	printf("\t-r ADDRESS:SIZE\n\t--read=ADDRESS:SIZE\n");
	printf("\t\tRead from memory SIZE bytes at the given ADDRESS.\n");
	printf("\t\tThe address must be word-aligned. The size must be multiple of 4.\n");
	printf("\t\tData stored to file specified with -l option.\n");
	printf("\n");

	printf("\t-w ADDRESS\n\t--write=ADDRESS\n");
	printf("\t\tWrite to RAM at the given ADDRESS.\n");
	printf("\t\tThe address must be word-aligned.\n");
	printf("\t\tData loaded from file specified with -l option.\n");
	printf("\n");

	printf("\t-e START:COUNT\n\t--erase=START:COUNT\n");
	printf("\t\tErase COUNT flash sectors begining from sector number START.\n");
	printf("\n");

	printf("\t-p ADDRESS\n\t--program=ADDRESS\n");
	printf("\t\tProgram flash begining at the given ADDRESS.\n");
	printf("\t\tThe address must be 512byte-aligned.\n");
	printf("\t\tData loaded from file specified with -l option.\n");
	printf("\t\tNOTE:\n");
	printf("\t\tThe boot signature at address 0x00000014 is calculated if and\n");
	printf("\t\tonly if ADDRESS = 0 and data size to be programmed is greater\n");
	printf("\t\tthen or equal 32 bytes.\n");
	printf("\t\tWhile programming less then 32 bytes at ADDRESS = 0 the\n");
	printf("\t\tresult is unpredictable.\n");
	printf("\t\tWARNING:\n");
	printf("\t\tThe read/modify/write is not supported!\n");
	printf("\n");

	printf("\t-g ADDRESS:MODE\n\t--go=ADDRESS:MODE\n");
	printf("\t\tRun user program at the given ADDRESS in specified MODE.\n");
	printf("\t\tMODE can be one of T or A character.\n");
	printf("\t\tT is for Thumb and A is for ARM mode execution.\n");
	printf("\t\tLowercase characters is allowed.\n");
	printf("\t\tThe address must be word-aligned.\n");
	printf("\t\tThe go comman is executed after successful execution of all\n");
	printf("\t\tother commands.\n");
	printf("\n");

	printf("\t-l FILENAME\n\t--file=FILENAME\n");
	printf("\t\tThe means of this option depends on operation.\n");
	printf("\t\tFor read operation data stored in the file with name FILENAME.\n");
	printf("\t\tFor write and program operations data loaded from file with\n");
	printf("\t\tname FILENAME.\n");
	printf("\n");

	printf("\t-v\n\t--verbose\n");
	printf("\t\tBe verbose. More options increase verbose level.\n");
	printf("\n");

	printf("\t-q\n\t--quiet\n");
	printf("\t\tBe quiet. No output to stdout.\n");
	printf("\n");

	printf("\t-V\n\t--version\n");
	printf("\t\tPrint version and exit.\n");
	printf("\n");

	printf("\t-h\n\t--help\n");
	printf("\t\tPrint this screan and exit.\n");
	printf("\n");

	ucprintdevs();
}

/* End of file */
