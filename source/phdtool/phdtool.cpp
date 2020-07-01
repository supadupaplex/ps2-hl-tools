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
// This file contains decal conversion functions
// 

////////// Includes //////////
#include "util.h"
#include "main.h"

////////// Functions //////////
bool ConvertPNGtoPHD(const char * FileName);
bool ConvertPHDtoPNG(const char * FileName);
uint PSIProperSize(uint Size);
void ScaleBitmap(uchar ** Bitmap, ulong * BitmapSize, uint OldWidth, uint OldHeight, uint NewWidth, uint NewHeight, bool Linear);
int BilinearPixel(uchar * Bitmap, int Width, int Height, int NewWidth, int NewHeight, int nx, int ny);
uchar LinFilter(uchar PxBegin, uchar PxEnd, float Ratio);
uchar CreateMIPs(uchar ** Bitmap, ulong * BitmapSize, uint Width, uint Height);
void PaletteFix(uchar * RGBAPalette, ulong RGBAPaletteSize, bool MulDiv);
bool ConvertBMPtoPHD(const char * FileName, bool Linear);
bool ConvertPHDtoBMP(const char * FileName, bool Linear);
void ConvertDecalPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool ToBMP);
void PaletteSwapRedAndGreen(uchar * RGBAPalette, ulong RGBAPaletteSize);
void FlipBitmap(uchar ** Bitmap, ulong * BitmapSize, uint Width, uint Height);


bool ConvertPNGtoPHD(const char * FileName)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

	sPNGHeader PNGHeader;
	sPHDHeader PHDHeader;
	sPSIHeader PSIHeader;
	uchar MIPCount;

	sPNGData * PNGPalette;
	sPNGData * PNGBitmap;
	uchar BytesPerPixel;

	char OutFile[PATH_LEN];
	char TexName[64];

	// Open file
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read PNG header
	PNGHeader.UpdateFromFile(&ptrInputF);
	PNGHeader.SwapEndian();

	// Check PNG header
	if (PNGHeader.CheckType() == PNG_INDEXED)
	{
		printf("8-bit PNG \nParameters - Width: %i, Height: %i \n", PNGHeader.Width, PNGHeader.Height);
		BytesPerPixel = 1;

		// Prepare PSI palette
		PNGPalette = PNGReadPalette(&ptrInputF);
		PaletteFix(PNGPalette->Data, PNGPalette->DataSize, false);

		// Prepare PSI bitmap
		PNGBitmap = PNGReadBitmap(&ptrInputF, PNGHeader.Width, PNGHeader.Height, BytesPerPixel, PNGHeader.BitDepth);

		// Resize bitmap to proper size
		uint OriginalWidth = PNGHeader.Width;
		uint OriginalHeight = PNGHeader.Height;
		PNGHeader.Update(PSIProperSize(OriginalWidth), PSIProperSize(OriginalHeight), PNG_INDEXED);
		ScaleBitmap(&PNGBitmap->Data, &PNGBitmap->DataSize, OriginalWidth, OriginalHeight, PSIProperSize(OriginalWidth), PSIProperSize(OriginalHeight), false);

		// Create MIPs
		MIPCount = CreateMIPs(&PNGBitmap->Data, &PNGBitmap->DataSize, PNGHeader.Width, PNGHeader.Height);

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PHD header
		PHDHeader.Update();
		FileWriteBlock(&ptrOutputF, &PHDHeader, sizeof(sPHDHeader));

		// Write PSI header
		FileGetName(FileName, TexName, sizeof(TexName), false);
		PSIHeader.Update(TexName, PNGHeader.Width, PNGHeader.Height, PSI_INDEXED, MIPCount);
		PSIHeader.UpdateUpscaleTaget(OriginalWidth, OriginalHeight);
		FileWriteBlock(&ptrOutputF, &PSIHeader, sizeof(sPSIHeader));

		// Write PSI data
		FileWriteBlock(&ptrOutputF, PNGPalette->Data, PNGPalette->DataSize);
		FileWriteBlock(&ptrOutputF, PNGBitmap->Data, PNGBitmap->DataSize);

		// Free memory
		free(PNGBitmap->Data);
		free(PNGPalette->Data);

		// Close files
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else
	{
		puts("8 bit PNG reqired ...");
		return false;
	}

	// Close files
	fclose(ptrInputF);

	return true;
}

