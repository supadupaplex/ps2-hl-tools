// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This file contains all definitions and declarations
//

#ifndef MAIN_H
#define MAIN_H

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), sscanf(), snprintf()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round()
#include <ctype.h>		// tolower()

////////// Definitions //////////
#define PROG_TITLE "\nPS2 HL model tool v1.15\n"
#define PROG_INFO "\
Developed by supadupaplex, 2017-2021\n\
License: BSD-3-Clause (check out license.txt)\n\
\n\
How to use:\n\
1) Windows explorer - drag and drop model file on mdltool.exe\n\
2) Command line/Batch - mdltool [model_file_name]\n\
Optional features:\n\
 - extract textures: mdltool extract [filename]\n\
 - report sequences: mdltool seqrep [filename]\n\
\n\
For more info check out readme.txt\n\
"
#define DOL_TEXTURE_HEADER_SIZE 0x20
#define BMP_TEXTURE_HEADER_SIZE 0x35
#define MDL_TEXTURE_HEADER_SIZE 0x00
#define EIGHT_BIT_PALETTE_ELEMENTS_COUNT 256
#define DOL_BMP_PALETTE_ELEMENT_SIZE 4
#define MDL_PALETTE_ELEMENT_SIZE 3
#define PSI_MIN_DIMENSION 8
#define NOTEXTURES_MODEL 0
#define NORMAL_MODEL 1
#define SEQ_MODEL 2
#define DUMMY_MODEL 3
#define UNKNOWN_MODEL -1
#define MDL_DEF_REF_SZ 0x68
#define MDL_FILE_REF_SZ 0x48
#define MDL_FILE_REF_SPACE 0x20

// Keywords
#define KWD_FADESTART "fadestart"
#define KWD_FADEEND "fadeend"
#define KWD_MAXPARTS "maxparts"
#define KWD_NUMGROUPS "groups"
#define KWD_GROUP "group"
#define KWD_PART "part"
#define KWD_BLANK "blank"

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////
#include "fops.h"

////////// Structures //////////

// MDL/DOL model header
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sModelHeader
{
	char Signature[4];			// "IDST"
	ulong Version;				// 0xA - GoldSrc model
	char Name[64];				// Internal model name
	ulong FileSize;				// Model file size
	char SomeData1[88];			// Data that is not important for conversion
	ulong SeqCount;				// How many sequences
	ulong SeqTableOffset;		// Location of sequence table 
	ulong SubmodelCount;		// How many submodels
	ulong SubmodelTableOffset;	// Location of submodel table 
	ulong TextureCount;			// How many textures
	ulong TextureTableOffset;	// Texture table location
	ulong TextureDataOffset;	// Texture data location
	ulong SkinCount;			// How many skins
	ulong SkinEntrySize;		// Size of entry in skin table (measured in shorts)
	ulong SkinTableOffset;		// Location of skin table
	ulong SubmeshCount;			// How many submeshes
	ulong SubmeshTableOffset;	// Location of submesh table
	char SomeData2[32];			// Data that is not important for conversion

	void UpdateFromFile(FILE ** ptrFile)	// Update header from file
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sModelHeader));

		// I found some models that have long non null terminated internal name string
		// that was causing creepy beeping during printf(), so there is a fix for that
		Name[63] = '\0';
	}

	int CheckModel()					// Check model type
	{
		if (this->Signature[0] == 'I' && this->Signature[1] == 'D' && this->Signature[2] == 'S' && this->Version == 0xA)
		{
			if (this->Signature[3] == 'T')
			{
				if (this->TextureCount > 0)
					return NORMAL_MODEL;
				else
					return NOTEXTURES_MODEL;
			}
			else if (this->Signature[3] == 'Q')
			{
				return SEQ_MODEL;
			}
		}

		return UNKNOWN_MODEL;
	}

	void Rename(const char * NewName)
	{
		// Clear Name[] from garbage and leftovers
		memset(this->Name, 0x00, sizeof(this->Name));

		strcpy(this->Name, NewName);
	}
};

// MDL/DOL sequence descriptor
#pragma pack(1)					// No padding/spacers
struct sModelSeq
{
	char Name[32];			// Sequence name
	char SomeData1[124];
	int Num;				// Sequence file number
	char SomeData2[16];
};


