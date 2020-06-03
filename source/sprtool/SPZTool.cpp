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
// This file contains sprite conversion functions
//

////////// Includes //////////
#include "main.h"

////////// Functions //////////
void ConvertSPZToSPR(const char * cFile, bool Resize);
void ConvertSPRToSPZ(const char * cFile, bool HQResize);
uint PSIProperSize(uint Size, bool HQ);

void ConvertSPZToSPR(const char * cFile, bool Resize)
{
	FILE * ptrSPZ;
	FILE * ptrSPR;
	char cOutputFileName[255];

	sSPZHeader SPZHeader;
	sSPZFrameTableEntry * SPZFrameTable;
	sSPZFrameHeader * SPZFrameHeaders;

	sSPRHeader SPRHeader;
	sSPRFrameHeader SPRFrameHeader;
	eSPRType SPRType;
	eSPRFormat SPRFormat;

	sTexture * Textures;

	// Open *.spz file
	SafeFileOpen(&ptrSPZ, cFile, "rb");
	// Load header from file
	SPZHeader.UpdateFromFile(&ptrSPZ);
	// Check file
	if (SPZHeader.CheckSignature() == false)
	{
		puts("Incorrect file.");
		return;
	}
	if (SPZHeader.FrameCount == 0)
	{
		puts("Empty sprite file.");
		return;
	}

	// Load frame table from *.spz file
	SPZFrameTable = (sSPZFrameTableEntry *)malloc(sizeof(sSPZFrameTableEntry) * SPZHeader.FrameCount);
	for (int i = 0; i < SPZHeader.FrameCount; i++)
		SPZFrameTable[i].UpdateFromFile(&ptrSPZ, sizeof(sSPZHeader) + i * sizeof(sSPZFrameTableEntry));


	// Load frame headers from *.spz file
	SPZFrameHeaders = (sSPZFrameHeader *)malloc(sizeof(sSPZFrameHeader) * SPZHeader.FrameCount);
	for (int i = 0; i < SPZHeader.FrameCount; i++)
		SPZFrameHeaders[i].UpdateFromFile(&ptrSPZ, SPZFrameTable[i].FrameOffset);


	// Load frames from *.spz file
	Textures = (sTexture *)malloc(sizeof(sTexture) * SPZHeader.FrameCount);
	uint BitmapOffset;
	uint BitmapSize;
	uint PaletteOffset;
	uint PaletteSize;
	uint MaxWidth = SPZFrameHeaders[0].Width;
	uint MaxHeight = SPZFrameHeaders[0].Height;
	for (int i = 0; i < SPZHeader.FrameCount; i++)
	{
		// Load frame
		BitmapOffset = SPZFrameTable[i].FrameOffset + sizeof(sSPZFrameHeader) + EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPZ_PALETTE_ELEMENT_SIZE;
		BitmapSize = SPZFrameHeaders[i].Width * SPZFrameHeaders[i].Height;
		PaletteOffset = SPZFrameTable[i].FrameOffset + sizeof(sSPZFrameHeader);
		PaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPZ_PALETTE_ELEMENT_SIZE;

		Textures[i].Initialize();
		Textures[i].UpdateFromFile(&ptrSPZ, BitmapOffset, BitmapSize, PaletteOffset, PaletteSize, SPZFrameHeaders[i].Name, SPZFrameHeaders[i].Width, SPZFrameHeaders[i].Height);

		// Resize frame to it's original size (if specified)
		if (Resize == true)
			Textures[i].ScaleResize(SPZFrameHeaders[i].UpWidth, SPZFrameHeaders[i].UpHeight);

		// Find maximum frame sizes
		if (Textures[i].Width > MaxWidth)
			MaxWidth = Textures[i].Width;
		if (Textures[i].Height > MaxHeight)
			MaxHeight = Textures[i].Height;
	}

	// Create new *.spr file
	FileGetFullName(cFile, cOutputFileName, sizeof(cOutputFileName));
	strcat(cOutputFileName, ".spr");
	SafeFileOpen(&ptrSPR, cOutputFileName, "wb");

	// Detect *.spz format
	if (Textures[0].PaletteCheckSPZFormat() == SPZ_ADDITIVE)
	{
		puts("Format: additive");
		SPRFormat = SPR_ADDITIVE;
	}
	else if (Textures[0].PaletteCheckSPZFormat() == SPZ_ALPHATEST)
	{
		puts("Format: alphatest");
		SPRFormat = SPR_ALPHATEST;
	}
	else if (Textures[0].PaletteCheckSPZFormat() == SPZ_INDEXALPHA)
	{
		puts("Format: indexalpha");
		SPRFormat = SPR_INDEXALPHA;
	}
	else
	{
		printf("Detected unknown format: [0x%X]. Treating as additive \n", Textures[0].PaletteCheckSPZFormat());
		SPRFormat = SPR_ADDITIVE;
	}

	// Detect *.spz type
	if (SPZHeader.Type == SPZ_VP_PARALLEL)
	{
		puts("Type: vp_parallel");
		SPRType = SPR_VP_PARALLEL;
	}
	else if (SPZHeader.Type == SPZ_VP_PARALLEL_UPRIGHT)
	{
		puts("Type: vp_parallel_upright");
		SPRType = SPR_VP_PARALLEL_UPRIGHT;
	}
	else if (SPZHeader.Type == SPZ_ORIENTED)
	{
		puts("Type: oriented");
		SPRType = SPR_ORIENTED;
	}
	else if (SPZHeader.Type == SPZ_VP_PARALLEL_ORIENTED)
	{
		puts("Type: vp_parallel_oriented");
		SPRType = SPR_VP_PARALLEL_ORIENTED;
	}
	else
	{
		printf("Detected unknown type: [0x%X]. Treating as vp_parellel \n", SPZHeader.Type);
		SPRType = SPR_VP_PARALLEL;
	}

	// Convert frames to appropriate format
	for (int i = 0; i < SPZHeader.FrameCount; i++)
	{
		Textures[i].PaletteRemoveAlpha();
		Textures[i].PaletteReformat(SPR_PALETTE_ELEMENT_SIZE);
		Textures[i].PaletteMulDiv(true);
		if (SPRFormat == SPR_INDEXALPHA)
			Textures[i].PalettePatchIAColors(SPR_PALETTE_ELEMENT_SIZE, true);
	}

	// Write header
	SPRHeader.Update(MaxWidth, MaxHeight, SPZHeader.FrameCount, SPRType, SPRFormat);
	FileWriteBlock(&ptrSPR, &SPRHeader, sizeof(sSPRHeader));
	
	// Write palette (taking palette from 1-st textre as sprite palette)
	FileWriteBlock(&ptrSPR, Textures[0].Palette, Textures[0].PaletteSize);

	// Write frames
	for (int i = 0; i < SPZHeader.FrameCount; i++)
	{
		SPRFrameHeader.Update(Textures[i].Width, Textures[i].Height);			// Write header
		FileWriteBlock(&ptrSPR, &SPRFrameHeader, sizeof(sSPRFrameHeader));

		FileWriteBlock(&ptrSPR, Textures[i].Bitmap, Textures[i].BitmapSize);	// Write bitmap
	}

	// Free memory
	free(SPZFrameHeaders);
	free(SPZFrameTable);
	free(Textures);

	// Close files
	fclose(ptrSPZ);
	fclose(ptrSPR);
}