bool ConvertPHDtoPNG(const char * FileName)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

	sPHDHeader PHDHeader;
	sPNGHeader PNGHeader;
	sPSIHeader PSIHeader;

	sPNGData PNGPalette;
	sPNGData PNGBitmap;
	uchar BytesPerPixel;

	char OutFile[PATH_LEN];

	uchar * RawBitmap;
	ulong RawBitmapSize;

	uchar * RGBAPalette;
	ulong RGBAPaletteSize;

	// Open PHD
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read and check PHD header
	PHDHeader.UpdateFromFile(&ptrInputF);
	if (PHDHeader.Check() != true)
	{
		puts("Invalid decal file ...");
		return false;
	}

	// Read PSI Header
	PSIHeader.UpdateFromFile(&ptrInputF, sizeof(sPHDHeader));

	// Check PSI Header
	if (PSIHeader.CheckType() == PSI_INDEXED)
	{
		printf("Parameters - Width: %i, Height: %i \n", PSIHeader.Width, PSIHeader.Height);
		BytesPerPixel = 1;

		// Prepare PNG palette
		RGBAPaletteSize = 0x400;
		RGBAPalette = (uchar *)malloc(RGBAPaletteSize);
		if (RGBAPalette == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RGBAPalette, sizeof(sPHDHeader) + sizeof(sPSIHeader), RGBAPaletteSize);
		PaletteFix(RGBAPalette, RGBAPaletteSize, true);
		PNGPalette.Data = RGBAPalette;
		PNGPalette.DataSize = RGBAPaletteSize;

		// Prepare PNG bitmap
		RawBitmapSize = PSIHeader.Height * PSIHeader.Width * BytesPerPixel;
		RawBitmap = (uchar *)malloc(RawBitmapSize);
		if (RawBitmap == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RawBitmap, sizeof(sPHDHeader) + sizeof(sPSIHeader) + RGBAPaletteSize, RawBitmapSize);
		ScaleBitmap(&RawBitmap, &RawBitmapSize, PSIHeader.Width, PSIHeader.Height, PSIHeader.UpWidth, PSIHeader.UpHeight, false);	// Resize bitmap to it's original size
		PNGBitmap.Data = RawBitmap;
		PNGBitmap.DataSize = RawBitmapSize;

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".png");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PNG header
		PNGHeader.Update(PSIHeader.UpWidth, PSIHeader.UpHeight, PNG_INDEXED);
		PNGHeader.SwapEndian();
		FileWriteBlock(&ptrOutputF, &PNGHeader, sizeof(sPNGHeader));

		// Write PNG data
		PNGWriteChunk(&ptrOutputF, "iTXt", "Comment\0\0\0\0\0Converted with PS2 Half-life PHD tool", strlen("CommentConverted with PS2 Half-life PHD tool") + 5);
		PNGWritePalette(&ptrOutputF, &PNGPalette);
		PNGWriteBitmap(&ptrOutputF, PSIHeader.UpWidth, PSIHeader.UpHeight, BytesPerPixel, &PNGBitmap);
		PNGWriteChunk(&ptrOutputF, "IEND", NULL, NULL);

		// Free memory
		free(PNGBitmap.Data);
		free(RGBAPalette);

		// Close file
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else
	{
		puts("Can't recognise decal");
		return false;
	}

	// Close file
	fclose(ptrInputF);

	return true;
}