// Extra section of DOL model headers
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sDOLExtraSection
{
	ulong LODDataOffset;	// Points to a location of a LOD data section
	uchar MaxBodyParts;		// Maximum number of body parts inside one body group
	uchar NumBodyGroups;	// How many body groups are present in the model (setting both MaxBodyParts and NumBodyGroups to 0 disables LODs)
	uchar Magic[2];			// Filled with zeroes
	ulong FadeStart;		// Model fade: start distance
	ulong FadeEnd;			// Model fade: end distance
};

// DOL model LOD table entry
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sDOLLODEntry
{
	ulong LODCount;			// Number of LODs for the specific body part (excluding full quality body part (LOD0))
	ulong LODDistances[4];	// Distances at which corresponding LODs are displayed
};

// MDL/DOL texture table entry
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sModelTextureEntry
{
	char Name[68];				// Texture name
	ulong Width;				// Texture width
	ulong Height;				// Texture height
	ulong Offset;				// Texture offset (in bytes)

	void UpdateFromFile(FILE ** ptrFile, ulong TextureTableOffset, ulong TextureTableEntryNumber)		// Update texture entry from model file
	{
		FileReadBlock(ptrFile, this, TextureTableOffset + TextureTableEntryNumber * sizeof(sModelTextureEntry), sizeof(sModelTextureEntry));
	}

	void Update(const char * NewName, ulong NewWidth, ulong NewHeight, ulong NewOffset)			// Update texture entry with new data
	{
		// Clear memory from garbage
		memset(this, 0x00, sizeof(sModelTextureEntry));

		// Update fields
		strcpy(this->Name, NewName);
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->Offset = NewOffset;
	}
};

// 8-bit *.bmp header
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sBMPHeader
{
	char Signature1[2];		// "BM" Signature
	ulong FileSize;			// Total file size (in bytes)
	ulong Signature2;		// 0x00000000
	ulong Offset;			// Offset
	ulong StructSize;		// BMP version
	ulong Width;			// Picture Width (in pixels)
	ulong Height;			// Picture Height (in pixels)
	ushort Signature3;		// 
	ushort BitsPerPixel;	// How many bits per 1 pixel
	ulong Compression;		// Compression type
	ulong PixelDataSize;	// Size of bitmap
	ulong HorizontalPPM;	// Horizontal pixels per meter value
	ulong VerticalPPM;		// Vertical pixels per meter value
	ulong ColorTabSize;		// How many colors are present in color table
	ulong ColorTabAlloc;	// How many colors are actually used in color table

	void Update(unsigned long int Width, unsigned long int Height)		// Update all fields of BMP header
	{
		this->Signature1[0] = 'B';				// Add 'BM' signature
		this->Signature1[1] = 'M';				//
		this->Signature2 = 0x00000000;			// Should be 0x00000000
		this->Offset = 0x00000436;				// Bitmap offset for version 3 BMP
		this->StructSize = 0x00000028;			// Version 3 BMP
		this->Signature3 = 0x0001;				// Should be 0x0001 for BMP
		this->BitsPerPixel = 0x0008;			// 8 bit palletized bitmap
		this->Compression = 0x00000000;			// RGB bitmap (no compression)
		this->HorizontalPPM = 0x00000000;		// Horizontal pixels per meter value (not used)
		this->VerticalPPM = 0x00000000;			// Vertical pixels per meter value (not used)
		this->ColorTabSize = 0x00000100;		// How many colors in color table
		this->ColorTabAlloc = 0x00000100;		// How many colors actually used in color table
		this->Width = Width;					// Picture Width (in pixels)
		this->Height = Height;					// Picture Height (in pixels)
		this->PixelDataSize = Height * Width;					// For this case it = Height * Width
		this->FileSize = this->PixelDataSize + this->Offset;	// For this case it = PixelDataSize + Offset
	}
};

// DOL texture (psi) header
struct sDOLTextureHeader
{
	char Name[16];		// Internal image name (same as texture name in model texture table, but without *.bmp and cut, if longer than 16 bytes)
	ulong LODCount;		// How many LODs. Used in decals only
	ulong Type;			// 2 - 8 bit palettized image, 5 - 32 bit RGBA image
	ushort Width;		// Texture width (in pixels)
	ushort Height;		// Texture height (in pixels)
	ushort UpWidth;		// Upscale: target width (in pixels)
	ushort UpHeight;	// Upscale: target height (in pixels)

