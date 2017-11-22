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
// This file contains functions that perporm various operations with PNG files
//
// Zlib library is used within this module to perform DEFLATE\INFLATE operations
//
// Useful links about PNG format:
// https://medium.com/@duhroach/how-png-works-f1174e3cc7b7
// http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
// http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html

////////// Includes //////////
#include "main.h"

sPNGData * PNGReadChunk(FILE ** ptrFile, const char * Marker)		// Markers: "IHDR", "PLTE", "tRNS", "IDAT", "IEND"
{
	uchar * FileData;
	ulong FileDataSize;

	sPNGData * PNGData;			// This pointer would be returned by function
	uchar * FinalData = NULL;	// Data from all chunks with corresponding marker
	ulong FinalDataSize = 0;	// Size of data from all chunks with corresponding marker

								// Check marker size
	if (strlen(Marker) < 4)
		return NULL;

	// Allocate memory for file data and PNG data
	FileDataSize = FileSize(ptrFile);
	FileData = (uchar *)malloc(FileDataSize);
	FileReadBlock(ptrFile, FileData, 0, FileDataSize);
	PNGData = (sPNGData *)malloc(sizeof(sPNGData));
	if (FileData == NULL || PNGData == NULL)
	{
		puts("Unable to allocate memory! \n");
		exit(EXIT_FAILURE);
	}

	// Accumulate data from all chunks with corresonding markers
	ulong ChunkCounder = 0;
	for (ulong i = 0; i < FileDataSize - 4; i++)
	{
		// Look for marker
		if (FileData[i] == Marker[0])
			if (FileData[i + 1] == Marker[1] && FileData[i + 2] == Marker[2] && FileData[i + 3] == Marker[3])
			{
				uchar * TempBuff = NULL;		// Temporary storage
				ulong TempBuffSize = 0;			// Temporary storage size

				uchar * ChunkData = NULL;		// Data from current chunk
				ulong ChunkDataSize = 0;		// Size of current chunk

												// Check chunk size
				memcpy(&ChunkDataSize, &FileData[i - 4], sizeof(ChunkDataSize));
				ChunkDataSize = _byteswap_ulong(ChunkDataSize);						// Swap endian

																					// Allocate memory for data from this chunk
				ChunkData = (uchar *)malloc(ChunkDataSize);
				if (ChunkData == NULL)
				{
					puts("Unable to allocate memory! \n");
					exit(EXIT_FAILURE);
				}

				// Accumulate data from this chunk
				memcpy(ChunkData, &FileData[i + 4], ChunkDataSize);

				// Copy data from old FinalData to Temp and then destroy it
				if (FinalData != NULL && FinalDataSize != 0)
				{
					TempBuffSize = FinalDataSize;
					TempBuff = (uchar *)malloc(TempBuffSize);
					memcpy(TempBuff, FinalData, FinalDataSize);

					free(FinalData);
				}

				// Allocate more memory to FinalData
				FinalDataSize = TempBuffSize + ChunkDataSize;
				FinalData = (uchar *)malloc(FinalDataSize);
				if (FinalData == NULL)
				{
					puts("Unable to allocate memory! \n");
					exit(EXIT_FAILURE);
				}

				// Restore FinalData from Temp and copy new chunk to it
				memcpy(FinalData, TempBuff, TempBuffSize);
				memcpy(FinalData + TempBuffSize, ChunkData, ChunkDataSize);

				// Free memory
				free(ChunkData);
				free(TempBuff);

				ChunkCounder++;
			}
	}
	printf("Found %i %s chunk(s) \n", ChunkCounder, Marker);

	// Free memory
	free(FileData);

	// Prepare structure
	PNGData->DataSize = FinalDataSize;
	PNGData->Data = FinalData;

	// Return pointer to structure
	return PNGData;
}

