#pragma once

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

////////// Includes //////////
#include <stdio.h>	// puts(), printf(), sscanf(), snprintf()
#include <conio.h>	// _getch()
#include <direct.h>	// _mkdir()
#include <string.h>	// strcpy(), strcat(), strlen(), strtok(), strncpy()
#include <malloc.h> // malloc(), free()
#include <stdlib.h> // exit()
#include <math.h>	// round(), sqrt()
#include <ctype.h>	// tolower()

////////// Definitions //////////
#define PROG_VERSION "1.3"
#define EIGHT_BIT_PALETTE_ELEMENTS_COUNT 256
#define SPZ_PALETTE_ELEMENT_SIZE 4
#define SPR_PALETTE_ELEMENT_SIZE 3
#define PSI_MIN_DIMENSION 16

////////// Typedefs //////////
#include "types.h"

////////// Enumerators //////////
enum eSPRType { SPR_VP_PARALLEL_UPRIGHT = 0, SPR_FACING_UPRIGHT = 1, SPR_VP_PARALLEL = 2, SPR_ORIENTED = 3, SPR_VP_PARALLEL_ORIENTED = 4 };		// Possible *.spr types
enum eSPRFormat { SPR_NORMAL = 0, SPR_ADDITIVE = 1, SPR_INDEXALPHA = 2, SPR_ALPHATEST = 3 };													// Possible *.spr formats
enum eSPZType { SPZ_VP_PARALLEL = 0, SPZ_VP_PARALLEL_UPRIGHT = 1, SPZ_ORIENTED = 2, SPZ_VP_PARALLEL_ORIENTED = 3 };								// Possible *.spz types
enum eSPZFormat { SPZ_ADDITIVE = 0, SPZ_INDEXALPHA = 1, SPZ_ALPHATEST = 2 };																	// Possible *.spz formats

////////// Functions //////////
#include "fops.h"

////////// Structures //////////

// *.spr file header
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sSPRHeader
{
	char Signature[4];			// "IDSP" signature
	ulong Version;				// Value: 2 - Half-life strite
	ulong Type;					// Values: 0 - VP_PARALLEL_UPRIGHT, 1 - FACING_UPRIGHT, 2 - VP_PARALLEL, 3 - ORIENTED, 4 - VP_PARALLEL_ORIENTED
	ulong Format;				// Valurs: 0 - SPR_NORMAL, 1 - SPR_ADDITIVE, 2 - SPR_INDEXALPHA, 3 - SPR_ALPHTEST
	float BoundingRadius;		// Size of line, drawn from center of the sprite to corner
	ulong MaxWidth;				// Sprite Width (in pixels)
	ulong MaxHeight;			// Sprite Height (in pixels)
	ulong FrameCount;			// How many frames in srite
	float BeamLength;			// [Normal value: 0]
	ulong SyncType;				// Values: 0 - synchronized, [1 - random]
	ushort PaletteSize;			// Number of palette entries. Should be 256 for normal 8-bit Half-Life sprites

	void Update(ulong NewWidth, ulong NewHeight, ulong NewFrameCount, eSPRType NewSPRType, eSPRFormat NewSPRFormat)
	{
		this->Signature[0] = 'I';
		this->Signature[1] = 'D';
		this->Signature[2] = 'S';
		this->Signature[3] = 'P';
		this->Version = 2;
		this->Type = NewSPRType;
		this->Format = NewSPRFormat;
		this->BoundingRadius = sqrtf(((float) NewWidth / 2) * ((float) NewWidth / 2) + ((float) NewHeight / 2) * ((float) NewHeight / 2));
		this->MaxWidth = NewWidth;
		this->MaxHeight = NewHeight;
		this->FrameCount = NewFrameCount;
		this->BeamLength = 0;
		this->SyncType = 1;
		this->PaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT;
	}

	void UpdateFromFile(FILE ** ptrFile)
	{
		// Copy file header directly to this structure
		FileReadBlock(ptrFile, this, 0, sizeof(sSPRHeader));
	}

	bool CheckSignature()
	{
		if (this->Signature[0] == 'I' && this->Signature[1] == 'D' && this->Signature[2] == 'S' && this->Signature[3] == 'P' && this->Version == 2)
			return true;
		else
			return false;
	}
};

