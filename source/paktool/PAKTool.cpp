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
// Zlib library is used within this module to perform DEFLATE\INFLATE operations
//
// This module contains functions that perform various operations with PAK files
//

////////// Includes //////////
#include "main.h"				// Main header

////////// Functions //////////
void ExtractPAK(const char * cFile);																						// Extract given PAK file
void PackPAK(const char * cFolder, ulong SegmentSize);																		// Pack folder into PAK
bool DecompressPAK(const char * cFile);																						// Decompress PAK file
bool CompressPAK(const char * cFile);																						// Compress PAK file
int CheckPAK(const char * cFile, bool PrintInfo);																			// Check PAK file
bool ZDecompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize, ulong StartSize);		// Compress data with zlib
bool ZCompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize);						// Decompress data with zlib
ulong CalculateFileSpace(ulong FileSize, ulong SegmentSize);																// Calculate amount of space occupied by file inside PAK
void ConvertToGRE(const char * cFile);																						// Convert PAK to GRESTORE format



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
		if (inflateInit(&infstream) != Z_OK) //, ZLIB_VERSION, sizeof(infstream)) != Z_OK)
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

	puts("Decompression is completed successfully");

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

	puts("Compression is completed succesfully");

	// Return data pointer, data size and result
	*OutputData = NewData;
	*OutputDataSize = defstream.total_out;
	return true;
}

int CheckPAK(const char * cFile, bool PrintInfo)
{
	FILE * ptrInputF;				// Input file stream
	uPS2PAKHeader PS2PAKHeader;		// PAK header
	
	int PAKType;
	uint FileCounter;

	// Open file
	SafeFileOpen(&ptrInputF, cFile, "rb");

	// Load header
	PS2PAKHeader.UpdateFromFile(&ptrInputF);
	PAKType = PS2PAKHeader.CheckType();

	// Print information about file
	if (PrintInfo == true)
	{
		if (PAKType == PAK_NORMAL)
		{
			puts("\nNormal PAK");
			printf("Table offset: 0x%X \n", PS2PAKHeader.Normal.TableOffset);
			printf("Table size: 0x%X \n", PS2PAKHeader.Normal.TableSize);
			FileCounter = PS2PAKHeader.Normal.TableSize / sizeof(sPS2PAKFileEntry);
			printf("Files in PAK: %i \n", FileCounter);

			/*sPS2PAKFileEntry PS2PAKFileEntry;
			puts("\nList of files: \n");
			for (int i = 0; i < FileCounter; i++)
			{
				printf("\nFile entry #%i\n", i);
				PS2PAKFileEntry.UpdateFromFile(&ptrInputF, PS2PAKHeader.Normal.TableOffset + sizeof(sPS2PAKFileEntry) * i);
				printf("File name: %s \n", PS2PAKFileEntry.FileName);
				printf("File offset: 0x%X \n", PS2PAKFileEntry.FileOffset);
				printf("File size: %i bytes \n\n", PS2PAKFileEntry.FileSize);
			}*/
		}
		else if (PAKType == PAK_COMPRESSED)
		{
			puts("\nCompressed PAK");
			printf("Decompressed PAK target size: %i bytes \n", PS2PAKHeader.Compressed.PAKSize);
		}
		else
		{
			puts("\nUnsupported file ...\n");
		}
	}

	// Close file
	fclose(ptrInputF);

	return PAKType;
}

ulong CalculateFileSpace(ulong FileSize, ulong SegmentSize)
{
	// Gearbox
	//return (FileSize / SegmentSize + 1) * SegmentSize;

	// Proper
	return (ulong) ceil((double) FileSize / (double) SegmentSize) * SegmentSize;
}