void PNGWriteChunk(FILE ** ptrFile, const char * Marker, sPNGData * Chunk)	// Markers: "IHDR", "PLTE", "tRNS", "IDAT", "IEND"
{
	ulong CRC, ChunkSize;

	// Check marker size
	if (strlen(Marker) < 4)
		return;

	// Calculate CRC
	CRC = crc32(NULL, (const Bytef *)Marker, 4);
	if (Chunk->DataSize != 0)
		CRC = crc32(CRC, Chunk->Data, Chunk->DataSize);
	CRC = _byteswap_ulong(CRC);

	// Write chunk size
	ChunkSize = Chunk->DataSize;
	ChunkSize = _byteswap_ulong(ChunkSize);
	FileWriteBlock(ptrFile, &ChunkSize, sizeof(ChunkSize));

	// Write chunk marker
	FileWriteBlock(ptrFile, (void *)Marker, 4);

	// Write chunk data
	FileWriteBlock(ptrFile, Chunk->Data, Chunk->DataSize);

	// Write CRC
	FileWriteBlock(ptrFile, &CRC, sizeof(CRC));
}

void PNGWriteChunk(FILE ** ptrFile, const char * Marker, void * Data, ulong DataSize)	// Markers: "IHDR", "PLTE", "tRNS", "IDAT", "IEND"
{
	ulong CRC, ChunkSize;

	// Check marker size
	if (strlen(Marker) < 4)
		return;

	// Calculate CRC
	CRC = crc32(NULL, (const Bytef *)Marker, 4);
	if (DataSize != 0)
		CRC = crc32(CRC, (const Bytef *)Data, DataSize);
	CRC = _byteswap_ulong(CRC);

	// Write chunk size
	ChunkSize = DataSize;
	ChunkSize = _byteswap_ulong(ChunkSize);
	FileWriteBlock(ptrFile, &ChunkSize, sizeof(ChunkSize));

	// Write chunk marker
	FileWriteBlock(ptrFile, (void *)Marker, 4);

	// Write chunk data
	FileWriteBlock(ptrFile, Data, DataSize);

	// Write CRC
	FileWriteBlock(ptrFile, &CRC, sizeof(CRC));
}

uchar PNGGetByteFromRow(uchar * Row, uint PixelNumber, uint BitDepth)
{
	uchar Byte;
	uint ByteNumber;
	uint BitNumber;

	// Calculate byte and bit number
	ByteNumber = (PixelNumber * BitDepth) / 8;
	BitNumber = (PixelNumber * BitDepth) % 8;

	// Get data
	Byte = Row[ByteNumber];
	Byte = Byte << BitNumber;
	Byte = Byte >> (8 - BitDepth);

	return Byte;
}

bool PNGUnfilter(sPNGData * InData, uint Height, uint Width, uint BytesPerPixel, uint BitDepth)		// Reverse filtering effects (get raw data)
{
	uchar * RawData;
	ulong RawDataSize;

	// Allocate memory for decoded bitmap
	RawDataSize = Width * Height * BytesPerPixel;
	RawData = (uchar *)malloc(RawDataSize);
	if (RawData == NULL)
	{
		puts("Unable to allocate memory! \n");
		exit(EXIT_FAILURE);
	}

	uint RowLength = ceil((double)Width * (double)BytesPerPixel * (double)BitDepth / 8.0);	// Row length of original bitmap
	uint NewRowLength = Width * BytesPerPixel;												// Row length of output bitmap
	uchar FilterType;		// Filter type
	uchar Current;			// Current pixel
	uchar Previous;			// Previous (left) pixel
	uchar Upper;			// Upper pixel
	uchar UpperPrevious;	// Upper previous pixel
	uchar Paeth;			// Paeth predicted pixel
	for (ulong Row = 0; Row < Height; Row++)
	{
		FilterType = InData->Data[Row * (RowLength + 1)];	// Get filter type from 1-st byte of pixel row
		if (FilterType > 4)									// Return flase if unsupported fiter is detected 
			return false;
		if (BitDepth < 8 && FilterType != 0)				// No support for filtering with bit depth < 8
			return false;

		for (int i = 0; i < NewRowLength; i++)
		{
			// Calculate values for all filters
			Current = PNGGetByteFromRow(InData->Data + (Row * (RowLength + 1) + 1), i, BitDepth);
			if (i < BytesPerPixel)
			{
				Previous = 0;
			}
			else
			{
				Previous = RawData[Row * NewRowLength + i - BytesPerPixel];
			}
			if (Row == 0)
			{
				Upper = 0;
				UpperPrevious = 0;
			}
			else
			{
				Upper = RawData[(Row - 1) * NewRowLength + i];
				if (i < BytesPerPixel)
				{
					UpperPrevious = 0;
				}
				else
				{
					UpperPrevious = RawData[(Row - 1) * NewRowLength + i - BytesPerPixel];
				}
			}
			Paeth = PaethPredictor(Previous, Upper, UpperPrevious);

			// Unfilter image
			switch (FilterType)		// Filters are applied per pixels for each channel individually (R, G, B, A)
			{
			case 0:			// No filter
				RawData[Row * NewRowLength + i] = Current;
				break;
			case 1:			// SUB filter (Difference between current and previous pixels)
				RawData[Row * NewRowLength + i] = Current + Previous;
				break;
			case 2:			// UP filter (Difference between current and upper pixels)
				RawData[Row * NewRowLength + i] = Current + Upper;
				break;
			case 3:			// AVG filter (Difference between current pixel and average of prevoius and upper pixels)
				RawData[Row * NewRowLength + i] = Current + floor(((double)Previous + (double)Upper) / 2);
				break;
			case 4:			// Paeth filter (Difference between current pixel and predicted pixel. Predicted pixel can be previous, upper, or upper previous pixel. Criteria of prediction - lowest delta with current pixel.)
				RawData[Row * NewRowLength + i] = Current + Paeth;
				break;
			}
		}
	}

	// Destroy old data
	free(InData->Data);

	// Update input structure
	InData->Data = RawData;
	InData->DataSize = RawDataSize;

	return true;
}

