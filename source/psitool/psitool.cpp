// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

// 
// This file contains image conversion functions
// 

////////// Includes //////////
#include "util.h"
#include "main.h"

////////// Functions //////////
bool ConvertPNGtoPSI(const char * FileName);
bool ConvertPSItoPNG(const char * FileName);
void PatchRGBAPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool MulDiv);
void WierdRGBAPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool MulDiv);

bool ConvertPNGtoPSI(const char * FileName)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

	sPNGHeader PNGHeader;
	sPSIHeader PSIHeader;

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
	if (PNGHeader.CheckType() == PNG_RGBA || PNGHeader.CheckType() == PNG_RGB)
	{
		if (PNGHeader.CheckType() == PNG_RGBA)
		{
			UTIL_MSG("32-bit PNG \nParameters - Width: %i, Height: %i \n", PNGHeader.Width, PNGHeader.Height);
			BytesPerPixel = 4;
		}
		else
		{
			UTIL_MSG("24-bit PNG \nParameters - Width: %i, Height: %i \n", PNGHeader.Width, PNGHeader.Height);
			BytesPerPixel = 3;
		}


		// Prepare PSI data
		PNGBitmap = PNGReadBitmap(&ptrInputF, PNGHeader.Width, PNGHeader.Height, BytesPerPixel, PNGHeader.BitDepth);
		for (ulong i = 0; i < PNGBitmap->DataSize; i++)			// Divide PNG bitmap bytes by 2 to match PSI
			PNGBitmap->Data[i] /= 2;

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".psi");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PSI header
		FileGetName(FileName, TexName, sizeof(TexName), false);
		PSIHeader.Update(TexName, PNGHeader.Width, PNGHeader.Height, PSI_RGBA);
		FileWriteBlock(&ptrOutputF, &PSIHeader, sizeof(sPSIHeader));

		// Write PSI data
		FileWriteBlock(&ptrOutputF, PNGBitmap->Data, PNGBitmap->DataSize);

		// Free memory
		free(PNGBitmap->Data);

		// Close files
		fclose(ptrOutputF);

		UTIL_MSG("Done\n\n");
	}
	else if (PNGHeader.CheckType() == PNG_INDEXED)
	{
		UTIL_MSG("8-bit PNG \nParameters - Width: %i, Height: %i \n", PNGHeader.Width, PNGHeader.Height);
		BytesPerPixel = 1;

		// Prepare PSI palette
		PNGPalette = PNGReadPalette(&ptrInputF);
		PatchRGBAPalette(PNGPalette->Data, PNGPalette->DataSize, false);

		// Prepare PSI bitmap
		PNGBitmap = PNGReadBitmap(&ptrInputF, PNGHeader.Width, PNGHeader.Height, BytesPerPixel, PNGHeader.BitDepth);

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".psi");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PSI header
		FileGetName(FileName, TexName, sizeof(TexName), false);
		PSIHeader.Update(TexName, PNGHeader.Width, PNGHeader.Height, PSI_INDEXED);
		FileWriteBlock(&ptrOutputF, &PSIHeader, sizeof(sPSIHeader));

		// Write PSI data
		FileWriteBlock(&ptrOutputF, PNGPalette->Data, PNGPalette->DataSize);
		FileWriteBlock(&ptrOutputF, PNGBitmap->Data, PNGBitmap->DataSize);

		// Free memory
		free(PNGBitmap->Data);
		free(PNGPalette->Data);

		// Close files
		fclose(ptrOutputF);

		UTIL_MSG("Done\n\n");
	}
	else
	{
		UTIL_ERR("Unsupported PNG ...\n", return false);
	}

	// Close files
	fclose(ptrInputF);

	return true;
}