void ExtractPAK(const char * cFile)
{
	FILE * ptrInputF;		// Stream for input file (PAK)
	FILE * ptrOutputF;		// Stream for output files

	uPS2PAKHeader PS2PAKHeader;			// PAK header
	sPS2PAKFileEntry PS2PAKFileEntry;	// PAK file table entry

	char * TempBuffer;		// Buffer for file data
	ulong TempBufferSize;;	// Buffer size

	uint FileCounter;

	char cFolder[255];		// Output folder name
	char cOutFile[255];		// PAK file name
	char cTemp[255];		// Temporary string for concatenation

	// Open PAK file
	SafeFileOpen(&ptrInputF, cFile, "rb");

	// Read header
	PS2PAKHeader.UpdateFromFile(&ptrInputF);

	// Check header, extract if PAK file is decompressed
	if (PS2PAKHeader.CheckType() == PAK_UNKNOWN)
	{
		puts("\nUnsupported file ...\n");
		return;
	}
	else if (PS2PAKHeader.CheckType() == PAK_COMPRESSED)
	{
		puts("\nCompressed PAK. Decompress it to extract files ...\n");
		return;
	}
	if (PS2PAKHeader.CheckType() == PAK_NORMAL)
	{
		puts("Extracting ... \n");
		printf("Table offset: %x \n", PS2PAKHeader.Normal.TableOffset);
		printf("Table size: %x \n", PS2PAKHeader.Normal.TableSize);
		FileCounter = PS2PAKHeader.Normal.TableSize / 0x40;
		printf("Files in PAK: %i \n", FileCounter);

		// Create directory for extracted files
		FileGetPath(cFile, cFolder, sizeof(cFolder));
		strcat(cFolder, "ext-");
		FileGetName(cFile, cTemp, sizeof(cTemp), true);
		strcat(cFolder, cTemp);
		NewDir(cFolder);

		// Extract files
		for (int i = 0; i < FileCounter; i++)
		{
			printf("\nExtracting file #%i\n", i);
			PS2PAKFileEntry.UpdateFromFile(&ptrInputF, PS2PAKHeader.Normal.TableOffset + sizeof(sPS2PAKFileEntry) * i);

			printf("File name: %s \n", PS2PAKFileEntry.FileName);
			printf("File offset: 0x%X \n", PS2PAKFileEntry.FileOffset);
			printf("File size: %i bytes \n\n", PS2PAKFileEntry.FileSize);

			// Get full file name
			strcpy(cOutFile, cFolder);
			strcat(cOutFile, "\\");
			strcat(cOutFile, PS2PAKFileEntry.FileName);
			PatchSlashes(cOutFile, strlen(cOutFile), true);

			// Create all folders, specified in file's name (if needed)
			GenerateFolders(cOutFile);

			// Create file
			SafeFileOpen(&ptrOutputF, cOutFile, "wb");

			// Copy file data from PAK to RAM
			TempBufferSize = PS2PAKFileEntry.FileSize;
			TempBuffer = (char *)malloc(TempBufferSize);
			if (TempBuffer == NULL)
			{
				puts("Unable to allocate memory ...");
				_getch();
				exit(1);
			}
			FileReadBlock(&ptrInputF, TempBuffer, PS2PAKFileEntry.FileOffset, PS2PAKFileEntry.FileSize);

			// Write file data from RAM to output file
			FileWriteBlock(&ptrOutputF, TempBuffer, PS2PAKFileEntry.FileSize);

			// Destroy buffer
			free(TempBuffer);

			// Close output file
			fclose(ptrOutputF);
		}

		puts("\nExtraction complete\n");
	}

	// Close PAK
	fclose(ptrInputF);
}