bool PNGFilter(sPNGData * InData, uint Height, uint Width, uint BytesPerPixel, uchar FilterType)		// Apply filter to raw data
{
	uchar * FiltData;
	ulong FiltDataSize;

	// Check filter type
	if (FilterType > 4)
		return false;

	// Allocate memory for filtered bitmap
	FiltDataSize = Width * Height * BytesPerPixel + Height;
	FiltData = (uchar *)malloc(FiltDataSize);
	if (FiltData == NULL)
	{
		puts("Unable to allocate memory! \n");
		exit(EXIT_FAILURE);
	}

	uint RowLength = Width * BytesPerPixel;
	uchar Current;
	uchar Previous;
	uchar Upper;
	uchar UpperPrevious;
	uchar Paeth;
	for (ulong Row = 0; Row < Height; Row++)
	{
		FiltData[Row * (RowLength + 1)] = FilterType;		// Set filter type in first byte of row

		for (int i = 0; i < RowLength; i++)
		{
			// Calculate values for all filters
			Current = InData->Data[Row * RowLength + i];
			if (i < BytesPerPixel)
			{
				Previous = 0;
			}
			else
			{
				Previous = InData->Data[Row * RowLength + i - BytesPerPixel];
			}
			if (Row == 0)
			{
				Upper = 0;
				UpperPrevious = 0;
			}
			else
			{
				Upper = InData->Data[(Row - 1) * RowLength + i];
				if (i < BytesPerPixel)
				{
					UpperPrevious = 0;
				}
				else
				{
					UpperPrevious = InData->Data[(Row - 1) * RowLength + i - BytesPerPixel];
				}
			}
			Paeth = PaethPredictor(Previous, Upper, UpperPrevious);

			// Unfilter image
			switch (FilterType)		// Filters are applied per pixels for each channel separately (R, G, B, A)
			{
			case 0:			// No filter
				FiltData[Row * (RowLength + 1) + (i + 1)] = Current;
				break;
			case 1:			// SUB filter (Difference between current and previous pixels)
				FiltData[Row * (RowLength + 1) + (i + 1)] = Current - Previous;
				break;
			case 2:			// UP filter (Difference between current and upper pixels)
				FiltData[Row * (RowLength + 1) + (i + 1)] = Current - Upper;
				break;
			case 3:			// AVG filter (Difference between current pixel and average of prevoius and upper pixels)
				FiltData[Row * (RowLength + 1) + (i + 1)] = Current - floor(((double)Previous + (double)Upper) / 2);
				break;
			case 4:			// Paeth filter (Difference between current pixel and predicted pixel. Predicted pixel can be previous, upper, or upper previous pixel. Criteria of prediction - lowest delta with current pixel.)
				FiltData[Row * (RowLength + 1) + (i + 1)] = Current - Paeth;
				break;
			}
		}
	}

	// Destroy old data
	free(InData->Data);

	// Update input structure
	InData->Data = FiltData;
	InData->DataSize = FiltDataSize;

	return true;
}

