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
// Zlib library is used within this module to perform DEFLATE\INFLATE operations
//
// This module contains functions that perform decompression\compression of PS2 HL GUI *.txt descriptors
//

////////// Includes //////////
#include "Main.h"				// Main header

////////// Functions //////////
int CheckTXT(const char * cFile);
bool ZDecompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize, ulong StartSize);		// Compress data with zlib
bool ZCompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize);						// Decompress data with zlib
bool CompressTxt(const char * cFile);
bool DecompressTxt(const char * cFile);


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

	puts("Decompression is completed successfully.");

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

	puts("Compression is completed succesfully.");

	// Return data pointer, data size and result
	*OutputData = NewData;
	*OutputDataSize = defstream.total_out;
	return true;
}

int CheckTXT(const char * cFile)
{
	FILE * ptrInputF;					// Input file stream
	sPS2CmpTxtHeader PS2CmpTxtHeader;	// Compressed *.txt header

	// Open file
	SafeFileOpen(&ptrInputF, cFile, "rb");

	// Load header
	PS2CmpTxtHeader.UpdateFromFile(&ptrInputF);

	// Close file
	fclose(ptrInputF);

	return PS2CmpTxtHeader.Check();
}

bool CompressTxt(const char * cFile)
{
	FILE * ptrInFile;
	FILE * ptrOutFile;

	sPS2CmpTxtHeader PS2CmpTxtHeader;

	uchar * DData;
	ulong DDataSize;
	uchar * CData;
	ulong CDataSize;

	// Open input file
	SafeFileOpen(&ptrInFile, cFile, "rb");

	// Update and check header
	PS2CmpTxtHeader.UpdateFromFile(&ptrInFile);
	if (PS2CmpTxtHeader.Check() == true)
		return false;

	// Allocate memory
	DDataSize = FileSize(&ptrInFile);
	DData = (uchar *)malloc(DDataSize);
	if (DData == NULL)
		return false;

	// Get compressed data
	FileReadBlock(&ptrInFile, DData, 0, DDataSize);

	// Compress data
	if (ZCompress(DData, DDataSize, &CData, &CDataSize) == false)
		return false;

	// Close input file
	fclose(ptrInFile);

	// Open output file
	SafeFileOpen(&ptrOutFile, cFile, "wb");

	// Generate proper header for compressed *.txt
	PS2CmpTxtHeader.Update();

	// Write header and compressed data
	FileWriteBlock(&ptrOutFile, &PS2CmpTxtHeader, sizeof(sPS2CmpTxtHeader));
	FileWriteBlock(&ptrOutFile, CData, CDataSize);

	// Free memory
	free(DData);
	free(CData);

	// Close output file
	fclose(ptrOutFile);
}

bool DecompressTxt(const char * cFile)
{
	FILE * ptrInFile;
	FILE * ptrOutFile;

	sPS2CmpTxtHeader PS2CmpTxtHeader;

	uchar * CData;
	ulong CDataSize;
	uchar * DData;
	ulong DDataSize;

	// Open input file
	SafeFileOpen(&ptrInFile, cFile, "rb");

	// Update and check header
	PS2CmpTxtHeader.UpdateFromFile(&ptrInFile);
	if (PS2CmpTxtHeader.Check() == false)
		return false;

	// Allocate memory
	CDataSize = FileSize(&ptrInFile) - sizeof(sPS2CmpTxtHeader);
	CData = (uchar *) malloc(CDataSize);
	if (CData == NULL)
		return false;

	// Get compressed data
	FileReadBlock(&ptrInFile, CData, sizeof(sPS2CmpTxtHeader), CDataSize);

	// Decompress data
	if (ZDecompress(CData, CDataSize, &DData, &DDataSize, 0x100000) == false)
		return false;

	// Close input file
	fclose(ptrInFile);

	// Open output file
	SafeFileOpen(&ptrOutFile, cFile, "wb");

	// Write decompressed data
	FileWriteBlock(&ptrOutFile, DData, DDataSize);

	// Free memory
	free(CData);
	free(DData);

	// Close output file
	fclose(ptrOutFile);
}

void main(int argc, char * argv[])
{
	char cExtension[5];

	printf("\nPS2 HL TXT tool v%s \n", PROG_VERSION);

	if (argc == 1)
	{
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2017, Alexey Leushin. All rights reserved.");
		puts("Zlib library is used within this program to perform DEFLATE\\INFLATE operations.\n");
		puts("How to use: \n1) Windows explorer - drag and drop *.txt file txttool.exe \n2) Command line\\Batch - txttool [file_name] \n\nFor more info read ReadMe.txt \n");
		puts("Press any key to exit ...");
		_getch();
	}
	else if (argc == 2)
	{
		FileGetExtension(argv[1], cExtension, sizeof(cExtension));

		if (!strcmp(cExtension, ".txt") == true)
		{
			printf("Processing file: %s \n", argv[1]);

			if (CheckTXT(argv[1]) == true)					// Compressed PS2 TXT
			{
				if (DecompressTxt(argv[1]) == true)
					puts("Done! \n");
				else
					puts("Decompression failed! \n");
			}
			else											// Normal TXT
			{
				if (CompressTxt(argv[1]) == true)
					puts("Done! \n");
				else
					puts("Compression failed! \n");
			}
		}
		else												// Unsupported file
		{
			puts("Unsupported file ...");
		}
	}
	else
	{
		puts("Too many arguments ...");
	}
}