void PackPAK(const char * cFolder, ulong SegmentSize)
{
	FILE * ptrInputF;			// Stream for input files
	FILE * ptrOutputF;			// Stream for output file (PAK)

	sFileListEntry * FileList;			// List of files to pack
	uPS2PAKHeader PS2PAKHeader;			// PAK header
	sPS2PAKFileEntry PS2PAKFileEntry;	// PAK file table entry
	
	char * TempBuffer;			// Temporary buffer for files
	ulong TempBufferSize;		// Temporary buffer size
	
	uint FileCounter;					//
	ulong PS2PAKTableSizeCounter;		// Counters
	ulong PS2PAKDataSizeCounter;		//

	char cFile[255];			// Input file name
	const char * NextFile;		// Next file name from iterator
	char cOutFile[255];			// Output PAK file name


	// Count files in folder
	FileCounter = 0;
	DirIterInit(cFolder);
	while ( DirIterGet() )
		FileCounter++;
	DirIterClose();
	if (FileCounter == 0)
	{
		puts("Empty dir, nothing to pack ...");
		exit(1);
	}

	// Allocate memory for list of files
	FileList = (sFileListEntry *)malloc(sizeof(sFileListEntry)*FileCounter);
	printf("Found %i file(s), packing ...\n", FileCounter);

	// Fill list
	FileCounter = 0;
	DirIterInit(cFolder);
	while ( (NextFile = DirIterGet()) != NULL )
	{
		strcpy(cFile, NextFile);

		SafeFileOpen(&ptrInputF, cFile, "rb");
		//printf("%s, %i\n", cFile, FileSize(&ptrInputF));

		FileList[FileCounter].Update(cFile, FileSize(&ptrInputF));
		FileCounter++;

		fclose(ptrInputF);
	}
	DirIterClose();

	// Calculate PAK size
	PS2PAKDataSizeCounter = 0;
	for (uint i = 0; i < FileCounter; i++)									// Calculate file data size
		PS2PAKDataSizeCounter += CalculateFileSpace(FileList[i].FileSize, SegmentSize);
	PS2PAKTableSizeCounter = sizeof(sPS2PAKFileEntry) * FileCounter;		// Calculate file table size

	// Create new PAK file
	strcpy(cOutFile, cFolder);
	strcat(cOutFile, ".PAK");
	SafeFileOpen(&ptrOutputF, cOutFile, "wb");

	// Allocate buffer for header
	TempBufferSize = CalculateFileSpace(sizeof(sPS2NormalPAKHeader), SegmentSize);
	TempBuffer = (char *)malloc(TempBufferSize);
	// Clear buffer from garbage
	memset(TempBuffer, 0x00, TempBufferSize);
	// Generate header
	PS2PAKHeader.UpdateNormal(CalculateFileSpace(sizeof(sPS2NormalPAKHeader), SegmentSize) + PS2PAKDataSizeCounter, PS2PAKTableSizeCounter);
	// Copy header to buffer
	memcpy(TempBuffer, &PS2PAKHeader, sizeof(sPS2NormalPAKHeader));
	// Write buffer with header to file
	FileWriteBlock(&ptrOutputF, TempBuffer, TempBufferSize);
	// Destroy buffer
	free(TempBuffer);

	// Write file data to PAK
	for (uint i = 0; i < FileCounter; i++)
	{
		printf("\nPacking file #%i: %s \nSize: %i \n", i + 1, FileList[i].FileName, FileList[i].FileSize);

		// Open file
		strcpy(cFile, FileList[i].FileName);
		SafeFileOpen(&ptrInputF, cFile, "rb");

		// Allocate buffer for file data
		TempBufferSize = CalculateFileSpace(FileList[i].FileSize, SegmentSize);
		TempBuffer = (char *)malloc(TempBufferSize);

		// Clear allocated space from garbage
		memset(TempBuffer, 0x00, TempBufferSize);

		// Copy file data to buffer
		FileReadBlock(&ptrInputF, TempBuffer, 0, FileList[i].FileSize);

		// Write data from buffer to PAK file
		FileWriteBlock(&ptrOutputF, TempBuffer, TempBufferSize);
		fflush(ptrOutputF); // Write to the disk immediately
		
		// Destroy buffer
		free(TempBuffer);

		// Close file
		fclose(ptrInputF);
	}

	// Write file table to PAK
	puts("\nWriting file table ...");
	PS2PAKDataSizeCounter = CalculateFileSpace(sizeof(sPS2NormalPAKHeader), SegmentSize);	// Reset data size counter to first file's offset
	for (uint i = 0; i < FileCounter; i++)
	{
		// Create file entry
		memcpy(cFile, FileList[i].FileName + strlen(cFolder) + 1, strlen(FileList[i].FileName) - strlen(cFolder));		// Cut outside folders from path
		PatchSlashes(cFile, sizeof(cFile), false);																		// Patch windows backslashes to PAK slashes
		PS2PAKFileEntry.Update(cFile, PS2PAKDataSizeCounter, FileList[i].FileSize);

		// Write file entry
		FileWriteBlock(&ptrOutputF, &PS2PAKFileEntry, sizeof(sPS2PAKFileEntry));

		//Calculate next file's offset
		PS2PAKDataSizeCounter += CalculateFileSpace(FileList[i].FileSize, SegmentSize);
	}

	//for (int i = 0; i < FileCounter; i++)
	//	printf("#%i - File name: %s \n File Size: %i | Reference: %i | RFName: %s \n\n", i + 1, FileList[i].FileName, FileList[i].FileSize, FileList[i].ExtReference, FileList[i].ExtFileName);

	printf("\nDone\n\n");

	fclose(ptrOutputF);
	free(FileList);
}