bool ConvertPSItoPNG(const char * FileName)
{
	FILE *ptrInputF;						// Input file
	FILE *ptrOutputF;						// Output file

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

	// Open PSI
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read PSI Header
	PSIHeader.UpdateFromFile(&ptrInputF, 0);

	// Check PSI Header
	if (PSIHeader.CheckType() == PSI_RGBA)
	{
		UTIL_MSG("32-bit PSI image \nParameters - Width: %i, Height: %i \n", PSIHeader.Width1, PSIHeader.Height1);
		BytesPerPixel = 4;

		// Prepare PNG data
		RawBitmapSize = PSIHeader.Height1 * PSIHeader.Width1 * BytesPerPixel;
		UTIL_MALLOC(uchar*, RawBitmap, RawBitmapSize, exit(EXIT_FAILURE));
		FileReadBlock(&ptrInputF, RawBitmap, sizeof(sPSIHeader), RawBitmapSize);
		for (ulong i = 0; i < RawBitmapSize; i++)		// Multiply bitmap bytes by 2
			RawBitmap[i] *= 2;
		PNGBitmap.Data = RawBitmap;
		PNGBitmap.DataSize = RawBitmapSize;

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".png");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PNG header
		PNGHeader.Update(PSIHeader.Width1, PSIHeader.Height1, PNG_RGBA);
		PNGHeader.SwapEndian();
		FileWriteBlock(&ptrOutputF, &PNGHeader, sizeof(sPNGHeader));

		// Write PNG Data
		PNGWriteChunk(&ptrOutputF, "iTXt", "Comment\0\0\0\0\0Converted with PS2 Half-life PSI tool", strlen("CommentConverted with PS2 Half-life PSI tool") + 5);
		PNGWriteBitmap(&ptrOutputF, PSIHeader.Width1, PSIHeader.Height1, BytesPerPixel, &PNGBitmap);
		PNGWriteChunk(&ptrOutputF, "IEND", NULL, NULL);

		// Free memory
		free(PNGBitmap.Data);

		// Close file
		fclose(ptrOutputF);

		UTIL_MSG("Done\n\n");
	}
	else if (PSIHeader.CheckType() == PSI_INDEXED)
	{
		UTIL_MSG("8-bit PSI image \nParameters - Width: %i, Height: %i \n", PSIHeader.Width1, PSIHeader.Height1);
		BytesPerPixel = 1;

		// Prepare PNG palette
		RGBAPaletteSize = 0x400;
		UTIL_MALLOC(uchar*, RGBAPalette, RGBAPaletteSize, exit(EXIT_FAILURE));
		FileReadBlock(&ptrInputF, RGBAPalette, sizeof(sPSIHeader), RGBAPaletteSize);
		PatchRGBAPalette(RGBAPalette, RGBAPaletteSize, true);
		PNGPalette.Data = RGBAPalette;
		PNGPalette.DataSize = RGBAPaletteSize;

		// Prepare PNG bitmap
		RawBitmapSize = PSIHeader.Height1 * PSIHeader.Width1 * BytesPerPixel;
		UTIL_MALLOC(uchar*, RawBitmap, RawBitmapSize, exit(EXIT_FAILURE));
		FileReadBlock(&ptrInputF, RawBitmap, sizeof(sPSIHeader) + RGBAPaletteSize, RawBitmapSize);
		PNGBitmap.Data = RawBitmap;
		PNGBitmap.DataSize = RawBitmapSize;

		// Create output file
		FileGetFullName(FileName, OutFile, sizeof(OutFile));
		strcat(OutFile, ".png");
		SafeFileOpen(&ptrOutputF, OutFile, "wb");

		// Write PNG header
		PNGHeader.Update(PSIHeader.Width1, PSIHeader.Height1, PNG_INDEXED);
		PNGHeader.SwapEndian();
		FileWriteBlock(&ptrOutputF, &PNGHeader, sizeof(sPNGHeader));

		// Write PNG data
		PNGWriteChunk(&ptrOutputF, "iTXt", "Comment\0\0\0\0\0Converted with PS2 Half-life PSI tool", strlen("CommentConverted with PS2 Half-life PSI tool") + 5);
		PNGWritePalette(&ptrOutputF, &PNGPalette);
		PNGWriteBitmap(&ptrOutputF, PSIHeader.Width1, PSIHeader.Height1, BytesPerPixel, &PNGBitmap);
		PNGWriteChunk(&ptrOutputF, "IEND", NULL, NULL);

		// Free memory
		free(PNGBitmap.Data);
		free(RGBAPalette);

		// Close file
		fclose(ptrOutputF);

		UTIL_MSG("Done\n\n");
	}
	else
	{
		UTIL_ERR("Unknown image type\n", return false);
	}

	// Close file
	fclose(ptrInputF);

	return true;
}

void PatchRGBAPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool MulDiv)			// Patch color table.
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
		if ((i + 1) % PaletteElementSize == 0)	// Alpha
		{
			if (MulDiv == true)
			{
				if (RGBAPalette[i] > 0x7F)
					RGBAPalette[i] = 0x7F;

				RGBAPalette[i] *= 2;
			}
			else
			{
				RGBAPalette[i] /= 2;

				if (RGBAPalette[i] == 0x7F)
					RGBAPalette[i] = 0x80;
			}

			// Black out transparent pixels to avoid possible artifacts
			if (RGBAPalette[i] == 0x00)
			{
				RGBAPalette[i - 1] = 0;
				RGBAPalette[i - 2] = 0;
				RGBAPalette[i - 3] = 0;
			}
		}
		else									// RGB
		{
			if (MulDiv == true)
				RGBAPalette[i] *= 2;
			else
				RGBAPalette[i] /= 2;
		}
	}
}