bool PNGDecompress(sPNGData * InData)
{
	uchar * DData;
	ulong DDataSize;

	// Decompress image
	if (ZDecompress(InData->Data, InData->DataSize, &DData, &DDataSize, InData->DataSize) == false)
		return false;

	// Destroy old data
	free(InData->Data);

	// Update input structure
	InData->Data = DData;
	InData->DataSize = DDataSize;

	return true;
}

bool PNGCompress(sPNGData * InData)
{
	uchar * CData;
	ulong CDataSize;

	// Decompress image
	if (ZCompress(InData->Data, InData->DataSize, &CData, &CDataSize) == false)
		return false;

	// Destroy old data
	free(InData->Data);

	// Update input structure
	InData->Data = CData;
	InData->DataSize = CDataSize;

	return true;
}

int PaethPredictor(int a, int b, int c)
{
	int p, pa, pb, pc;
	// a = left, b = above, c = upper left
	p = a + b - c;		// initial estimate
	pa = abs(p - a);	// distances to a, b, c
	pb = abs(p - b);
	pc = abs(p - c);
	// return nearest of a, b, c,
	// breaking ties in order a, b, c.
	if ((pa <= pb) && (pa <= pc))
	{
		return a;
	}
	else if (pb <= pc)
	{
		return b;
	}
	else
	{
		return c;
	}
}

sPNGData * PNGReadPalette(FILE ** ptrFile)
{
	sPNGData * RGBPalette;
	sPNGData * Alpha;

	uchar * FullRGBPalette;
	uchar * FullAlpha;
	sPNGData * RGBAPalette;

	// Read palette
	RGBPalette = PNGReadChunk(ptrFile, "PLTE");

	// Check if palette is not present
	if (RGBPalette->Data == NULL)
	{
		puts("Corrupted file: palette chunk is not present ... \n");
		exit(EXIT_FAILURE);
	}
	else
	{
		// Check if palette is cut
		if (RGBPalette->DataSize < 0x300)
		{
			puts("Palette is cut, restoring ...");

			// Allocate memory for full RGB palette
			FullRGBPalette = (uchar *)malloc(0x300);
			if (FullRGBPalette == NULL)
			{
				puts("Unable to allocate memory! \n");
				exit(EXIT_FAILURE);
			}

			// Copy cut palette to full
			memset(FullRGBPalette, 0x00, 0x300);
			memcpy(FullRGBPalette, RGBPalette->Data, RGBPalette->DataSize);

			// Destroy cut palette and set pointer to full one
			free(RGBPalette->Data);
			RGBPalette->DataSize = 0x300;
			RGBPalette->Data = FullRGBPalette;
		}
	}

	// Read alpha
	Alpha = PNGReadChunk(ptrFile, "tRNS");

	// Check if alpha is not present
	if (Alpha->Data == NULL)
	{
		puts("Converting 24 bit palette to 32 bit ...");

		// Allocate memory for new alpha
		FullAlpha = (uchar *)malloc(0x100);
		if (FullAlpha == NULL)
		{
			puts("Unable to allocate memory! \n");
			exit(EXIT_FAILURE);
		}

		// Initialize new alpha
		memset(FullAlpha, 0xFF, 0x100);

		// Set pointer to new alpha
		Alpha->DataSize = 0x100;
		Alpha->Data = FullAlpha;
	}
	else
	{
		// Check if alpha is cut
		if (Alpha->DataSize < 0x100)
		{
			puts("Alpha is cut, restoring ...");

			// Allocate memory for full alpha
			FullAlpha = (uchar *)malloc(0x100);
			if (FullAlpha == NULL)
			{
				puts("Unable to allocate memory! \n");
				exit(EXIT_FAILURE);
			}

			// Copy cut alpha to full
			memset(FullAlpha, 0xFF, 0x100);
			memcpy(FullAlpha, Alpha->Data, Alpha->DataSize);

			// Destroy cut alpha and set pointer to full one
			free(Alpha->Data);
			Alpha->DataSize = 0x100;
			Alpha->Data = FullAlpha;
		}
	}

	// Allocate memory for RGBA palette
	RGBAPalette = (sPNGData *)malloc(sizeof(sPNGData));
	if (RGBAPalette == NULL)
	{
		puts("Unable to allocate memory! \n");
		exit(EXIT_FAILURE);
	}
	RGBAPalette->DataSize = 0x400;
	RGBAPalette->Data = (uchar *)malloc(RGBAPalette->DataSize);
	if (RGBAPalette->Data == NULL)
	{
		puts("Unable to allocate memory! \n");
		exit(EXIT_FAILURE);
	}

	// Merge RGB palette and Alpha to RGBA palette as in PSI
	for (ulong Element = 0; Element < 0x100; Element++)
	{
		RGBAPalette->Data[Element * 4 + 0] = RGBPalette->Data[Element * 3 + 0];
		RGBAPalette->Data[Element * 4 + 1] = RGBPalette->Data[Element * 3 + 1];
		RGBAPalette->Data[Element * 4 + 2] = RGBPalette->Data[Element * 3 + 2];
		RGBAPalette->Data[Element * 4 + 3] = Alpha->Data[Element];
	}

	// Free memory
	free(RGBPalette);
	free(Alpha);

	// Return pointer
	return RGBAPalette;
}