// *.spr texture header
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sSPRFrameHeader
{
	ulong Group;			// Group. [Normal velue - 0]
	long OriginX;			// Frame orogin X = (-1) * Width / 2
	long OriginY;			// Frame origin Y = Height / 2
	ulong Width;			// Frame width. Usually same as in other frames and header.
	ulong Height;			// Frame height.  Usually same as in other frames and header.

	void Update(ulong NewWidth, ulong NewHeight)
	{
		this->Group = 0;
		this->OriginX = (-1) * ((long) NewWidth) / 2;
		this->OriginY = NewHeight / 2;
		this->Width = NewWidth;
		this->Height = NewHeight;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong Offset)
	{
		// Copy texture header directly to this structure
		FileReadBlock(ptrFile, this, Offset, sizeof(sSPRFrameHeader));
	}
};

// *.spz file header
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sSPZHeader
{
	char Signature[4];			// "SPAZ" signature
	uchar Type;					// 0x00 - VP_PARALLEL, 0x01 - VP_PARALLEL_UPRIGHT, 0x02 - ORIENTED, 0x03 - VP_PARALLEL_ORIENTED
	uchar Magic;				// = 0
	uchar RAMFlag;				// It is set to 0, except GRESTORE.PAK (I think it is flag that indicates that offsets are pointing to RAM)
	uchar FrameCount;			// How many frames in sprite

	void Update(ulong NewFrameCount, eSPZType NewSPZType)
	{
		this->Signature[0] = 'S';
		this->Signature[1] = 'P';
		this->Signature[2] = 'A';
		this->Signature[3] = 'Z';
		this->Type = NewSPZType;
		this->Magic = 0;
		this->RAMFlag = 0;
		this->FrameCount = NewFrameCount;
	}

	void UpdateFromFile(FILE ** ptrFile)
	{
		// Copy file header directly to this structure
		FileReadBlock(ptrFile, this, 0, sizeof(sSPZHeader));
	}

	bool CheckSignature()
	{
		if (this->Signature[0] == 'S' && this->Signature[1] == 'P' && this->Signature[2] == 'A' && this->Signature[3] == 'Z' && this->Magic == 0 && this->RAMFlag == 0)
			return true;
		else
			return false;
	}
};

// *.spz frame table entry
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sSPZFrameTableEntry
{
	ulong FrameID;			// Frame ID (unique for each frame in all sprites). Used inly in GRESTORE.PAK, normally it equals zero
	ulong FrameOffset;		// Location of frame in file

	void Update(ulong NewFrameOffset)
	{
		this->FrameID = 0;
		this->FrameOffset = NewFrameOffset;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong Address)
	{
		// Copy texture table entry directly to this structure
		FileReadBlock(ptrFile, this, Address, sizeof(sSPZFrameTableEntry));
	}
};

// *.spz frame (psi) header
struct sSPZFrameHeader
{
	char Name[16];		// Internal image name
	ulong LODCount;		// How many LODs. Used in decals only
	ulong Type;			// 2 - 8 bit palettized image, 5 - 32 bit RGBA image
	ushort Width;		// Texture width (in pixels)
	ushort Height;		// Texture height (in pixels)
	ushort UpWidth;		// Upscale: target width (in pixels)
	ushort UpHeight;	// Upscale: target height (in pixels)

	void Update(const char * NewName, ushort NewWidth, ushort NewHeight)
	{
		// Clear Name from garbage and leftovers
		for (uint i = 0; i < sizeof(Name); i++)
			this->Name[i] = 0x00;

		// Update fields
		snprintf(this->Name, sizeof(Name), "%s", NewName);
		this->LODCount = 0;
		this->Type = 2;
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->UpWidth = NewWidth;
		this->UpHeight = NewHeight;
	}