// Palette fix for alphafont.psf
void WierdRGBAPalette(uchar * RGBAPalette, ulong RGBAPaletteSize, bool MulDiv)
{
	uint PaletteElementSize = 4;

	puts("Fixing palette for alphafont.psf...");
	for (ulong i = 0; i < RGBAPaletteSize; i++)
	{
		if ((i + 1) % PaletteElementSize == 0)
			continue;	// Skip alpha

		RGBAPalette[i] = 0x80;
	}
}

bool ConvertPSFtoPNG(const char * FileName)
{
	FILE *ptrInputF;						// Input font
	FILE *ptrOutputPSI;						// Output .psi
	FILE *ptrOutputINF;						// Output .inf

	sPNGHeader PNGHeader;
	sPSFHeader PSFHeader;

	sPNGData PNGPalette;
	sPNGData PNGBitmap;
	uchar BytesPerPixel;

	char OutFile[PATH_LEN];

	uchar * RawBitmap;
	ulong RawBitmapSize;

	uchar * RGBAPalette;
	ulong RGBAPaletteSize;

	// Open PSF
	SafeFileOpen(&ptrInputF, FileName, "rb");

	// Read and check PSF Header
	if (!PSFHeader.UpdateFromFile(&ptrInputF))
		UTIL_ERR("Bad font!", return false);

	UTIL_MSG("Font with %dx%d bitmap \n", PSF_BMP_W, PSF_BMP_H);
	BytesPerPixel = 1;

	// Prepare PNG palette
	RGBAPaletteSize = 0x400;
	UTIL_MALLOC(uchar*, RGBAPalette, RGBAPaletteSize, exit(EXIT_FAILURE));
	FileReadBlock(&ptrInputF, RGBAPalette, sizeof(sPSFHeader) + PSF_BMP_SZ, RGBAPaletteSize);
	PatchRGBAPalette(RGBAPalette, RGBAPaletteSize, true);
	PNGPalette.Data = RGBAPalette;
	PNGPalette.DataSize = RGBAPaletteSize;

	// Prepare PNG bitmap
	RawBitmapSize = PSF_BMP_H * PSF_BMP_W * BytesPerPixel;
	UTIL_MALLOC(uchar*, RawBitmap, RawBitmapSize, exit(EXIT_FAILURE));
	FileReadBlock(&ptrInputF, RawBitmap, sizeof(sPSFHeader), RawBitmapSize);
	PNGBitmap.Data = RawBitmap;
	PNGBitmap.DataSize = RawBitmapSize;

	// Create output .psi
	FileGetFullName(FileName, OutFile, sizeof(OutFile));
	strcat(OutFile, ".png");
	SafeFileOpen(&ptrOutputPSI, OutFile, "wb");

	// Write PNG header
	PNGHeader.Update(PSF_BMP_W, PSF_BMP_H, PNG_INDEXED);
	PNGHeader.SwapEndian();
	FileWriteBlock(&ptrOutputPSI, &PNGHeader, sizeof(sPNGHeader));

	// Write PNG data
	PNGWriteChunk(&ptrOutputPSI, "iTXt", "Comment\0\0\0\0\0Converted with PS2 Half-life PSI tool", strlen("CommentConverted with PS2 Half-life PSI tool") + 5);
	PNGWritePalette(&ptrOutputPSI, &PNGPalette);
	PNGWriteBitmap(&ptrOutputPSI, PSF_BMP_W, PSF_BMP_H, BytesPerPixel, &PNGBitmap);
	PNGWriteChunk(&ptrOutputPSI, "IEND", NULL, NULL);

	// Free memory
	free(PNGBitmap.Data);
	free(RGBAPalette);

	// Close output .psi
	fclose(ptrOutputPSI);

	// Create output .inf
	FileGetFullName(FileName, OutFile, sizeof(OutFile));
	strcat(OutFile, ".inf");
	SafeFileOpen(&ptrOutputINF, OutFile, "w");

	// Write symbol data
	PSFHeader.WriteSymbols(&ptrOutputINF);

	// Close output .inf
	fclose(ptrOutputINF);

	// Close input file
	fclose(ptrInputF);

	UTIL_MSG("Done\n\n");

	return true;
}