bool DecompressPAK(const char * cFile)
{
	FILE * ptrInputF;	// Compressed file pointer
	FILE * ptrOutputF;	// Decompressed file pointer
	
	uchar * CData;		// Compressed data
	ulong CDataSize;	// Compressed data size
	uchar * DData;		// Decompressed data
	ulong DDataSize;	// Decompressed data size

	uPS2PAKHeader PS2PAKHeader;	// PAK header
	char cOutFile[255];			// Output file name
	char cTemp[255];			// Temporary string for concatenation

	// Open and check compressed PAK
	SafeFileOpen(&ptrInputF, cFile, "rb");
	PS2PAKHeader.UpdateFromFile(&ptrInputF);
	if (PS2PAKHeader.CheckType() == PAK_UNKNOWN)
	{
		puts("Unsupported file");
		return false;
	}
	else if (PS2PAKHeader.CheckType() == PAK_NORMAL)
	{
		puts("File is already decompressed");
		return false;
	}

	puts("Decompressing ...");

	// Allocate memory for compressed data
	CDataSize = FileSize(&ptrInputF) - sizeof(PS2PAKHeader.Compressed.PAKSize);
	CData = (uchar *)malloc(CDataSize);

	// Read compressed data from file
	FileReadBlock(&ptrInputF, CData, sizeof(PS2PAKHeader.Compressed.PAKSize), CDataSize);

	// Decompress data. Setting start size of decompressed data to 2 sizes of compressed file to avoid looping.
	if (ZDecompress(CData, CDataSize, &DData, &DDataSize, CDataSize * 2) != true)
	{
		puts("Unable to decompress file ...");
		return false;
	}

	// Create output file
	FileGetPath(cFile, cOutFile, sizeof(cOutFile));
	strcat(cOutFile, "dec-");
	FileGetName(cFile, cTemp, sizeof(cTemp), true);
	strcat(cOutFile, cTemp);
	SafeFileOpen(&ptrOutputF, cOutFile, "wb");

	// Write decompressed data to file
	FileWriteBlock(&ptrOutputF, DData, DDataSize);

	// Free memory
	free(CData);
	free(DData);

	// Close files
	fclose(ptrOutputF);
	fclose(ptrInputF);

	// Give warning if file size is't equal to target
	if (PS2PAKHeader.Compressed.PAKSize != DDataSize)
		printf("\nWarning - File size mismatch! \nOriginal size: %i bytes \nTarget size: %i bytes \nActual size: %i bytes \n\n", CDataSize, PS2PAKHeader.Compressed.PAKSize, DDataSize);
	else
		printf("\nFile is successfully decompressed \nOriginal size: %i bytes \nDecompressed size: %i bytes \n\n", CDataSize, DDataSize);

	return true;
}