uint PSIProperSize(uint Size)	// Function returns closest proper dimension. PS2 HL proper PSI dimensions: 8 (min), 16, 32, 64, 128, 256, 512, ...
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
		if (Size > (CurrentSize / 2 + CurrentSize / 4))
			return CurrentSize;
		else
			return CurrentSize / 2;
	}
}

void ScaleBitmap(uchar ** Bitmap, ulong * BitmapSize, uint OldWidth, uint OldHeight, uint NewWidth, uint NewHeight, bool Linear)
{
	uchar * NewBitmap;

	// Skip if image has target size already
	if (OldWidth == NewWidth && OldHeight == NewHeight)
		return;

	// Allocate memory for new bitmap
	NewBitmap = (uchar *)malloc(NewWidth * NewHeight);
	if (NewBitmap == NULL)
	{
		puts("Unable to allocate memory!");
		UTIL_WAIT_KEY;
		exit(EXIT_FAILURE);
	}

	// Resize bitmap
	if (Linear)
	{
		// Linear, works only with BMP decals
		uchar * pPixel = NewBitmap;
		for (uint NewY = 0; NewY < NewHeight; NewY++)
			for (uint NewX = 0; NewX < NewWidth; NewX++)
			{
				*pPixel = BilinearPixel(*Bitmap, OldWidth, OldHeight, NewWidth, NewHeight, NewX, NewY);
				pPixel++;
			}
	}
	else
	{
		// Nearest, always works
		for (uint NewY = 0; NewY < NewHeight; NewY++)
			for (uint NewX = 0; NewX < NewWidth; NewX++)
			{
				uint OldY = (uint)round((double)NewY / (NewHeight - 1) * (OldHeight - 1));
				uint OldX = (uint)round((double)NewX / (NewWidth - 1) * (OldWidth - 1));

				NewBitmap[(NewWidth * NewY) + NewX] = (*Bitmap)[(OldWidth * OldY) + OldX];
			}
	}

	// Destroy old bitmap
	free(*Bitmap);

	// Set pointer to resized bitmap
	*Bitmap = NewBitmap;

	// Update bitmap size
	*BitmapSize = NewWidth * NewHeight;
}

// Args:
// PxBegin [		/Ratio/  ] PxEnd; ratio is between 0.0 and 1.0
// return - filtered value
uchar LinFilter(uchar PxBegin, uchar PxEnd, float Ratio)
{
	// Check ratio
	if (Ratio <= 0)
		return PxBegin;
	if (Ratio >= 1)
		return PxEnd;

	// Get result
	return (uchar) roundf( Ratio*((float)PxEnd-(float)PxBegin) + (float)PxBegin );
}

// Args:
// Bitmap, Width, Height - original bitmap pointer and dimensions
// Width, Height - new bitmap dimensions
// nx, ny - new bitmap pixel coordinates
// return - index of new bitmap pixel
#define CAP_VAL(VAL, MIN, MAX)	{ if (VAL > (MAX)) VAL = (MAX); else if (VAL < (MIN)) VAL = (MIN); }
int BilinearPixel(uchar * Bitmap, int Width, int Height, int NewWidth, int NewHeight, int nx, int ny)
{
	// Calculate sample coordinates
	float PxSzX = (float)Width / (float)NewWidth;
	float PxSzY = (float)Height / (float)NewHeight;
	float CoordX = PxSzX * ((float)nx + 0.5) - 0.5;
	float CoordY = PxSzY * ((float)ny + 0.5) - 0.5;
	CAP_VAL(CoordX, 0, Width-1);
	CAP_VAL(CoordY, 0, Height-1);

	// Collect pixels for filter
	float RatioX, RatioY, IntX, IntY;
	int Left, Top, Right, Bottom;
	RatioX = modff(CoordX, &IntX);
	RatioY = modff(CoordY, &IntY);
	Right = Left = (int)IntX;
	Bottom = Top = (int)IntY;
	Right++;
	Bottom++;
	CAP_VAL(Right, 0, Width-1);
	CAP_VAL(Bottom, 0, Height-1);

	// Apply linear filter
	uchar hSampleTop, hSampleBot;
	int BaseTop = Width * Top;
	int BaseBot = Width * Bottom;
	hSampleTop = LinFilter(*(Bitmap+BaseTop+Left), *(Bitmap+BaseTop+Right), RatioX);
	hSampleBot = LinFilter(*(Bitmap+BaseBot+Left), *(Bitmap+BaseBot+Right), RatioX);
	return LinFilter(hSampleTop, hSampleBot, RatioY);
}