bool ConvertPNGtoPSF(const char * FileName)
{
	FILE *ptrInputINF;						// Input .inf
	FILE *ptrInputPNG;						// Input .png
	FILE *ptrOutputF;						// Output .psi font

	sPNGHeader PNGHeader;
	sPSFHeader PSFHeader;

	sPNGData * PNGPalette;
	sPNGData * PNGBitmap;
	uchar BytesPerPixel;

	char NameBuf[PATH_LEN];

	// Open .inf
	SafeFileOpen(&ptrInputINF, FileName, "r");

	// Open .png
	FileGetFullName(FileName, NameBuf, sizeof(NameBuf));
	strcat(NameBuf, ".png");
	SafeFileOpen(&ptrInputPNG, NameBuf, "rb");

	// Read PNG header
	PNGHeader.UpdateFromFile(&ptrInputPNG);
	PNGHeader.SwapEndian();

	// Check PNG header
	if (PNGHeader.CheckType() != PNG_INDEXED)
		UTIL_ERR("Indexed PNG required...", return false);
	if (PNGHeader.Width != PSF_BMP_W || PNGHeader.Height != PSF_BMP_H)
		UTIL_ERR("Bad PNG size...", return false);

	UTIL_MSG("Converting font...\n");

	// Prepare palette
	PNGPalette = PNGReadPalette(&ptrInputPNG);
	PatchRGBAPalette(PNGPalette->Data, PNGPalette->DataSize, false);
	if (strstr(FileName, "alphafont"))
		WierdRGBAPalette(PNGPalette->Data, PNGPalette->DataSize, false);

	// Prepare bitmap
	BytesPerPixel = 1;
	PNGBitmap = PNGReadBitmap(&ptrInputPNG, PNGHeader.Width, PNGHeader.Height, BytesPerPixel, PNGHeader.BitDepth);

	// Create output file
	FileGetFullName(FileName, NameBuf, sizeof(NameBuf));
	strcat(NameBuf, ".psf");
	SafeFileOpen(&ptrOutputF, NameBuf, "wb");

	// Write PSF header
	PSFHeader.ReadSymbols(&ptrInputINF);
	FileWriteBlock(&ptrOutputF, &PSFHeader, sizeof(sPSFHeader));

	// Write PSF bitmap and palette
	FileWriteBlock(&ptrOutputF, PNGBitmap->Data, PNGBitmap->DataSize);
	FileWriteBlock(&ptrOutputF, PNGPalette->Data, PNGPalette->DataSize);

	// Free memory
	free(PNGBitmap->Data);
	free(PNGPalette->Data);

	// Close files
	fclose(ptrInputPNG);
	fclose(ptrInputINF);
	fclose(ptrOutputF);

	UTIL_MSG("Done\n\n");

	return true;
}

int main(int argc, char * argv[])
{
	char Extension[5];

	// Output info
	UTIL_MSG(PROG_TITLE);

	// Check arguments
	if (argc == 1)
	{
		// No arguments - show help screen
		UTIL_MSG(PROG_INFO);
		UTIL_WAIT_KEY("Press any key to exit ...");
	}
	else if (argc == 2)
	{
		FileGetExtension(argv[1], Extension, sizeof(Extension));

		UTIL_MSG("Processing file: %s \n", argv[1]);
		if (!strcmp(Extension, ".png"))				// Convert PNG to PSI
		{
			if (ConvertPNGtoPSI(argv[1]) == true)
				return 0;
			else
				UTIL_MSG_ERR("Can't convert image ... \n");
		}
		else if (!strcmp(Extension, ".psi"))		// Convert PSI to PNG
		{
			if (ConvertPSItoPNG(argv[1]) == true)
				return 0;
			else
				UTIL_MSG_ERR("Can't convert image ... \n");
		}
		else if (!strcmp(Extension, ".psf"))		// PSF font
		{
			if (ConvertPSFtoPNG(argv[1]) == true)
				return 0;
			else
				UTIL_MSG_ERR("Can't convert font ... \n");
		}
		else if (!strcmp(Extension, ".inf"))		// INF+PNG to PSF font
		{
			if (ConvertPNGtoPSF(argv[1]) == true)
				return 0;
			else
				UTIL_MSG_ERR("Can't convert font ... \n");
		}
		else
		{
			UTIL_MSG_ERR("Wrong file extension ... \n");
		}
	}
	else
	{
		UTIL_MSG_ERR("Too many arguments ... \n");
	}
	
	return 1;
}