bool CompressPAK(const char * cFile)
{
	FILE * ptrInputF;	// Decompressed file
	FILE * ptrOutputF;	// Compressed file

	uchar * DData;		// Decompressed data
	ulong DDataSize;	// Decompressed data size
	uchar * CData;		// Compressed data
	ulong CDataSize;	// Compressed data size

	uPS2PAKHeader PS2PAKHeader;	// PAK header
	char cOutFile[255];			// Output file name
	char cTemp[255];			// Temporary string for concatenation

	// Open and check PAK
	SafeFileOpen(&ptrInputF, cFile, "rb");
	PS2PAKHeader.UpdateFromFile(&ptrInputF);
	if (PS2PAKHeader.CheckType() == PAK_UNKNOWN)
	{
		puts("Unsupported file");
		return false;
	}
	else if (PS2PAKHeader.CheckType() == PAK_COMPRESSED)
	{
		puts("File is already compressed");
		return false;
	}

	puts("Compressing ...");

	// Allocate memory for decompressed data
	DDataSize = FileSize(&ptrInputF);
	DData = (uchar *)malloc(DDataSize);

	// Read decompressed data from file
	FileReadBlock(&ptrInputF, DData, 0, DDataSize);

	// Compress data
	if (ZCompress(DData, DDataSize, &CData, &CDataSize) != true)
	{
		puts("Zlib: unable to compress file ...");
		return false;
	}

	// Create output file
	FileGetPath(cFile, cOutFile, sizeof(cOutFile));
	strcat(cOutFile, "cmp-");
	FileGetName(cFile, cTemp, sizeof(cTemp), true);
	strcat(cOutFile, cTemp);
	SafeFileOpen(&ptrOutputF, cOutFile, "wb");

	// Write size of decompressed file and compressed data to file
	FileWriteBlock(&ptrOutputF, &DDataSize, sizeof(DDataSize));
	FileWriteBlock(&ptrOutputF, CData, CDataSize);

	// Free memory
	free(CData);
	free(DData);

	// Close files
	fclose(ptrOutputF);
	fclose(ptrInputF);

	// Print some info
	printf("\nFile is successfully compressed \nOriginal size: %i bytes \nCompressed size: %i bytes \n\n", DDataSize, CDataSize);

	return true;
}