sPNGData * PNGReadBitmap(FILE ** ptrFile, uint Width, uint Height, uchar BytesPerPixel, uint BitDepth)
{
	sPNGData * PNGImgData;

	// Allocate memory
	PNGImgData = (sPNGData *)malloc(sizeof(sPNGData));
	if (PNGImgData == NULL)
	{
		puts("Unable to allocate memory! \n");
		exit(EXIT_FAILURE);
	}

	// Read compressed data
	PNGImgData = PNGReadChunk(ptrFile, "IDAT");
	if (PNGImgData->Data == NULL)
	{
		puts("Can't read image data ... \n");
		exit(EXIT_FAILURE);
	}

	// Decompress data
	PNGDecompress(PNGImgData);

	// Unfilter
	if (PNGUnfilter(PNGImgData, Height, Width, BytesPerPixel, BitDepth) == false)
	{
		puts("Can't unfilter image ... \n");
		exit(EXIT_FAILURE);
	}

	// If bimap is 24 bit then convert it to 32 bit format (add alpha)
	if (BytesPerPixel == 3)
	{
		uchar * Bitmap32;
		ulong Bitmap32Size;

		puts("Converting 24 bit bitmap to 32 bit format ...");

		// Allocate memory
		Bitmap32Size = Width * Height * 4;
		Bitmap32 = (uchar *)malloc(Bitmap32Size);
		if (Bitmap32 == NULL)
		{
			puts("Unable to allocate memory ... \n");
			exit(EXIT_FAILURE);
		}

		// Add alpha to each pixel of bitmap
		for (ulong Pixel = 0; Pixel < (Width * Height); Pixel++)
		{
			Bitmap32[Pixel * 4 + 0] = PNGImgData->Data[Pixel * BytesPerPixel + 0];
			Bitmap32[Pixel * 4 + 1] = PNGImgData->Data[Pixel * BytesPerPixel + 1];
			Bitmap32[Pixel * 4 + 2] = PNGImgData->Data[Pixel * BytesPerPixel + 2];
			Bitmap32[Pixel * 4 + 3] = 0xFF;
		}

		// Destroy 24 bit bitmap and set pointer to 32 bit bitmap
		free(PNGImgData->Data);
		PNGImgData->DataSize = Bitmap32Size;
		PNGImgData->Data = Bitmap32;
	}

	return PNGImgData;
}