uchar CreateMIPs(uchar ** Bitmap, ulong * BitmapSize, uint Width, uint Height)			// Create MIPs for decal. Function overrides old bitmap and returns MIP count.
{
	uint a;
	uint b;
	uchar MIPCount;
	ulong MIPSize;
	uchar * NewBitmap;
	ulong NewBitmapSize;
	uchar * OldBitmap = *Bitmap;

	// Check how many MIPs needed
	a = Width;
	b = Height;
	MIPCount = 0;
	MIPSize = 0;
	while (a > 8 && b > 8)		// At least one side of smallest MIP should be less or equal 8
	{
		a /= 2;
		b /= 2;
		MIPSize += (a * b);
		MIPCount++;
	}

	// Allocate memory for new bitmap
	NewBitmapSize = *BitmapSize + MIPSize;
	NewBitmap = (uchar *)malloc(NewBitmapSize);

	// Actual MIP creation work
	ulong Offset = 0;
	for (int CurrentMIP = 0; CurrentMIP <= MIPCount; CurrentMIP++)
	{
		uint MIPWidth = Width >> CurrentMIP;
		uint MIPHeight = Height >> CurrentMIP;

		// Create MIP
		for (int NewY = 0; NewY < MIPHeight; NewY++)
		{
			for (int NewX = 0; NewX < MIPWidth; NewX++)
			{
				int OldX = (int)round((double)NewX / (MIPWidth - 1) * (Width - 1));
				int OldY = (int)round((double)NewY / (MIPHeight - 1) * (Height - 1));

				NewBitmap[Offset + (MIPWidth * NewY) + NewX] = OldBitmap[(Width * OldY) + OldX];
			}
		}

		// Calculate offset of new MIP
		Offset += MIPHeight * MIPWidth;
	}


	// Destroy old bitmap
	free(OldBitmap);

	// Save pointer to new bitmap
	*Bitmap = NewBitmap;
	*BitmapSize = NewBitmapSize;

	return MIPCount;
}

void PaletteFix(uchar * RGBAPalette, ulong RGBAPaletteSize,  bool MulDiv)			// Fix/unfix color table
{
	uchar Remainder;
	uchar Temp;

	// Reformat color table
	uint PaletteElementSize = 4;
	for (ulong i = 0; i < RGBAPaletteSize; i++)
	{
		Remainder = i % (0x20 * PaletteElementSize);

		if (((0x10 * PaletteElementSize) <= Remainder) && (Remainder < (0x18 * PaletteElementSize)))
		{
			Temp = RGBAPalette[i];
			RGBAPalette[i] = RGBAPalette[i - (0x08 * PaletteElementSize)];
			RGBAPalette[i - (0x08 * PaletteElementSize)] = Temp;
		}
	}

	// Multiply/Divide color table elements
	for (ulong i = 0; i < RGBAPaletteSize; i++)
	{
		if (MulDiv == true)
			RGBAPalette[i] *= 2;
		else
			RGBAPalette[i] /= 2;
	}
}