void ConvertSPRToSPZ(const char * cFile, bool HQResize)
{
	FILE * ptrSPZ;
	FILE * ptrSPR;
	char cOutputFileName[255];

	sSPRHeader SPRHeader;
	sSPRFrameHeader * SPRFrameHeaders;

	sSPZHeader SPZHeader;
	sSPZFrameTableEntry SPZFrameTableEntry;
	sSPZFrameHeader SPZFrameHeader;
	eSPZType SPZType;
	eSPZFormat SPZFormat;

	sTexture * Textures;

	// Open *.spr file
	SafeFileOpen(&ptrSPR, cFile, "rb");
	// Load header from file
	SPRHeader.UpdateFromFile(&ptrSPR);
	// Check file
	if (SPRHeader.CheckSignature() == false)
	{
		puts("Incorrect file.");
		return;
	}
	if (SPRHeader.FrameCount == 0)
	{
		puts("Empty sprite file.");
		return;
	}

	// Load frame headers from *.spr file
	SPRFrameHeaders = (sSPRFrameHeader *)malloc(sizeof(sSPRFrameHeader) * SPRHeader.FrameCount);
	ulong HeaderOffset = sizeof(sSPRHeader) + EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPR_PALETTE_ELEMENT_SIZE;
	for (int i = 0; i < SPRHeader.FrameCount; i++)
	{
		SPRFrameHeaders[i].UpdateFromFile(&ptrSPR, HeaderOffset);

		// Calculate offset of the next header
		HeaderOffset += sizeof(sSPRFrameHeader) + SPRFrameHeaders[i].Width * SPRFrameHeaders[i].Height;
	}

	// Load frames from *.spr file
	Textures = (sTexture *)malloc(sizeof(sTexture) * SPRHeader.FrameCount);
	uint BitmapOffset;
	uint BitmapSize;
	uint PaletteOffset;
	uint PaletteSize;
	uint FrameOffset = sizeof(sSPRHeader) + EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPR_PALETTE_ELEMENT_SIZE;
	char Temp[13];
	char TextureName[16];
	char FileName[64];
	FileGetName(cFile, FileName, sizeof(FileName), false);
	for (int i = 0; i < SPRHeader.FrameCount; i++)
	{
		// Load frame
		BitmapOffset = FrameOffset + sizeof(sSPRFrameHeader);
		BitmapSize = SPRFrameHeaders[i].Width * SPRFrameHeaders[i].Height;
		PaletteOffset = sizeof(sSPRHeader);
		PaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPR_PALETTE_ELEMENT_SIZE;

		Textures[i].Initialize();
		snprintf(Temp, sizeof(Temp), "%s", FileName);
		snprintf(TextureName, sizeof(TextureName), "%s%03i", Temp, i + 1);
		Textures[i].UpdateFromFile(&ptrSPR, BitmapOffset, BitmapSize, PaletteOffset, PaletteSize, TextureName, SPRFrameHeaders[i].Width, SPRFrameHeaders[i].Height);

		// Calculate offset for next frame
		FrameOffset += sizeof(sSPRFrameHeader) + SPRFrameHeaders[i].Width * SPRFrameHeaders[i].Height;
	}

	// Create new *.spz file
	FileGetFullName(cFile, cOutputFileName, sizeof(cOutputFileName));
	strcat(cOutputFileName, ".spz");
	SafeFileOpen(&ptrSPZ, cOutputFileName, "wb");

	// Detect format
	if (SPRHeader.Format == SPR_ADDITIVE)
	{
		puts("Format: additive");
		SPZFormat = SPZ_ADDITIVE;
	}
	else if (SPRHeader.Format == SPR_ALPHATEST)
	{
		puts("Format: alphatest");
		SPZFormat = SPZ_ALPHATEST;
	}
	else if (SPRHeader.Format == SPR_INDEXALPHA)
	{
		puts("Format: indexalpa");
		SPZFormat = SPZ_INDEXALPHA;
	}
	else
	{
		puts("Unappropriate format for conversion to *.spz. Treating as additive.");
		SPZFormat = SPZ_ADDITIVE;
	}

	// Detect type
	if (SPRHeader.Type == SPR_VP_PARALLEL_UPRIGHT || SPRHeader.Type == SPR_FACING_UPRIGHT)
	{
		puts("Type: vp_parallel_upright");
		SPZType = SPZ_VP_PARALLEL_UPRIGHT;
	}
	else if (SPRHeader.Type == SPR_VP_PARALLEL)
	{
		puts("Type: vp_parallel");
		SPZType = SPZ_VP_PARALLEL;
	}
	else if (SPRHeader.Type == SPR_ORIENTED)
	{
		puts("Type: oriented");
		SPZType = SPZ_ORIENTED;
	}
	else if (SPRHeader.Type == SPR_VP_PARALLEL_ORIENTED)
	{
		puts("Type: vp_parallel_oriented");
		SPZType = SPZ_VP_PARALLEL_ORIENTED;
	}
	else
	{
		puts("Unappropriate type for conversion to *.spz. Treating as vp_parallel.");
		SPZType = SPZ_VP_PARALLEL;
	}

	// Convert frames to appropriate format
	for (int i = 0; i < SPRHeader.FrameCount; i++)
	{
		Textures[i].PaletteMulDiv(false);
		Textures[i].PaletteAddSPZAlpha(SPZFormat);
		if (SPZFormat == SPZ_INDEXALPHA)
			Textures[i].PalettePatchIAColors(SPZ_PALETTE_ELEMENT_SIZE, false);
		Textures[i].PaletteReformat(SPZ_PALETTE_ELEMENT_SIZE);
	}

	// Write header
	SPZHeader.Update(SPRHeader.FrameCount, SPZType);
	FileWriteBlock(&ptrSPZ, &SPZHeader, sizeof(sSPZHeader));

	// Write frame table
	FrameOffset = sizeof(sSPZHeader) + sizeof(sSPZFrameTableEntry) * SPRHeader.FrameCount;
	if ((SPRHeader.FrameCount % 2) == 0)		// Make sure that first frame starts from new 16-byte block (PS2 version likes everything to be aligned)
		FrameOffset += 8;
	for (int i = 0; i < SPRHeader.FrameCount; i++)
	{
		// Write frame table entry to file
		SPZFrameTableEntry.Update(FrameOffset);
		FileWriteBlock(&ptrSPZ, &SPZFrameTableEntry, sizeof(sSPZFrameTableEntry));

		// Calculate offset for next frame (with resizing in mind)
		FrameOffset += sizeof(sSPZFrameHeader) + EIGHT_BIT_PALETTE_ELEMENTS_COUNT * SPZ_PALETTE_ELEMENT_SIZE + PSIProperSize(Textures[i].Width, HQResize) * PSIProperSize(Textures[i].Height, HQResize);
	}

	// Add 8 blank bytes if table has even number of elements (PS2 version likes everything to be alligned within 16-byte sized sectors)
	if ((SPRHeader.FrameCount % 2) == 0)
		for (int i = 0; i < 8; i++)
			fputc(0x00, ptrSPZ);

	// Write frames
	uint OriginalHeight;
	uint OriginalWidth;
	for (int i = 0; i < SPRHeader.FrameCount; i++)
	{
		// Resize frame to approriate for PS2 HL size
		OriginalWidth = Textures[i].Width;
		OriginalHeight = Textures[i].Height;
		Textures[i].ScaleResize(PSIProperSize(OriginalWidth, HQResize), PSIProperSize(OriginalHeight, HQResize));

		// Write header
		SPZFrameHeader.Update(Textures[i].Name, Textures[i].Width, Textures[i].Height);
		SPZFrameHeader.UpdateUpscaleTarget(OriginalWidth, OriginalHeight);
		FileWriteBlock(&ptrSPZ, &SPZFrameHeader, sizeof(sSPZFrameHeader));

		// Write palette
		FileWriteBlock(&ptrSPZ, Textures[i].Palette, Textures[i].PaletteSize);

		// Write bitmap
		FileWriteBlock(&ptrSPZ, Textures[i].Bitmap, Textures[i].BitmapSize);
	}

	// Free memory
	free(SPRFrameHeaders);
	free(Textures);

	// Close files
	fclose(ptrSPR);
	fclose(ptrSPZ);
}