void PNGWritePalette(FILE ** ptrFile, sPNGData * RGBAPalette)
{
	ulong RGBPaletteSize = 0x300;
	uchar RGBPalette[0x300];

	ulong AlphaSize = 0x100;
	uchar Alpha[0x100];

	// Divide RGBA palette to RGB palette and Alpha as in PNG
	for (ulong Element = 0; Element < 0x100; Element++)
	{
		RGBPalette[Element * 3 + 0] = RGBAPalette->Data[Element * 4 + 0];
		RGBPalette[Element * 3 + 1] = RGBAPalette->Data[Element * 4 + 1];
		RGBPalette[Element * 3 + 2] = RGBAPalette->Data[Element * 4 + 2];

		Alpha[Element] = RGBAPalette->Data[Element * 4 + 3];
	}

	// Write palette
	PNGWriteChunk(ptrFile, "PLTE", RGBPalette, RGBPaletteSize);

	// Write alpha
	PNGWriteChunk(ptrFile, "tRNS", Alpha, AlphaSize);
}

void PNGWriteBitmap(FILE ** ptrFile, uint Width, uint Height, uchar BytesPerPixel, sPNGData * RGBABitmap)
{
	uchar FilterType = 4;	// Paeth filter

							// Apply filter
	PNGFilter(RGBABitmap, Height, Width, BytesPerPixel, FilterType);

	// Compress image
	PNGCompress(RGBABitmap);

	// Write image "IDAT" chunk
	PNGWriteChunk(ptrFile, "IDAT", RGBABitmap);
}

bool ZDecompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize, ulong StartSize)
{
	// Setting up zlib variables for decomression
	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;

	// Setting up other variables
	uchar * NewData;
	ulong NewDataSize;
	bool Finish;

	// Set starting size of decompressed data (would be increased if bigger)
	NewDataSize = StartSize;

	// Decompression loop
	do
	{
		// Allocate memory for decompressed data
		NewData = (uchar *)malloc(NewDataSize);
		if (NewData == NULL)
		{
			puts("Unable to allocate memory ...");
			return false;
		}

		infstream.next_in = (Bytef *)InputData;			// Input data pointer (compressed data)
		infstream.avail_in = (uint)InputDataSize;		// Size of input data
		infstream.next_out = (Bytef *)NewData;			// Output data pointer (decompressed data)
		infstream.avail_out = (uint)NewDataSize;		// Size of output data

														// Decompression work
		if (inflateInit(&infstream, ZLIB_VERSION, sizeof(infstream)) != Z_OK)
		{
			puts("Zlib: can't decompress data ...");
			return false;
		}
		inflate(&infstream, Z_FINISH);
		inflateEnd(&infstream);

		// if buffer is full then increase buffer size and retry decompression
		if (NewDataSize == infstream.total_out)
		{
			free(NewData);
			NewDataSize = NewDataSize * 2 + 1;		// +1 to avoid infinite loop, when StartSize = 0

			Finish = false;
		}
		else
		{
			Finish = true;
		}
	} while (Finish == false);

	// Check if output data has zero size
	if (infstream.total_out == 0)
		return false;

	//puts("Decompression is completed successfully.");

	// Return data pointer, data size and result
	*OutputData = NewData;
	*OutputDataSize = infstream.total_out;
	return true;
}

bool ZCompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize)
{
	// Allocate memory for compressed data
	uchar * NewData;
	ulong NewDataSize;

	NewDataSize = InputDataSize;
	NewData = (uchar *)malloc(NewDataSize);
	if (NewData == NULL)
	{
		puts("Unable to allocate memory ...");
		return false;
	}

	// Setting up zlib variables for compression
	z_stream defstream;
	defstream.zalloc = Z_NULL;
	defstream.zfree = Z_NULL;
	defstream.opaque = Z_NULL;

	defstream.next_in = (Bytef *)InputData;			// Input data pointer (decompressed data)
	defstream.avail_in = (ulong)InputDataSize;		// Size of input data
	defstream.next_out = (Bytef *)NewData;			// Output data pointer (compressed data)
	defstream.avail_out = (ulong)NewDataSize;		// Size of output data

													// Compression work.
	deflateInit(&defstream, Z_BEST_COMPRESSION);
	deflate(&defstream, Z_FINISH);
	deflateEnd(&defstream);

	// Check if output data has zero size
	if (defstream.total_out == 0)
		return false;

	//puts("Compression is completed succesfully.");

	// Return data pointer, data size and result
	*OutputData = NewData;
	*OutputDataSize = defstream.total_out;
	return true;
}