bool ConvertBMPtoPHD(const char * FileName, bool Linear)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

	sBMPHeader BMPHeader;
	sPHDHeader PHDHeader;
	sPSIHeader PSIHeader;
	uchar MIPCount;

	uchar * RawBitmap;
	ulong RawBitmapSize;

	uchar * RGBAPalette;
	ulong RGBAPaletteSize;

	char OutFile[PATH_LEN];
	char TexName[64];

	// Open file
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read BMP header
	BMPHeader.UpdateFromFile(&ptrInputF);

	// Check BMP header
	if (BMPHeader.Check() == true)
	{
		printf("8-bit BMP \nParameters - Width: %i, Height: %i \n", BMPHeader.Width, BMPHeader.Height);

		// Prepare PSI palette
		RGBAPaletteSize = 0x400;
		RGBAPalette = (uchar *)malloc(RGBAPaletteSize);
		if (RGBAPalette == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RGBAPalette, sizeof(sBMPHeader), RGBAPaletteSize);
		PaletteSwapRedAndGreen(RGBAPalette, RGBAPaletteSize);

		// Prepare PSI bitmap
		RawBitmapSize = BMPHeader.Height * BMPHeader.Width;
		RawBitmap = (uchar *)malloc(RawBitmapSize);
		if (RawBitmap == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RawBitmap, sizeof(sBMPHeader) + RGBAPaletteSize, RawBitmapSize);
		FlipBitmap(&RawBitmap, &RawBitmapSize, BMPHeader.Width, BMPHeader.Height);

		// Fix palette
		ConvertDecalPalette(RGBAPalette, RGBAPaletteSize, false);
		PaletteFix(RGBAPalette, RGBAPaletteSize, false);

		// Resize bitmap to proper size
		uint OriginalWidth = BMPHeader.Width;
		uint OriginalHeight = BMPHeader.Height;
		BMPHeader.Update(PSIProperSize(OriginalWidth), PSIProperSize(OriginalHeight));
		ScaleBitmap(&RawBitmap, &RawBitmapSize, OriginalWidth, OriginalHeight, PSIProperSize(OriginalWidth), PSIProperSize(OriginalHeight), Linear);

		// Create MIPs
		MIPCount = CreateMIPs(&RawBitmap, &RawBitmapSize, BMPHeader.Width, BMPHeader.Height);

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PHD header
		PHDHeader.Update();
		FileWriteBlock(&ptrOutputF, &PHDHeader, sizeof(sPHDHeader));

		// Write PSI header
		FileGetName(FileName, TexName, sizeof(TexName), false);
		PSIHeader.Update(TexName, BMPHeader.Width, BMPHeader.Height, PSI_INDEXED, MIPCount);
		PSIHeader.UpdateUpscaleTaget(OriginalWidth, OriginalHeight);
		FileWriteBlock(&ptrOutputF, &PSIHeader, sizeof(sPSIHeader));

		// Write PSI data
		FileWriteBlock(&ptrOutputF, RGBAPalette, RGBAPaletteSize);
		FileWriteBlock(&ptrOutputF, RawBitmap, RawBitmapSize);

		// Free memory
		free(RGBAPalette);
		free(RawBitmap);

		// Close files
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else
	{
		puts("8 bit BMP reqired ...");
		return false;
	}

	// Close files
	fclose(ptrInputF);

	return true;
}

