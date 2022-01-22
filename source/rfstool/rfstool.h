// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This file contains all definitions and declarations
//

#ifndef RFSTOOL_H
#define RFSTOOL_H

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), sscanf(), snprintf(), rename(), remove()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy(), memset()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round(), sqrt(), ceil()
#include <ctype.h>		// tolower()

////////// Zlib stuff //////////
#include "zlib.h"
#include <assert.h>

////////// Definitions //////////
#define PROG_TITLE "\nPS2 HL RAM FS tool v1.00\n"
#define PROG_INFO "\
Developed by supadupaplex, 2022\n\
License: BSD-3-Clause (check out license.txt)\n\
Zlib library is used to perform deflate operations\n\
\n\
This tool can extract RAM FS contents from PCSX2 save state or EE RAM dump,\n\
it is intended for direct save dictionary harvesting (*.hl1/2/3 files)\n\
for PAKS/DICTS.PAK.\n\
\n\
(!) Versions supported so far: SLUS_200.66 and SLES_505.04\n\
\n\
How to prepare PCSX2 save state for desired map:\n\
1) Start the game in PCSX2\n\
2) Go to a desired map\n\
3) Do a quick save in the pause menu (Pause menu -> Quick Save)\n\
4) Perform a save state in the PCSX2 (System -> Save state -> Slot X)\n\
\n\
How to use:\n\
1) Windows explorer - drag and drop PCSX2 save state file on rfstool.exe\n\
2) Command line/Batch - rfstool [file_name]\n\
\n\
For more info check out readme.txt\n\
"

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////
#include "fops.h"

////////// Structures //////////
typedef struct
{
	uint flag1;		// if all 3 flags are set then the file data's ok to extract
	char name[256];	// file name
	uint flag2;
	uint flag3;
	uint pdata;		// pointer to the file data (in the EE RAM)
	uint sz[3];		// 1 and 2 seem to be the same, 0 - set only for compressed?
	uint idk[3];	// ???
} ps2_ramfs_entry_t;


#define PS2_EERAM_SZ	0x2000000 // 32 MiB
#define PS2_RAMFS_FILES	50

// these are individual for each version
#define SLUS20066_IDSTR_OFFSET	0x328250
#define SLUS20066_IDSTR			"BASLUS-20066"
#define SLUS20066_RAMFS_BASE	0x3a6580

#define SLES50504_IDSTR_OFFSET	0x327F50
#define SLES50504_IDSTR			"BESLES-50504"
#define SLES50504_RAMFS_BASE	0x3a6400

typedef struct {
	uchar *mem;
	ps2_ramfs_entry_t *pramfs;

	int open_from_file(const char * fname)
	{
		FILE * pf;
		uint ramfs_base;

		UTIL_MALLOC(uchar*, mem, PS2_EERAM_SZ, exit(EXIT_FAILURE));

		SafeFileOpen(&pf, fname, "rb");

		// Check size
		if (FileSize(&pf) != PS2_EERAM_SZ)
			return 1;

		FileReadBlock(&pf, mem, 0, PS2_EERAM_SZ);
		fclose(pf);

		// Check version
		ramfs_base = INT_MAX;

		if (!strncmp(SLUS20066_IDSTR, (char *)mem + SLUS20066_IDSTR_OFFSET,
			sizeof(SLUS20066_IDSTR)))
		{
			ramfs_base = *(uint *)(mem + SLUS20066_RAMFS_BASE);
		}
		else if (!strncmp(SLES50504_IDSTR, (char *)mem + SLES50504_IDSTR_OFFSET,
			sizeof(SLES50504_IDSTR)))
		{
			ramfs_base = *(uint *)(mem + SLES50504_RAMFS_BASE);
		}

		if (ramfs_base >= PS2_EERAM_SZ)
		{
			// No match
			close();
			return 1;
		}

		// OK
		pramfs = (ps2_ramfs_entry_t *)(mem + ramfs_base);
		return 0;
	}

	ps2_ramfs_entry_t *ramfs_file_get(uint fidx)
	{
		ps2_ramfs_entry_t *ret = NULL;

		if (!pramfs || fidx > PS2_RAMFS_FILES)
			return ret;

		ret = &pramfs[fidx];
		if (ret->flag1 && ret->flag2 && ret->flag3 && ret->sz[1] && ret->sz[2])
			return ret;

		return NULL;
	}

	void close()
	{
		free(mem);
		pramfs = NULL;
	}

} sPS2_EERAM;

#endif // RFSTOOL_H