uint PSIProperSize(uint Size, bool HQ)	// Function returns closest proper dimension. PS2 HL proper PSI dimensions: 16 (min), 32, 64, 128, 256, 512, ...
{
	uint CurrentSize;

	// Calculate closest (bigger or equal) proper dimension
	CurrentSize = PSI_MIN_DIMENSION;
	while (CurrentSize < Size)
		CurrentSize *= 2;

	// Return proper dimension
	if (Size <= PSI_MIN_DIMENSION)
	{
		return PSI_MIN_DIMENSION;
	}
	else
	{
		if (Size > (CurrentSize / 2 + CurrentSize / 4) || HQ == true)
			return CurrentSize;
		else
			return CurrentSize / 2;
	}
}

int main(int argc, char * argv[])
{
	printf("PS2 HL Sprite Tool v%s\n", PROG_VERSION);

	if (argc == 1)
	{
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2017, Alexey Leushin. All rights reserved.\n");
		puts("How to use: \n1) Windows explorer - drag and drop sprite file on sprtool.exe \n2) Command line\\Batch - sprtool (noresize) [file_name] \n\nFor more info read ReadMe.txt \n");
		puts("Press any key to exit ...");
		_getch();
	}
	else if (argc == 2)
	{
		char Extension[5];
		FileGetExtension(argv[1], Extension, sizeof(Extension));

		if (!strcmp(Extension, ".spr") == true)
		{
			printf("Proccessing file: %s \n", argv[1]);
			ConvertSPRToSPZ(argv[1], false);
			puts("Done! \n");
			return 0;
		}
		else if (!strcmp(Extension, ".spz") == true)
		{
			printf("Proccessing file: %s \n", argv[1]);
			ConvertSPZToSPR(argv[1], true);
			puts("Done! \n");
			return 0;
		}
		else
		{
			puts("Wrong file extension.");
		}

	}
	else if (argc == 3)
	{
		char Extension[5];
		FileGetExtension(argv[2], Extension, sizeof(Extension));

		if (!strcmp(argv[1], "noresize") == true && !strcmp(Extension, ".spz") == true)
		{
			printf("Proccessing file: %s \n", argv[2]);
			ConvertSPZToSPR(argv[2], false);
			puts("Done! \n");
			return 0;
		}
		else if (!strcmp(argv[1], "hq") == true && !strcmp(Extension, ".spr") == true)
		{
			printf("Proccessing file: %s \n", argv[2]);
			ConvertSPRToSPZ(argv[2], true);
			puts("Done! \n");
			return 0;
		}
		else
		{
			puts("Nope! \n");
		}
	}
	else
	{
		puts("Wrong command line parameters.");
	}
	
	return 1;
}