bool ConvertPHDtoBMP(const char * FileName, bool Linear)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

	sPHDHeader PHDHeader;
	sBMPHeader BMPHeader;
	sPSIHeader PSIHeader;

	uchar * RawBitmap;
	ulong RawBitmapSize;

	uchar * RGBAPalette;
	ulong RGBAPaletteSize;

	char OutFile[PATH_LEN];

	// Open PHD
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read and check PHD header
	PHDHeader.UpdateFromFile(&ptrInputF);
	if (PHDHeader.Check() != true)
	{
		puts("Invalid decal file ...");
		return false;
	}

	// Read PSI Header
	PSIHeader.UpdateFromFile(&ptrInputF, sizeof(sPHDHeader));

	// Check PSI Header
	if (PSIHeader.CheckType() == PSI_INDEXED)
	{
		printf("Parameters - Width: %i, Height: %i \n", PSIHeader.Width, PSIHeader.Height);

		// Prepare BMP palette
		RGBAPaletteSize = 0x400;
		RGBAPalette = (uchar *)malloc(RGBAPaletteSize);
		if (RGBAPalette == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RGBAPalette, sizeof(sPHDHeader) + sizeof(sPSIHeader), RGBAPaletteSize);
		PaletteFix(RGBAPalette, RGBAPaletteSize, true);
		PaletteSwapRedAndGreen(RGBAPalette, RGBAPaletteSize);

		// Prepare BMP bitmap
		RawBitmapSize = PSIHeader.Height * PSIHeader.Width;
		RawBitmap = (uchar *)malloc(RawBitmapSize);
		if (RawBitmap == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}
		FileReadBlock(&ptrInputF, RawBitmap, sizeof(sPHDHeader) + sizeof(sPSIHeader) + RGBAPaletteSize, RawBitmapSize);
		FlipBitmap(&RawBitmap, &RawBitmapSize, PSIHeader.Width, PSIHeader.Height);		

		// Fix palette
		ConvertDecalPalette(RGBAPalette, RGBAPaletteSize, true);

		// Resize bitmap to it's original size
		ScaleBitmap(&RawBitmap, &RawBitmapSize, PSIHeader.Width, PSIHeader.Height, PSIHeader.UpWidth, PSIHeader.UpHeight, Linear);

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".bmp");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write BMP header
		BMPHeader.Update(PSIHeader.UpWidth, PSIHeader.UpHeight);
		FileWriteBlock(&ptrOutputF, &BMPHeader, sizeof(sBMPHeader));

		// Write BMP palette and bitmap
		FileWriteBlock(&ptrOutputF, RGBAPalette, RGBAPaletteSize);
		FileWriteBlock(&ptrOutputF, RawBitmap, RawBitmapSize);

		// Free memory
		free(RawBitmap);
		free(RGBAPalette);

		// Close file
		fclose(ptrOutputF);

		puts("Done\n\n");
	}
	else
	{
		puts("Can't recognise decal");
		return false;
	}

	// Close file
	fclose(ptrInputF);

	return true;
}

void FlipBitmap(uchar ** Bitmap, ulong * BitmapSize, uint Width, uint Height)		// Flip bitmap vertically. Needed for conversion to\from BMP
{
	uchar * NewBitmap;

	// Allocate memory for new bitmap
	NewBitmap = (uchar *)malloc(Width * Height);
	if (NewBitmap == NULL)
	{
		puts("Unable to allocate memory!");
		UTIL_WAIT_KEY;
		exit(EXIT_FAILURE);
	}

	// Copy flipped bitmap to new place
	for (uint y = 0; y < Height; y++)
		for (uint x = 0; x < Width; x++)
			NewBitmap[(Width * y) + x] = (*Bitmap)[(Width * ((Height - 1) - y)) + x];

	// Destroy old bitmap
	free(*Bitmap);

	// Save pointer to new bitmap
	*Bitmap = NewBitmap;
}

void ConvertDecalPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool ToBMP)
{
	// Get decal color from the last entry in palette
	uchar PaletteElementSize = 4;
	int LastBase = (EIGHT_BIT_PALETTE_ELEMENTS_COUNT - 1) * PaletteElementSize;
	uchar R = RGBAPalette[LastBase + 0];
	uchar G = RGBAPalette[LastBase + 1];
	uchar B = RGBAPalette[LastBase + 2];

	if (ToBMP == true)
	{
		// Conversion to BMP:
		//	entry #0   contains white color (transparent)
		//	... color darkens
		//	entry #254 contains black color (non-transparent)
		//	entry #255 contains decal color

		// Rewrite all entries except the last one
		uchar Color;
		for (uint i = 0, Color = 0xFF; i < (EIGHT_BIT_PALETTE_ELEMENTS_COUNT-1); i++, Color--)
		{
			RGBAPalette[i * PaletteElementSize + 0] = Color;
			RGBAPalette[i * PaletteElementSize + 1] = Color;
			RGBAPalette[i * PaletteElementSize + 2] = Color;
			RGBAPalette[i * PaletteElementSize + 3] = 0x00;
		}

		// Fix the last one
		RGBAPalette[LastBase + 3] = 0x00;
	}
	else
	{
		// Conversion to PHD:
		//	entry #0   contains min alpha (transparent)
		//	... alpha grows
		//	entry #255 contains max alpha (non-transparent)
		//	every entry contains the same color

		// Rewrite all entries except the last one
		for (uint i = 0; i < (EIGHT_BIT_PALETTE_ELEMENTS_COUNT-1); i++)
		{
			RGBAPalette[i * PaletteElementSize + 0] = R;
			RGBAPalette[i * PaletteElementSize + 1] = G;
			RGBAPalette[i * PaletteElementSize + 2] = B;
			RGBAPalette[i * PaletteElementSize + 3] = i;
		}

		// Fix the last one
		RGBAPalette[LastBase + 3] = 0xFF;
	}
}

void PaletteSwapRedAndGreen(uchar * RGBAPalette, ulong RGBAPaletteSize)
{
	uchar Temp;

	uchar PaletteElementSize = 4;
	for (uint Element = 0; Element < EIGHT_BIT_PALETTE_ELEMENTS_COUNT; Element++)
	{
		// Swap 1-st and 3-rd bytes in color table element
		Temp = RGBAPalette[Element * PaletteElementSize];
		RGBAPalette[Element * PaletteElementSize] = RGBAPalette[Element * PaletteElementSize + 2];
		RGBAPalette[Element * PaletteElementSize + 2] = Temp;
	}
}

int main(int argc, char * argv[])
{
	char Extension[5];

	// Output info
	printf("PS2 HL decal (PHD) tool v%s \n", PROG_VERSION);

	// Check arguments
	if (argc == 1)
	{
		// No arguments - show help screen
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2017, Alexey Leushin. All rights reserved.");
		puts("Zlib library is used within this program to perform DEFLATE\\INFLATE operations.\n");
		puts("How to use: \n1) Windows explorer - drag and drop PS2 HL Decal or PNG file on phdtool.exe \n2) Command line/Batch - phdtool [file_name]  \nOptional: convert to PNG - phdtool topng [phd_file_name] \n");
		puts("\nFor more info read ReadMe.txt \n");
		puts("Press any key to exit ...");
		UTIL_WAIT_KEY;
	}
	else if (argc == 2)
	{
		FileGetExtension(argv[1], Extension, sizeof(Extension));

		printf("Processing file: %s \n", argv[1]);
		if (!strcmp(Extension, ".png"))				// Convert PNG to PS2 HL Decal
		{
			if (ConvertPNGtoPHD(argv[1]) == true)
				return 0;
			else
				puts("Can't convert decal ... \n");
		}
		else if (!strcmp(Extension, ".bmp"))		// Convert BMP to PS2 HL Decal
		{
			if (ConvertBMPtoPHD(argv[1], true) == true)
				return 0;
			else
				puts("Can't convert decal ... \n");
		}
		else										// Convert PS2 HL Decal to BMP
		{
			// Convert PS2 decals to bmp by default
			if (ConvertPHDtoBMP(argv[1], true) == true)
				return 0;
			else
				puts("Can't convert decal ... \n");
		}
	}
	else if (argc == 3)
	{
		if (!strcmp(argv[1], "topng"))
		{
			printf("Processing file: %s \n", argv[2]);
			if (ConvertPHDtoPNG(argv[2]) == true)				// Convert PS2 HL Decal to PNG
				return 0;
			else
				puts("Can't convert decal ... \n");
		}
		else
		{
			puts("Wrong arguments ... \n");
		}
	}
	else
	{
		puts("Too many arguments ... \n");
	}
	
	return 1;
}
