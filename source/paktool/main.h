/*
=====================================================================
Copyright (c) 2017-2020, Alexey Leushin
All rights reserved.

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of the copyright holders nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
=====================================================================
*/

//
// This file contains all definitions and declarations
//

#ifndef MAIN_H
#define MAIN_H

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), gets_s(), sscanf(), snprintf(), rename(), remove()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy(), memset()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round(), sqrt(), ceil()
#include <ctype.h>		// tolower()

////////// Zlib stuff //////////
#include "zlib.h"
#include <assert.h>

////////// Definitions //////////
#define PROG_VERSION "1.32"
#define PAK_NORMAL 0
#define PAK_COMPRESSED 1
#define PAK_UNKNOWN -1
#define PS2HL_NPAK_SEG_SIZE 0x800		// PS2 HL normal PAK segment size (PS2 HL likes everything to be alligned)
#define PS2HL_CPAK_SEG_SIZE 0x10		// PS2 HL compressed PAK segment size (PS2 HL likes everything to be alligned)
#define GLOBAL_PAK_RAM_OFFSET 0x1F7DFC0	// Base address of GLOBAL.PAK inside PS2's RAM
#define GLOBAL_PAK_RAM_ALIGN 0x40		// Alignment of GLOBAL.PAK in PS2's RAM
#define SPZ_BASE_FRAMEID 5				// Initial ID of SPZ frames in GLOBAL.PAK

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////
#include "fops.h"

////////// Structures //////////

// PS2 PAK header
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2NormalPAKHeader
{
	char Signature[4];		// "PACK" - signature
	ulong TableOffset;		// Offset of file table (in bytes)
	ulong TableSize;		// Size of file table (in bytes)
};
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2CompressedPAKHeader
{
	ulong PAKSize;			// Size of uncompressed PAK file
	uchar Signature[2];		// '78 DA' - ZLIB signature
};
#pragma pack(1)				// Fix unwanted 0x00 bytes in union
union uPS2PAKHeader
{
	struct sPS2NormalPAKHeader Normal;
	struct sPS2CompressedPAKHeader Compressed;

	void UpdateNormal(int NewTableOffset, int NewTableSize)
	{
		this->Normal.Signature[0]='P';
		this->Normal.Signature[1]='A';
		this->Normal.Signature[2]='C';
		this->Normal.Signature[3]='K';
		this->Normal.TableOffset = NewTableOffset;
		this->Normal.TableSize = NewTableSize;
	}

	void UpdateCompressed(int NewPAKSize)
	{
		this->Compressed.PAKSize = NewPAKSize;
		this->Compressed.Signature[0] = 0x78;
		this->Compressed.Signature[1] = 0xDA;
	}

	char CheckType()	// Returns: 0 if normal PAK, 1 if compressed PAK, -1 if unknown
	{
		if ((this->Normal.Signature[0] == 'P') && (this->Normal.Signature[1] == 'A') && (this->Normal.Signature[2] == 'C') && (this->Normal.Signature[3] == 'K'))
		{
			return PAK_NORMAL;
		}
		else
		{
			if ((this->Compressed.Signature[0] == 0x78) && (this->Compressed.Signature[1] == 0xDA))
				return PAK_COMPRESSED;
			else
				return PAK_UNKNOWN;
		}
	}

	void UpdateFromFile(FILE **ptrFile)
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sPS2NormalPAKHeader));
	}
};

// PS2 PAK file table entry
#pragma pack(1)					// Fix unwanted 0x00 bytes in structure
struct sPS2PAKFileEntry
{
	char FileName[56];			// File name
	ulong FileOffset;			// File offset (in bytes)
	ulong FileSize;				// File size (in bytes)

	void Update(const char * NewFileName, ulong NewFileOffset, ulong NewFileSize)
	{
		memset(this, 0x00, sizeof(sPS2PAKFileEntry));			// Fill file entry with 0xCD bytes (like in PS2 HL PAK files)

		strcpy(this->FileName, NewFileName);
		this->FileOffset = NewFileOffset;
		this->FileSize = NewFileSize;
	}

	void UpdateFromFile(FILE **ptrFile, ulong Offset)
	{
		FileReadBlock(ptrFile, this, Offset, sizeof(sPS2PAKFileEntry));
	}
};

// File list entry (contains information about all files that would be packed)
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sFileListEntry
{
	char  FileName[PATH_LEN];	// File name
	ulong FileSize;				// File size

	void Update(char * NewFileName, ulong NewFileSize)
	{
		strcpy(this->FileName, NewFileName);
		this->FileSize = NewFileSize;
	}
};

// *.spz file header
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sSPZHeader
{
	char Signature[4];			// "SPAZ" signature
	uchar Type;					// 0x00 - VP_PARALLEL, (0x01 - ?), 0x02 - ORIENTED, 0x03 - VP_PARALLEL_ORIENTED
	uchar Magic;				// = 0
	uchar RAMFlag;				// It is set to 0, except GRESTORE.PAK (I think it is flag that indicates that offsets are pointing to RAM)
	uchar FrameCount;			// How many frames in sprite

	void Update(ulong NewFrameCount, uchar NewSPZType, uchar NewRAMFlag)
	{
		this->Signature[0] = 'S';
		this->Signature[1] = 'P';
		this->Signature[2] = 'A';
		this->Signature[3] = 'Z';
		this->Type = NewSPZType;
		this->Magic = 0;
		this->RAMFlag = NewRAMFlag;
		this->FrameCount = NewFrameCount;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong Address)
	{
		// Copy file header directly to this structure
		FileReadBlock(ptrFile, this, Address, sizeof(sSPZHeader));
	}

	bool CheckSignature()
	{
		if (this->Signature[0] == 'S' && this->Signature[1] == 'P' && this->Signature[2] == 'A' && this->Signature[3] == 'Z' && this->Magic == 0 && this->RAMFlag == 0)
			return true;
		else
			return false;
	}
};

// *.spz texture table entry
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sSPZFrameTableEntry
{
	ulong FrameID;			// Frame ID (unique for each frame in all sprites). Used inly in GRESTORE.PAK, normally it equals zero
	ulong FrameOffset;		// Offset

	void Update(ulong NewFrameID, ulong NewFrameOffset)
	{
		this->FrameID = NewFrameID;
		this->FrameOffset = NewFrameOffset;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong Address)
	{
		// Copy texture table entry directly to this structure
		FileReadBlock(ptrFile, this, Address, sizeof(sSPZFrameTableEntry));
	}
};

#endif // MAIN_H
