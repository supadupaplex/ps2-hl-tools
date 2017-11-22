/*
=====================================================================
Copyright (c) 2017, Alexey Leushin
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

#pragma once

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), snprintf()
#include <conio.h>		// _getch()
#include <direct.h>		// _mkdir()
#include <string.h>		// strcpy(), strcat(), strlen(), strncpy()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// floor(), round()
#include <ctype.h>		// tolower()
#include <sys\stat.h>	// stat()
#include <windows.h>	// CreateDitectoryA()
#include <intrin.h>		// Byte swap functions (swap endian for PNG)

////////// Definitions //////////
#define PROG_VERSION "1.26"

#define PSI_MIN_DIMENSION 16
#define EIGHT_BIT_PALETTE_ELEMENTS_COUNT 256

// Image tyes
#define UNKNOWN_IMG 0
#define RGB_PNG 2
#define INDEXED_PNG 3
#define RGBA_PNG 6
#define INDEXED_PSI 2
#define RGBA_PSI 5

////////// Zlib stuff //////////
#define ZLIB_WINAPI
#include "zlib.h"
#include <assert.h>

////////// Typedefs //////////
typedef unsigned short int ushort;
typedef unsigned long int ulong;
typedef unsigned int uint;
typedef unsigned char uchar;
#pragma pack(1)
struct sPNGData
{
	ulong DataSize;
	uchar * Data;
};

////////// Functions //////////

// File operations
ulong FileSize(FILE **ptrFile);																			// Get size of file
void FileReadBlock(FILE **ptrSrcFile, void * DstBuff, ulong Addr, ulong Size);							// Read block from file to buffer
void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Addr, ulong Size);							// Write data from buffer to file
void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Size);										// Write data from buffer to file
void SafeFileOpen(FILE **ptrFile, const char * FileName, char * Mode);									// Try to open file, if problem oocur then exit
void FileGetExtension(const char * Path, char * OutputBuffer, uint OutputBufferSize);					// Get file extension
void FileGetName(const char * Path, char * OutputBuffer, uint OutputBufferSize, bool WithExtension);	// Get name of file with or without extension
void FileGetFullName(const char * Path, char * OutputBuffer, uint OutputBufferSize);					// Get full file name (with folders) without extension
void FileGetPath(const char * Path, char * OutputBuffer, uint OutputBufferSize);						// Get file path
bool CheckFile(char * FileName);																		// Check existance of file
void GenerateFolders(char * cPath);																		// Make sure, that all folders in path are existing
void PatchSlashes(char * cPathBuff, ulong BuffSize, bool SlashToBackslash);								// Patch slashes when transitioning between PAK and Windows file names
bool CheckDir(const char * Path);																		// Check if path is directory
void NewDir(const char * DirName);

// PNG Functions
sPNGData * PNGReadChunk(FILE ** ptrFile, const char * Marker);												// Read data from all PNG chunks with specified marker
void PNGWriteChunk(FILE ** ptrFile, const char * Marker, sPNGData * Chunk);									// Write chunk to PNG
void PNGWriteChunk(FILE ** ptrFile, const char * Marker, void * Data, ulong DataSize);						// Write chunk to PNG
bool PNGDecompress(sPNGData * InData);																		// Decompress bitmap
bool PNGCompress(sPNGData * InData);																		// Compress bitmap
uchar PNGGetByteFromRow(uchar * Row, uint PixelNumber, uint BitDepth);										// Get pixel byte from row
bool PNGUnfilter(sPNGData * InData, uint Height, uint Width, uint BytesPerPixel, uint BitDepth);			// Revert filtering from bitmap
bool PNGFilter(sPNGData * InData, uint Height, uint Width, uint BytesPerPixel, uchar FilterType);			// Apply filter to bitmap
int PaethPredictor(int a, int b, int c);																	// Paeth predictor function
sPNGData * PNGReadPalette(FILE ** ptrFile);																	// Read palette from PNG file
sPNGData * PNGReadBitmap(FILE ** ptrFile, uint Width, uint Height, uchar BytesPerPixel, uint BitDepth);		// Read raw bitmap from PNG file
void PNGWritePalette(FILE ** ptrFile, sPNGData * RGBAPalette);												// Write palette to PNG file
void PNGWriteBitmap(FILE ** ptrFile, uint Width, uint Height, uchar BytesPerPixel, sPNGData * RGBABitmap);	// Write bitmap to PNG file
bool ZDecompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize, ulong StartSize);	// Compress data with Zlib
bool ZCompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize);					// Decompress data with Zlib

////////// Structures //////////

// *.png image header
#pragma pack(1)
struct sPNGHeader
{
	ulong Signature1;			// [0x89504E47]
	ulong Signature2;			// [0x0D0A1A0A]
	ulong IHDTSize;				// 13 [0x0D]
	ulong IHDT;					// "IHDT" [0x49484452]

	ulong Width;				// Image width (in pixels)
	ulong Height;				// Image height (in pixels)
	
	uchar BitDepth;				// = 8 (From Wiki: The permitted formats encode each number as an unsigned integral value using a fixed number of bits, referred to in the PNG specification as the bit depth.)
	uchar ColorType;			// 2 - TrueColor (RGB), 3 - Indexed, 6 - TrueColor (RGBA)
	uchar Compression;			// = 0 (No compression)
	uchar Filter;				// = 0 (Per-row filtering)
	uchar Interlacing;			// = 0 (No interlacing)
	ulong CRC32;				// Checksum of header

	void SwapEndian()			// Swap endian after reading or before writing to file
	{
		this->Signature1 = _byteswap_ulong(this->Signature1);
		this->Signature2 = _byteswap_ulong(this->Signature2);
		this->IHDTSize = _byteswap_ulong(this->IHDTSize);
		this->IHDT = _byteswap_ulong(this->IHDT);
		this->Width = _byteswap_ulong(this->Width);
		this->Height = _byteswap_ulong(this->Height);
		this->CRC32 = _byteswap_ulong(this->CRC32);
	}

	void UpdateFromFile(FILE ** ptrFile)
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sPNGHeader));
	}

	void Update(ulong NewWidth, ulong NewHeight, ulong NewColorType)
	{
		ulong CRC;

		this->Signature1 = 0x89504E47;		// 
		this->Signature2 = 0x0D0A1A0A;		// 
		this->IHDTSize = 13;				// always = 13
		this->IHDT = 0x49484452;			// "IHDT"

		this->Width = NewWidth;				// Image width (in pixels)
		this->Height = NewHeight;			// Image height (in pixels)

		this->BitDepth = 8;					// 8 is common value
		this->ColorType = NewColorType;		// Supported types: 2 - TrueColor (RGB), 3 - Indexed, 6 - TrueColor (RGBA)
		this->Compression = 0;				// No compression
		this->Filter = 0;					// Per-row filtering
		this->Interlacing = 0;				// No interlacing

		// Calculate CRC
		this->SwapEndian();
		CRC = crc32(NULL, (const Bytef *) &this->IHDT, sizeof(ulong) * 3 + sizeof(uchar) * 5);
		this->SwapEndian();
		this->CRC32 = CRC;
	}

	uchar CheckType()
	{
		if (this->Signature1 != 0x89504E47 || this->Signature2 != 0x0D0A1A0A || this->IHDTSize != 13 || this->IHDT != 0x49484452
			|| this->BitDepth > 8 || this->Compression != 0 || this->Filter != 0 || this->Interlacing != 0)
			return UNKNOWN_IMG;

		return this->ColorType;
	}
};

// PS2 HL Decal header
#pragma pack(1)
struct sPHDHeader
{
	char Signature[64];		// Filled with zeroes

	void Update()
	{
		memset(this->Signature, 0x00, sizeof(sPHDHeader));
	}

	void UpdateFromFile(FILE ** ptrFile)
	{
		// Copy header from file to this structure
		FileReadBlock(ptrFile, this, 0, sizeof(sPHDHeader));
	}

	bool Check()
	{
		for (ulong i = 0; i < sizeof(sPHDHeader); i++)
		{
			if (this->Signature[i] != 0x00)
				return false;
		}

		return true;
	}
};

// *.psi image header
#pragma pack(1)
struct sPSIHeader
{
	char Name[16];		// Internal name
	uchar Magic[3];		// Filled with zeroes in most cases
	uchar LODCount;		// Number of LODs that present in image file (used in decals only)
	ulong Type;			// 2 - 8 bit indexed bitmap, 5 - 32 bit RGBA bitmap
	ushort Width;		// Texture width (in pixels)
	ushort Height;		// Textre Height (in pixels)
	ushort UpWidth;		// Upscale target: width (in pixels)
	ushort UpHeight;	// Upscale target: height (in pixels)

	void Update(const char * NewName, ushort NewWidth, ushort NewHeight, ulong NewType, uchar NewLODCount)
	{
		// Clear this structure from garbage and leftovers
		memset(this, 0x00, sizeof(sPSIHeader));

		// Update fields
		snprintf(this->Name, sizeof(Name), "%s", NewName);
		this->LODCount = NewLODCount;
		this->Type = NewType;
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->UpWidth = NewWidth;
		this->UpHeight = NewHeight;
	}

	void UpdateUpscaleTaget(ushort NewUpWidth, ushort NewUpHeight)
	{
		this->UpWidth = NewUpWidth;
		this->UpHeight = NewUpHeight;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong Address)
	{
		// Copy texture header directly to this structure
		FileReadBlock(ptrFile, this, Address, sizeof(sPSIHeader));
	}

	uchar CheckType()
	{
		if (this->Type == INDEXED_PSI || this->Type == RGBA_PSI)
			return this->Type;
		else
			return UNKNOWN_IMG;
	}
};

// 8-bit *.bmp header
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sBMPHeader
{
	char Signature1[2];		// "BM" Signature
	ulong FileSize;			// Total file size (in bytes)
	ulong Signature2;		// = 0
	ulong Offset;			// Offset
	ulong StructSize;		// = 0x28 for BMP version 3
	ulong Width;			// Picture Width (in pixels)
	ulong Height;			// Picture Height (in pixels)
	ushort Signature3;		// = 1 for BMP version 3
	ushort BitsPerPixel;	// How many bits per 1 pixel
	ulong Compression;		// Compression type
	ulong PixelDataSize;	// Size of bitmap
	ulong HorizontalPPM;	// Horizontal pixels per meter value
	ulong VerticalPPM;		// Vertical pixels per meter value
	ulong ColorTabSize;		// How many colors are present in color table
	ulong ColorTabAlloc;	// How many colors are actually used in color table

	void Update(unsigned long int Width, unsigned long int Height)		// Update all fields of BMP header
	{
		this->Signature1[0] = 'B';
		this->Signature1[1] = 'M';
		this->Signature2 = 0x00000000;
		this->Offset = 0x00000436;
		this->StructSize = 0x00000028;
		this->Signature3 = 0x0001;
		this->BitsPerPixel = 0x0008;
		this->Compression = 0x00000000;
		this->HorizontalPPM = 0x00000000;		// = 0 (not used)
		this->VerticalPPM = 0x00000000;			// = 0 (not used)
		this->ColorTabSize = 0x00000100;
		this->ColorTabAlloc = 0x00000100;
		this->Width = Width;
		this->Height = Height;
		this->PixelDataSize = Height * Width;
		this->FileSize = this->PixelDataSize + this->Offset;
	}

	void UpdateFromFile(FILE ** ptrFile)
	{
		// Copy header from file to this structure
		FileReadBlock(ptrFile, this, 0, sizeof(sBMPHeader));
	}

	bool Check()
	{
		if (this->Signature1[0] == 'B' && this->Signature1[1] == 'M' && this->Signature2 == 0 && this->Signature3 == 1 && this->BitsPerPixel == 8 && this->Compression == 0)
			return true;
		else
			return false;
	}
};