void ConvertToGRE(const char * cFile)
{
	FILE * ptrInPAK;						// Input file stream
	FILE * ptrOutPAK;						// Output file stream
	char cOutFileName[255];					// Output file name
	char cTemp[255];						// Temporary string for concatenation

	uPS2PAKHeader PS2PAKHeader;				// PAK file header
	char * PAKData;							// PAK file raw data
	ulong PAKDataSize;						// PAK file raw data size
	sPS2PAKFileEntry * PAKFileTable;		// Pointer to PAK file table
	ulong PAKFileTableSize;					// PAK file table size
	ulong PAKFileCount;						// How many files in PAK

	sSPZHeader * SPZHeader;					// Pointer to SPZ header
	sSPZFrameTableEntry * SPZFrameEntry;	// Pointer to SPZ frame table
	bool ModelFlag;							// For model detection

	puts("Converting to GRESTORE ... \n");

	// Open input pak
	SafeFileOpen(&ptrInPAK, cFile, "rb");
	
	// Read header and check PAK
	PS2PAKHeader.UpdateFromFile(&ptrInPAK);
	if (PS2PAKHeader.CheckType() != PAK_NORMAL)
	{
		puts("Can't apply patch ...");
		return;
	}

	// Load PAK file data
	PAKDataSize = FileSize(&ptrInPAK) - PS2PAKHeader.Normal.TableSize;
	PAKData = (char *)malloc(PAKDataSize);
	if (PAKData == NULL)
	{
		puts("Unable to allocate memory ...");
		_getch();
		exit(1);
	}
	FileReadBlock(&ptrInPAK, PAKData, 0, PAKDataSize);

	// Load PAK file table
	PAKFileTableSize = PS2PAKHeader.Normal.TableSize;
	PAKFileTable = (sPS2PAKFileEntry *)malloc(PAKFileTableSize);
	PAKFileCount = PAKFileTableSize / sizeof(sPS2PAKFileEntry);
	if (PAKFileTable == NULL)
	{
		puts("Unable to allocate memory ...");
		_getch();
		exit(1);
	}
	FileReadBlock(&ptrInPAK, PAKFileTable, PS2PAKHeader.Normal.TableOffset, PAKFileTableSize);

	// Patch sprite frames
	char cExtension[5];
	ulong FrameID = SPZ_BASE_FRAMEID;
	ulong RAMOffset = GLOBAL_PAK_RAM_OFFSET - FileSize(&ptrInPAK) + (FileSize(&ptrInPAK) % GLOBAL_PAK_RAM_ALIGN);
	ModelFlag = false;
	for (ulong File = 0; File < PAKFileCount; File++)
	{
		FileGetExtension(PAKFileTable[File].FileName, cExtension, sizeof(cExtension));

		if (!strcmp(cExtension, ".spz") == true)
		{
			SPZHeader = (sSPZHeader *) &PAKData[PAKFileTable[File].FileOffset];

			if (SPZHeader->CheckSignature() == true)
			{
				SPZHeader->RAMFlag = 1;

				for (uint Frame = 0; Frame < SPZHeader->FrameCount; Frame++)
				{
					SPZFrameEntry = (sSPZFrameTableEntry *) (&PAKData[PAKFileTable[File].FileOffset] + sizeof(sSPZHeader) + sizeof(sSPZFrameTableEntry) * Frame);
					SPZFrameEntry->Update(FrameID, RAMOffset + PAKFileTable[File].FileOffset + SPZFrameEntry->FrameOffset);

					FrameID++;
				}
			}
		}
		else if (!strcmp(cExtension, ".dol") == true)
		{
			ModelFlag = true;
		}
	}

	// Open output pak
	FileGetPath(cFile, cOutFileName, sizeof(cOutFileName));
	strcat(cOutFileName, "gre-");
	FileGetName(cFile, cTemp, sizeof(cTemp), true);
	strcat(cOutFileName, cTemp);
	SafeFileOpen(&ptrOutPAK, cOutFileName, "wb");

	// Write modified PAK file data and PAK file table
	FileWriteBlock(&ptrOutPAK, PAKData, PAKDataSize);
	FileWriteBlock(&ptrOutPAK, PAKFileTable, PAKFileTableSize);

	// Show warning if found model files
	if (ModelFlag == true)
	{
		puts("Warning! Model files should not be inside GLOBAL.PAK and GRESTORE.PAK.");
		puts("You may experience problems with those PAK's. Press any key to confirm ... \n");
		_getch();
	}

	// Free memory
	free(PAKData);
	free(PAKFileTable);

	// Close files
	fclose(ptrInPAK);
	fclose(ptrOutPAK);
}

