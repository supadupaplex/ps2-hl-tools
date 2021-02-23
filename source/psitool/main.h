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
#define PROG_TITLE "\nPS2 HL image tool v1.2\n\n"
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
	uchar LODCount;		// Number of MIPs (used in decals only)
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

// *.psf header
#define PSF_SYMBOLS 256
#define PSF_BMP_W 256
#define PSF_BMP_H 256
#define PSF_BMP_SZ (PSF_BMP_W * PSF_BMP_H)
#define PSF_PLTE_SZ (256 * 4)
#define PSF_SZ (sizeof(sPSFHeader) + PSF_BMP_SZ + PSF_PLTE_SZ)
#pragma pack(1)
struct sPSFSymEntry
{
	uchar PosX; // Position in the bitmap: X ...
	uchar PosY; // ... and Y
	uchar Width; // Width (height is always 25)
	uchar Null; // Always 0x00
};
#pragma pack(1)
struct sPSFHeader
{
	uint idk[4];
	sPSFSymEntry Symbols[PSF_SYMBOLS];

	bool CheckType()
	{
		if (idk[0] == 0x19 &&
			idk[1] == 0x100 &&
			idk[2] == 0x8 &&
			idk[3] == 0x19)
			return true;

		return false;
	}

	bool UpdateFromFile(FILE ** ptrFile)
	{
		if (FileSize(ptrFile) == PSF_SZ)
		{
			FileReadBlock(ptrFile, this, 0, sizeof(sPSFHeader));
			return CheckType();
		}

		return false;
	}

	void ReadSymbols(FILE ** ptrFile)
	{
		char Buf[128];
		int Line;
		unsigned int s, x, y, w;

		// (Re)init self
		Init();

		// Read symbols
		Line = 0;
		while (!feof(*ptrFile))
		{
			// Get line
			fgets(Buf, sizeof(Buf), *ptrFile);

			// Skip empty lines
			if (Buf[0] == '\0' || Buf[0] == '#')
				continue;

			// Read symbol
			s = x = y = w = 256;
			sscanf(Buf, "%u %u %u %u ", &s, &x, &y, &w);
			UTIL_MSG_DEBUG("Read symbol: %u %u %u %u\n", s, x, y, w);

			// Check
			if (s > 255 || x > 255 || y > 255 || w > 255)
			{
				UTIL_MSG_ERR("Bad line %d: \"%s\"\n", Line, Buf);
				continue;
			}

			// Add symbol
			Symbols[s].PosX = x;
			Symbols[s].PosY = y;
			Symbols[s].Width = w;

			Line++;
		}
	}

	void WriteSymbols(FILE ** ptrFile)
	{
		int s;

		fputs("# This file contains symbol mappings\n", *ptrFile);
		fputs("# Format: A B C D, where:\n", *ptrFile);
		fputs("# * A: 0-255, decimal symbol code (32-126 ASCII)\n", *ptrFile);
		fputs("# * B: 0-255, symbol X coordinate in bitmap\n", *ptrFile);
		fputs("# * C: 0-255, symbol Y coordinate in bitmap\n", *ptrFile);
		fputs("# * D: 0-255, symbol width (height is always 25)\n", *ptrFile);
		fputs("# ! Warning: bitmap must have 256x256 dimensions\n", *ptrFile);
		fputs("#\n# C\tX\tY\tWidth\n", *ptrFile);

		for (s = 0; s < PSF_SYMBOLS; s++)
		{
			// Skip empty
			if (Symbols[s].PosX == 0 &&
				Symbols[s].PosY == 0 &&
				Symbols[s].Width == 0 &&
				Symbols[s].Null == 0)
				continue;

			// Write symbol code, position and width
			fprintf(*ptrFile, "%d\t%d\t%d\t%d",
				s, Symbols[s].PosX, Symbols[s].PosY, Symbols[s].Width);

			// Mark ASCII symbols
			if (32 <= s && s <= 126)
				fprintf(*ptrFile, "\t# \'%c\'", s);

			// Newline
			fputc('\n', *ptrFile);
		}
	}

	void Init()
	{
		idk[0] = idk[3] = 0x19;
		idk[1] = 0x100;
		idk[2] = 0x8;
		memset(Symbols, 0x00, sizeof(Symbols));
	}
};

#endif // MAIN_H
