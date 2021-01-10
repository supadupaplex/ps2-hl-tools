// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This file contains all definitions and declarations
//

#ifndef MAIN_H
#define MAIN_H

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), snprintf()
#include <string.h>		// strcpy(), strcat(), strlen(), strncpy()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// floor(), ceil()
#include <ctype.h>		// tolower()

////////// Definitions //////////
#define PROG_TITLE "\nPS2 HL image tool v1.11\n"
#define PROG_INFO "\
Developed by supadupaplex, 2017-2021\n\
License: BSD-3-Clause (check out license.txt)\n\
Zlib library is used to perform deflate/inflate operations\n\
\n\
How to use:\n\
1) Windows explorer - drag and drop image file on psitool.exe\n\
2) Command line/Batch - psitool [image_file_name]\n\
\n\
For more info check out readme.txt \n\
"

// Image types
#define PSI_UNKNOWN 0
#define PSI_INDEXED 2
#define PSI_RGBA 5

////////// Zlib stuff //////////
#include "zlib.h"
#include <assert.h>

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////

// File operations
#include "fops.h"

// PNG Functions
#include "pngtool.h"

////////// Structures //////////

// *.psi image header
#pragma pack(1)
struct sPSIHeader
{
	char Name[16];		// Internal name
	uchar Magic[3];		// Filled with zeroes in most cases
	uchar LODCount;		// Number of LODs that present in image file (used in decals only)
	ulong Type;			// 2 - 8 bit indexed bitmap, 5 - 32 bit RGBA bitmap
	ushort Width1;		// Texture width (in pixels)
	ushort Height1;		// Textre Height (in pixels)
	ushort Width2;		// Usually same as "Width1", but not always (used for in-engine upscale?)
	ushort Height2;		// Usually same as "Height2", but not always (used for in-engine upscale?)

	void Update(const char * NewName, ushort NewWidth, ushort NewHeight, ulong NewType)
	{
		// Clear this structure from garbage and leftovers
		memset(this, 0x00, sizeof(sPSIHeader));

		// Update fields
		snprintf(this->Name, sizeof(Name), "%s", NewName);
		this->LODCount = 0;
		this->Type = NewType;
		this->Width1 = NewWidth;
		this->Height1 = NewHeight;
		this->Width2 = NewWidth;
		this->Height2 = NewHeight;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong Address)
	{
		// Copy texture header directly to this structure
		FileReadBlock(ptrFile, this, Address, sizeof(sPSIHeader));
	}

	uchar CheckType()
	{
		if (this->Type == PSI_INDEXED || this->Type == PSI_RGBA)
			return this->Type;
		else
			return PSI_UNKNOWN;
	}
};

#endif // MAIN_H