	void Update(const char * NewName, ushort NewWidth, ushort NewHeight)
	{
		// Clear memory from garbage
		memset(this, 0x00, sizeof(sDOLTextureHeader));

		// Update fields
		snprintf(this->Name, sizeof(this->Name), "%s", NewName);
		this->LODCount = 0;
		this->Type = 2;
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->UpWidth = NewWidth;
		this->UpHeight = NewHeight;
	}
};

// Model texture data
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sTexture
{
	char Name[64];				// Texture name
	ulong Width;				// Texture width (in pixels)
	ulong Height;				// Texture height (in pixels)
	uchar * Palette;			// Texture palette
	ulong PaletteSize;			// Texture palette size
	uchar * Bitmap;				// Pointer to bitmap
	
	void Initialize()			// Initialize structure
	{
		strcpy(this->Name, "New_Texture");
		this->Width = 0;
		this->Height = 0;
		this->Palette = NULL;
		this->PaletteSize = 0;
		this->Bitmap = NULL;
	}

	void UpdateFromFile(FILE ** ptrFile, ulong FileBitmapOffset, ulong FileBitmapSize, ulong FilePaletteOffset, ulong FilePaletteSize, const char * NewName, ulong NewWidth, ulong NewHeight)	// Update from file
	{
		// Destroy old palette and bitmap
		free(Palette);
		free(Bitmap);

		// Allocate memory for new ones
		Palette = (uchar *) malloc(FilePaletteSize);
		Bitmap = (uchar *) malloc(FileBitmapSize);
		if (Palette == NULL || Bitmap == NULL)
		{
			UTIL_WAIT_KEY("Unable to allocate memory ...");
			exit(EXIT_FAILURE);
		}

		// Copy data from file to memory
		FileReadBlock(ptrFile, (char *) Palette, FilePaletteOffset, FilePaletteSize);
		FileReadBlock(ptrFile, (char *) Bitmap, FileBitmapOffset, FileBitmapSize);

		// Update other fields
		strcpy(this->Name, NewName);
		this->Width = NewWidth;
		this->Height = NewHeight;
		this->PaletteSize = FilePaletteSize;
	}

	/*void Rename(const char * NewName)
	{
		// Clear name from garbage and leftovers
		memset(this->Name, 0x00, sizeof(this->Name));;

		// Update name
		strcpy(this->Name, NewName);
	}

	void UpdateImageData(char * NewBitmap, ulong NewWidth, ulong NewHeight)
	{
		this->Bitmap = (uchar *) NewBitmap;
		this->Width = NewWidth;
		this->Height = NewHeight;
	}*/

	void ScaleResize(ulong NewWidth, ulong NewHeight)			// Resize bitmap. Used for MDL to DOL conversion.
	{
		char * NewBitmap;

		// Allocate memory for new bitmap
		NewBitmap = (char *) malloc(NewWidth * NewHeight);	
		if (NewBitmap == NULL)
		{
			UTIL_WAIT_KEY("Unable to allocate memory ...");
			exit(EXIT_FAILURE);
		}

		// Copy resized old bitmap to new one
		for (int NewY = 0; NewY < NewHeight; NewY++)
			for (int NewX = 0; NewX < NewWidth; NewX++)
			{
				int OldY = (int) round((double) NewY / (NewHeight - 1) * (this->Height - 1));
				int OldX = (int) round((double) NewX / (NewWidth - 1) * (this->Width - 1));

				NewBitmap[(NewWidth * NewY) + NewX] = this->Bitmap[(this->Width * OldY) + OldX];
			}

		
		// Destroy old bitmap
		free(this->Bitmap);

		// Save pointer to new bitmap
		this->Bitmap = (uchar *) NewBitmap;

		// Update bitmap size
		this->Width = NewWidth;
		this->Height = NewHeight;
	}

	void TileResize(ulong NewWidth, ulong NewHeight)			// Resize bitmap by tiling old in new one. Used for MDL to DOL conversion.
	{
		char * NewBitmap;

		// Allocate memory for new bitmap
		NewBitmap = (char *)malloc(NewWidth * NewHeight);
		if (NewBitmap == NULL)
		{
			UTIL_WAIT_KEY("Unable to allocate memory ...");
			exit(EXIT_FAILURE);
		}

		// Tile new bitmap with old one
		uint NewX, NewY, OldX, OldY;
		OldX = 0;
		OldY = 0;
		for (int NewY = 0; NewY < NewHeight; NewY++)
		{
			for (int NewX = 0; NewX < NewWidth; NewX++)
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
		this->Bitmap = (uchar *)NewBitmap;

		// Update bitmap size
		this->Width = NewWidth;
		this->Height = NewHeight;
	}

	void FlipBitmap()		// Flip bitmap vertically. Needed for DOL\MDL to BMP conversion and vice versa.
	{
		char * NewBitmap;

		// Allocate memory for new bitmap
		NewBitmap = (char *)malloc(this->Width * this->Height);
		if (NewBitmap == NULL)
		{
			UTIL_WAIT_KEY("Unable to allocate memory ...");
			exit(EXIT_FAILURE);
		}

		// Copy flipped bitmap to new place
		for (int y = 0; y < this->Height; y++)
			for (int x = 0; x < this->Width; x++)
				NewBitmap[(this->Width * y) + x] = this->Bitmap[(this->Width * ((this->Height - 1) - y)) + x];

		// Destroy old bitmap
		free(this->Bitmap);

		// Save pointer to new bitmap
		this->Bitmap = (uchar *) NewBitmap;
	}

	void PaletteReformat(uint PaletteElementSize)			// Reposition color table elements. Used for DOL to MDL and MDL to DOL conversions.
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

	/*
	// Old method of color correction (similar used in Jed's HLMV). Replaced with PaletteReformat().

	void PatchBitmap()	// Patch bitmap byte data. Needed for BMP\MDL to DOL conversion and vice versa.
	{
		uchar Remainder;

		for (int i = 0; i < (this->Width * this->Height); i++)
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

	void PaletteRemoveSpacers()	// Convert palette to MDL format
	{
		char * NewPalette;
		ulong NewPaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * MDL_PALETTE_ELEMENT_SIZE;

		if (this->PaletteSize == EIGHT_BIT_PALETTE_ELEMENTS_COUNT * DOL_BMP_PALETTE_ELEMENT_SIZE)
		{
			// Allocate memory for new palette
			NewPalette = (char *)malloc(NewPaletteSize);
			if (NewPalette == NULL)
			{
				UTIL_WAIT_KEY("Unable to allocate memory ...");
				exit(EXIT_FAILURE);
			}

			// Copy palette without spacers to new place
			int ByteCounterOld = 0;
			int ByteCounterNew = 0;
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
			this->Palette = (uchar *) NewPalette;

			// Update size
			this->PaletteSize = NewPaletteSize;
		}
	}

	void PaletteAddSpacers(char Spacer)		// Convert palette to DOL\BMP format
	{
		char * NewPalette;
		ulong NewPaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * DOL_BMP_PALETTE_ELEMENT_SIZE;

		if (this->PaletteSize == EIGHT_BIT_PALETTE_ELEMENTS_COUNT * MDL_PALETTE_ELEMENT_SIZE)
		{
			// Allocate memory for new palette
			NewPalette = (char *)malloc(NewPaletteSize);
			if (NewPalette == NULL)
			{
				UTIL_WAIT_KEY("Unable to allocate memory ...");
				exit(EXIT_FAILURE);
			}

			// Copy palette with spacers to new place
			int ByteCounterOld = 0;
			int ByteCounterNew = 0;
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
			this->Palette = (uchar *) NewPalette;

			// Update size
			this->PaletteSize = NewPaletteSize;
		}
	}

	void PaletteSwapRedAndGreen(int ElementSize)		// Needed for MDL/DOL to BMP conversion and vice versa.
	{
		char Temp;

		for (int Element = 0; Element < EIGHT_BIT_PALETTE_ELEMENTS_COUNT; Element++)	// Swap 1-st and 3-rd bytes in element
		{
			Temp = this->Palette[Element * ElementSize];
			this->Palette[Element * ElementSize] = this->Palette[Element * ElementSize + 2];
			this->Palette[Element * ElementSize + 2] = Temp;
		}
	}
};

#endif // MAIN_H