int main(int argc, char * argv[])
{
	char cPath[255];
	char cFName[255];
	char cTempFileName[255];
	char cNewFileName[255];
	char Action;

	printf("PS2 HL PAK tool v%s \n", PROG_VERSION);

	if (argc == 1)
	{
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2017-2020, Alexey Leushin. All rights reserved.");
		puts("Zlib library is used within this program to perform DEFLATE\\INFLATE operations.\n");
		puts("How to use: \n1) Windows explorer - drag and drop file or directory on paktool.exe \n2) Command line\\Batch - paktool [file\\dir_name] \n\nFor more info read ReadMe.txt \n");
		puts("Press any key to exit ...");
		_getch();
	}
	else if (argc == 2)
	{
		if (CheckDir(argv[1]) == true)						// Folder
		{
			puts("\nChoose PAK type:");
			puts(" n - normal, align=2048 [PAK0, DECALS, UGMESAVE, DICTS]");
			puts(" s - normal, align=16 [pausegui, discfail]");
			puts(" c - compressed, align=16 [VALVE, DECAY, MIDHL, MIDTOH, MIDDECAY, FRONTEND,");
			puts("\tGUISOUND, GDECALS, CGMESAVE, SYSSAVE, SYSTEM]");
			puts(" g - global, compressed, align=16, *.spz patch [GLOBAL+GRESTORE]");
			do
			{
				Action = _getch();
			} while (Action != 'n' && Action != 'c' && Action != 'g' && Action != 's');

			if (Action == 'n')
			{
				// Normal
				PackPAK(argv[1], PS2HL_NPAK_SEG_SIZE);
			}
			else if (Action == 's')
			{
				// Normal with small alignment (pausegui.pak)
				PackPAK(argv[1], PS2HL_CPAK_SEG_SIZE);
			}
			else if (Action == 'c')
			{
				// Get path and name
				FileGetPath(argv[1], cPath, sizeof(cPath));
				FileGetName(argv[1], cFName, sizeof(cFName), true);

				// Pack
				PackPAK(argv[1], PS2HL_CPAK_SEG_SIZE);
				
				// Compress
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				CompressPAK(cTempFileName);

				// Delete temp file
				remove(cTempFileName);

				// Rename comressed file				
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "cmp-", cFName, ".PAK");
				snprintf(cNewFileName, sizeof(cNewFileName), "%s%s%s", cPath, cFName, ".PAK");
				FileSafeRename(cTempFileName, cNewFileName);
			}
			else
			{
				// Get path and name
				FileGetPath(argv[1], cPath, sizeof(cPath));
				FileGetName(argv[1], cFName, sizeof(cFName), true);

				// Pack
				PackPAK(argv[1], PS2HL_CPAK_SEG_SIZE);

				// Create GRESTORE
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				ConvertToGRE(cTempFileName);

				// Compress PAKs
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				CompressPAK(cTempFileName);
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "gre-", cFName, ".PAK");
				CompressPAK(cTempFileName);

				// Delete temp files
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				remove(cTempFileName);
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "gre-", cFName, ".PAK");
				remove(cTempFileName);

				// Rename comressed files
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "cmp-", cFName, ".PAK");
				snprintf(cNewFileName, sizeof(cNewFileName), "%s%s", cPath, "GLOBAL.PAK");
				FileSafeRename(cTempFileName, cNewFileName);
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "cmp-gre-", cFName, ".PAK");
				snprintf(cNewFileName, sizeof(cNewFileName), "%s%s", cPath, "GRESTORE.PAK");
				FileSafeRename(cTempFileName, cNewFileName);
			}
		}
		else if (CheckPAK(argv[1], false) == 0)				// Normal PS2 PAK
		{
			// Extract
			ExtractPAK(argv[1]);
		}
		else if (CheckPAK(argv[1], false) == 1)				// Compressed PS2 PAK
		{
			// Get path and name
			FileGetPath(argv[1], cPath, sizeof(cPath));
			FileGetName(argv[1], cFName, sizeof(cFName), true);

			// Decompress
			if (DecompressPAK(argv[1]) == true)
			{
				// Extract
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, "dec-", cFName);
				ExtractPAK(cTempFileName);

				// Delete temp file
				remove(cTempFileName);
			}
		}
		else if (CheckPAK(argv[1], false) == -1)			// Unsupported file
		{
			puts("Unsupported file ...");
		}
	}
	else if (argc == 3)
	{
		if (!strcmp(argv[1], "test") == true)
		{
			CheckPAK(argv[2], true);
		}
		else if (!strcmp(argv[1], "extract") == true)
		{
			if (CheckPAK(argv[2], false) == 0)		// Normal PAK
			{
				// Extract
				ExtractPAK(argv[2]);
			}
			else									// Compressed PAK
			{
				// Get path and name
				FileGetPath(argv[2], cPath, sizeof(cPath));
				FileGetName(argv[2], cFName, sizeof(cFName), true);

				// Decompress
				if (DecompressPAK(argv[2]) == true)
				{
					// Extract
					snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, "dec-", cFName);
					ExtractPAK(cTempFileName);

					// Delete temp file
					remove(cTempFileName);
				}
			}
		}
		else if (!strcmp(argv[1], "pack") == true)
		{
			if (CheckDir(argv[2]) == true)
			{
				// Pack
				PackPAK(argv[2], PS2HL_NPAK_SEG_SIZE);
			}
			else
			{
				puts("Specified path isn't directory ...");
			}
		}
		else if (!strcmp(argv[1], "pack16") == true)
		{
			if (CheckDir(argv[2]) == true)
			{
				// Pack
				PackPAK(argv[2], PS2HL_CPAK_SEG_SIZE);
			}
			else
			{
				puts("Specified path isn't directory ...");
			}
		}
		else if (!strcmp(argv[1], "cpack") == true)
		{
			if (CheckDir(argv[2]) == true)
			{
				// Get path and name
				FileGetPath(argv[2], cPath, sizeof(cPath));
				FileGetName(argv[2], cFName, sizeof(cFName), true);

				// Pack
				PackPAK(argv[2], PS2HL_CPAK_SEG_SIZE);

				// Compress
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				CompressPAK(cTempFileName);

				// Delete temp file
				remove(cTempFileName);

				// Rename comressed file				
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "cmp-", cFName, ".PAK");
				snprintf(cNewFileName, sizeof(cNewFileName), "%s%s%s", cPath, cFName, ".PAK");
				FileSafeRename(cTempFileName, cNewFileName);
			}
			else
			{
				puts("Specified path isn't directory ...");
			}
		}
		else if (!strcmp(argv[1], "gpack") == true)
		{
			if (CheckDir(argv[2]) == true)
			{
				// Get path and name
				FileGetPath(argv[2], cPath, sizeof(cPath));
				FileGetName(argv[2], cFName, sizeof(cFName), true);

				// Pack
				PackPAK(argv[2], PS2HL_CPAK_SEG_SIZE);

				// Create GRESTORE
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				ConvertToGRE(cTempFileName);

				// Compress PAKs
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				CompressPAK(cTempFileName);
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "gre-", cFName, ".PAK");
				CompressPAK(cTempFileName);

				// Delete temp files
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s", cPath, cFName, ".PAK");
				remove(cTempFileName);
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "gre-", cFName, ".PAK");
				remove(cTempFileName);

				// Rename comressed files
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "cmp-", cFName, ".PAK");
				snprintf(cNewFileName, sizeof(cNewFileName), "%s%s", cPath, "GLOBAL.PAK");
				FileSafeRename(cTempFileName, cNewFileName);
				snprintf(cTempFileName, sizeof(cTempFileName), "%s%s%s%s", cPath, "cmp-gre-", cFName, ".PAK");
				snprintf(cNewFileName, sizeof(cNewFileName), "%s%s", cPath, "GRESTORE.PAK");
				FileSafeRename(cTempFileName, cNewFileName);
			}
			else
			{
				puts("Specified path isn't directory ...");
			}
		}
		else if (!strcmp(argv[1], "decompress") == true)
		{
			DecompressPAK(argv[2]);
		}
		else if (!strcmp(argv[1], "compress") == true)
		{
			CompressPAK(argv[2]);
		}
		else
		{
			puts("Can't recognise command ...");
		}
	}
	else
	{
		puts("Too many arguments ...");
	}
	
	return 0;
}