	void UpdateUpscaleTarget(ulong NewUpWidth, ulong NewUpHeight)
	{
		this->UpWidth = NewUpWidth;
		this->UpHeight = NewUpHeight;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong Address)
	{
		// Copy texture header directly to this structure
		FileReadBlock(ptrFile, this, Address, sizeof(sSPZFrameHeader));
	}
};

// Texture table entry
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sTexture
{
	char Name[64];				// Texture name
	ulong Width;				// Texture width (in pixels)
	ulong Height;				// Texture height (in pixels)
	uchar * Palette;			// Texture palette
	ulong PaletteSize;			// Texture palette size
	uchar * Bitmap;				// Pointer to bitmap
	ulong BitmapSize;			// Size of bitmap = Width * Height

	void Initialize()			// Initialize structure
	{
		strcpy(this->Name, "New_Texture");
		this->Width = 0;
		this->Height = 0;
		this->Palette = NULL;
		this->PaletteSize = 0;
		this->Bitmap = NULL;
		this->BitmapSize = 0;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong FileBitmapOffset, ulong FileBitmapSize, ulong FilePaletteOffset, ulong FilePaletteSize, const char * NewName, ulong NewWidth, ulong NewHeight)	// Update from file
	{
		// Destroy old palette and bitmap
		free(Palette);
		free(Bitmap);

		// Allocate memory for new ones
		Palette = (uchar *)malloc(FilePaletteSize);
		Bitmap = (uchar *)malloc(FileBitmapSize);
		if (Palette == NULL || Bitmap == NULL)
		{
			puts("Unable to allocate memory ...");
			_getch();
			exit(EXIT_FAILURE);
		}

		// Copy data from file to memory
		FileReadBlock(ptrFile, (char *)Palette, FilePaletteOffset, FilePaletteSize);
		FileReadBlock(ptrFile, (char *)Bitmap, FileBitmapOffset, FileBitmapSize);

		// Update other fields
		strcpy_s(this->Name, sizeof(Name), NewName);
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->PaletteSize = FilePaletteSize;
		this->BitmapSize = this->Width * this->Height;
	}

	/*void PatchBitmap()	// Patch bitmap byte data. Needed for BMP\SPR to SPZ conversion and vice versa.
	{
		uchar Remainder;

		for (ulong i = 0; i < (this->Width * this->Height); i++)
		{
			Remainder = this->Bitmap[i] % 0x20;

			if (((0x00 <= Remainder) && (Remainder <= 0x07)) || ((0x18 <= Remainder) && (Remainder <= 0x1F)))
			{
				this->Bitmap[i] = this->Bitmap[i];
			}
			else
			{
				if ((0x07 < Remainder) && (Remainder < 0x10))
					this->Bitmap[i] = this->Bitmap[i] + 0x08;
				else
					this->Bitmap[i] = this->Bitmap[i] - 0x08;
			}
		}
	}*/
	
	void PaletteReformat(uint PaletteElementSize)			// Peposition palette table elements. Used for SPZ to SPR and SPR to SPZ conversions.
	{
		uchar Remainder;
		uchar Temp;

		for (ulong i = 0; i < this->PaletteSize; i++)
		{
			Remainder = i % (0x20 * PaletteElementSize);

			if (((0x10 * PaletteElementSize) <= Remainder) && (Remainder < (0x18 * PaletteElementSize)))
			{
				Temp = this->Palette[i];
				this->Palette[i] = this->Palette[i - (0x08 * PaletteElementSize)];
				this->Palette[i - (0x08 * PaletteElementSize)] = Temp;
			}
		}
	}

	void PaletteMulDiv(bool Multiply)						// Multiply (true) or divide (false) color table entries by 2. Multiply - convert *.SPZ to *.SPR, divide - convert *.SPR to *.SPZ 
	{														// !!! Apply this function to palete with removed alpha !!!
		for (ulong i = 0; i < this->PaletteSize; i++)
		{
			if (Multiply == true)
				this->Palette[i] = this->Palette[i] * 2;
			else
				this->Palette[i] = this->Palette[i] / 2;
		}
	}

	void PaletteRemoveAlpha()	// Convert palette to SPR format
	{
		uchar * NewPalette;
		ulong NewPaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPR_PALETTE_ELEMENT_SIZE;

		if (this->PaletteSize == EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPZ_PALETTE_ELEMENT_SIZE)
		{
			// Allocate memory for new palette
			NewPalette = (uchar *)malloc(NewPaletteSize);
			if (NewPalette == NULL)
			{
				puts("Unable to allocate memory!");
				_getch();
				exit(EXIT_FAILURE);
			}

			// Copy palette without alpha to new place
			ulong ByteCounterOld = 0;
			ulong ByteCounterNew = 0;
			for (ByteCounterOld = 0; ByteCounterOld < this->PaletteSize; ByteCounterOld++)	// Copy every first 3 out of 4 bytes from old palette to new one
			{
				if ((ByteCounterOld + 1) % 4 != 0)
				{
					NewPalette[ByteCounterNew] = this->Palette[ByteCounterOld];
					ByteCounterNew++;
				}
			}

			// Destroy old palette
			free(this->Palette);

			// Save pointer to new palette
			this->Palette = NewPalette;

			// Update size
			this->PaletteSize = NewPaletteSize;
		}
	}

	eSPZFormat PaletteCheckSPZFormat()						// Analyze SPZ palette and return it's tranparency format
	{														// !!! Use this function before removal of alpha !!!
		ulong Counter = 0;		// Byte counter

		// Count palette alpha bytes that have value lower than 0x80
		for (ulong i = 0; i < this->PaletteSize; i++)
			if ((i + 1) % SPZ_PALETTE_ELEMENT_SIZE == 0)
				if (this->Palette[i] < 0x80)
					Counter++;

		// Return type
		if (Counter == 0)
		{
			return SPZ_ADDITIVE;
		}
		else if (Counter == 1)
		{
			return SPZ_ALPHATEST;
		}
		else
		{
			return SPZ_INDEXALPHA;
		}
	}

	void PaletteAddSPZAlpha(eSPZFormat SPZFormat)		// Add alpha channel to palette (used for conversion to to *.SPZ format).
	{													// !!! Apply this to *.SPR palette BEFORE reformatting !!!
		uchar * NewPalette;
		ulong NewPaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPZ_PALETTE_ELEMENT_SIZE;

		if (this->PaletteSize == EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPR_PALETTE_ELEMENT_SIZE)
		{
			// Allocate memory for new palette
			NewPalette = (uchar *)malloc(NewPaletteSize);
			if (NewPalette == NULL)
			{
				puts("Unable to allocate memory!");
				_getch();
				exit(EXIT_FAILURE);
			}

			// Add transparency info to the palette and copy it to new place
			ulong ByteCounterOld = 0;
			ulong ByteCounterNew = 0;
			uchar TransparencyIndex = 0;
			bool IncrementTransparencyIndex = false;
			for (ByteCounterNew = 0; ByteCounterNew < NewPaletteSize; ByteCounterNew++)	// Copy colors + alpha to new palette
			{
				if ((ByteCounterNew + 1) % 4 != 0)
				{
					NewPalette[ByteCounterNew] = this->Palette[ByteCounterOld];
					ByteCounterOld++;
				}
				else
				{
					if (SPZFormat == SPZ_ADDITIVE)
					{
						NewPalette[ByteCounterNew] = 0x80;				// Mark all palette elements as non-transparent
					}
					else if (SPZFormat == SPZ_ALPHATEST)
					{
						if ((ByteCounterNew + 1) == NewPaletteSize)
							NewPalette[ByteCounterNew] = 0x00;			// Mark last element of palette as fully transparent
						else
							NewPalette[ByteCounterNew] = 0x80;
					}
					else if (SPZFormat == SPZ_INDEXALPHA)
					{
						// Increment transparency index within every two elements
						NewPalette[ByteCounterNew] = TransparencyIndex;

						if (IncrementTransparencyIndex == true)
							TransparencyIndex++;

						IncrementTransparencyIndex = (!IncrementTransparencyIndex);
					}
					else
					{
						NewPalette[ByteCounterNew] = 0x80;				// Treat unknown *.spz format as additive
					}
				}
			}

			// Destroy old palette
			free(this->Palette);

			// Save pointer to new palette
			this->Palette = NewPalette;

			// Update size
			this->PaletteSize = NewPaletteSize;
		}
	}

	void PalettePatchIAColors(uint PaletteElementSize, bool ToSPR)			// Patch indexalpha sprite's color table to SPR's (true) or SPZ's (false) format.
	{																		// !!! Apply this to *.SPR palette BEFORE reformatting !!!
		// Get last color entry in palette
		double R = this->Palette[(EIGHT_BIT_PALETTE_ELEMENTS_COUNT - 1) * PaletteElementSize + 0];
		double G = this->Palette[(EIGHT_BIT_PALETTE_ELEMENTS_COUNT - 1) * PaletteElementSize + 1];
		double B = this->Palette[(EIGHT_BIT_PALETTE_ELEMENTS_COUNT - 1) * PaletteElementSize + 2];

		// Patch color data
		for (uint i = 0; i < EIGHT_BIT_PALETTE_ELEMENTS_COUNT; i++)
		{
			if (ToSPR == true)
			{
				this->Palette[i * PaletteElementSize + 0] = (uchar) round(R / (double) EIGHT_BIT_PALETTE_ELEMENTS_COUNT * (double) i);
				this->Palette[i * PaletteElementSize + 1] = (uchar) round(G / (double) EIGHT_BIT_PALETTE_ELEMENTS_COUNT * (double) i);
				this->Palette[i * PaletteElementSize + 2] = (uchar) round(B / (double) EIGHT_BIT_PALETTE_ELEMENTS_COUNT * (double) i);
			}
			else
			{
				this->Palette[i * PaletteElementSize + 0] = (uchar) R;
				this->Palette[i * PaletteElementSize + 1] = (uchar) G;
				this->Palette[i * PaletteElementSize + 2] = (uchar) B;
			}
		}
	}

	void ScaleResize(ulong NewWidth, ulong NewHeight)			// Resize bitmap.
	{
		uchar * NewBitmap;

		// Allocate memory for new bitmap
		NewBitmap = (uchar *)malloc(NewWidth * NewHeight);
		if (NewBitmap == NULL)
		{
			puts("Unable to allocate memory!");
			_getch();
			exit(EXIT_FAILURE);
		}

		// Resize bitmap
		for (uint NewY = 0; NewY < NewHeight; NewY++)
			for (uint NewX = 0; NewX < NewWidth; NewX++)
			{
				uint OldY = (uint)round((double)NewY / (NewHeight - 1) * (this->Height - 1));
				uint OldX = (uint)round((double)NewX / (NewWidth - 1) * (this->Width - 1));

				NewBitmap[(NewWidth * NewY) + NewX] = this->Bitmap[(this->Width * OldY) + OldX];
			}


		// Destroy old bitmap
		free(this->Bitmap);

		// Set pointer to resized bitmap
		this->Bitmap = NewBitmap;

		// Update bitmap size
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->BitmapSize = this->Width * this->Height;
	}

	/*
	void UpdateImage(char * NewBitmap, ulong NewWidth, ulong NewHeight)
	{
		this->Bitmap = (uchar *)NewBitmap;
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->BitmapSize = this->Width * this->Height;
	}

	void UpdatePalette(char * NewPalette, ulong NewPaletteSize)
	{
		this->Palette = (uchar *)NewPalette;
		this->PaletteSize = NewPaletteSize;
	}

	void PaletteAddSpacers(uchar Spacer)		// Add spacers to to BMP's palette
	{
		uchar * NewPalette;
		ulong NewPaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPZ_BMP_PALETTE_ELEMENT_SIZE;

		if (this->PaletteSize == EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPR_PALETTE_ELEMENT_SIZE)
		{
			// Allocate memory for new palette
			NewPalette = (uchar *)malloc(NewPaletteSize);
			if (NewPalette == NULL)
			{
				puts("Unable to allocate memory!");
				_getch();
				exit(EXIT_FAILURE);
			}

			// Copy palette with spacers to new place
			ulong ByteCounterOld = 0;
			ulong ByteCounterNew = 0;
			for (ByteCounterNew = 0; ByteCounterNew < NewPaletteSize; ByteCounterNew++)	// Copy 3 bytes from old palette + Spacer to new one
			{
				if ((ByteCounterNew + 1) % 4 != 0)
				{
					NewPalette[ByteCounterNew] = this->Palette[ByteCounterOld];
					ByteCounterOld++;
				}
				else
				{
					NewPalette[ByteCounterNew] = Spacer;
				}
			}

			// Destroy old palette
			free(this->Palette);

			// Save pointer to new palette
			this->Palette = NewPalette;

			// Update size
			this->PaletteSize = NewPaletteSize;
		}
	}

	void PaletteSwapRedAndGreen(uint ElementSize)		// Needed for SPR\SPZ to BMP conversion and vice versa.
	{
		uchar Temp;

		for (uint Element = 0; Element < EIGHT_BIT_PALETTE_ELEMENTS_COUNT; Element++)	// Swap 1-st and 3-rd bytes in element
		{
			Temp = this->Palette[Element * ElementSize];
			this->Palette[Element * ElementSize] = this->Palette[Element * ElementSize + 2];
			this->Palette[Element * ElementSize + 2] = Temp;
		}
	}
	
		void Rename(const char * NewName)
	{
		strcpy(this->Name, NewName);
	}

	void TileResize(ulong NewWidth, ulong NewHeight)			// Resize bitmap by tiling old in new one.
	{
		uchar * NewBitmap;

		// Allocate memory for new bitmap
		NewBitmap = (uchar *)malloc(NewWidth * NewHeight);
		if (NewBitmap == NULL)
		{
			puts("Unable to allocate memory!");
			_getch();
			exit(EXIT_FAILURE);
		}

		// Tile new bitmap with old one
		ulong NewX, NewY, OldX, OldY;
		OldX = 0;
		OldY = 0;
		for (NewY = 0; NewY < NewHeight; NewY++)
		{
			for (NewX = 0; NewX < NewWidth; NewX++)
			{
				if (NewX == 0)
					OldX = 0;

				NewBitmap[(NewWidth * NewY) + NewX] = this->Bitmap[(this->Width * OldY) + OldX];

				OldX++;
				if (OldX >= this->Width)
					OldX = 0;
			}
			OldY++;
			if (OldY >= this->Height)
				OldY = 0;

		}



		// Destroy old bitmap
		free(this->Bitmap);

		// Save pointer to new bitmap
		this->Bitmap = NewBitmap;

		// Update bitmap size
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->BitmapSize = this->Width * this->Height;
	}

	void FlipBitmap()		// Flip bitmap vertically. Needed for SPZ\SPR to BMP conversion and vice versa.
	{
		uchar * NewBitmap;

		// Allocate memory for new bitmap
		NewBitmap = (uchar *)malloc(this->Width * this->Height);
		if (NewBitmap == NULL)
		{
			puts("Unable to allocate memory!");
			_getch();
			exit(EXIT_FAILURE);
		}

		// Copy flipped bitmap to new place
		for (uint y = 0; y < this->Height; y++)
			for (uint x = 0; x < this->Width; x++)
				NewBitmap[(this->Width * y) + x] = this->Bitmap[(this->Width * ((this->Height - 1) - y)) + x];

		// Destroy old bitmap
		free(this->Bitmap);

		// Save pointer to new bitmap
		this->Bitmap = NewBitmap;
	}
	*/
};
