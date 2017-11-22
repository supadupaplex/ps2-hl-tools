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
// This file contains functions that perform various file operations
//

////////// Includes //////////
#include "main.h"

ulong FileSize(FILE **ptrFile)
{
	fseek(*ptrFile, 0, SEEK_END);						// Move pointer to the file's end
	return ftell(*ptrFile);								// Return pointer position
}

void FileReadBlock(FILE **ptrSrcFile, void * DstBuff, ulong Addr, ulong Size)
{
	fseek(*ptrSrcFile, Addr, SEEK_SET);					// Seek to specified address
	fread(DstBuff, (size_t)1, Size, *ptrSrcFile);		// 
}

void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Addr, ulong Size)
{
	fseek(*ptrDstFile, Addr, SEEK_SET);					// Seek to specified address
	fwrite(SrcBuff, (size_t)1, Size, *ptrDstFile);		// Write block
	fseek(*ptrDstFile, 0, SEEK_END);					// Set pointer to file's end
}

void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Size)
{
	fseek(*ptrDstFile, 0, SEEK_END);					// Set pointer to file's end
	fwrite(SrcBuff, (size_t)1, Size, *ptrDstFile);		// Write block
}

void SafeFileOpen(FILE **ptrFile, const char * FileName, char * Mode)
{
	fopen_s(ptrFile, FileName, Mode);
	if (*ptrFile == NULL)
	{
		printf("Error: can't open file: %s", FileName);
		exit(EXIT_FAILURE);
	}
}

void FileGetExtension(const char * Path, char * OutputBuffer, uint OutputBufferSize)
{
	if (OutputBufferSize > 4)
		for (int i = 0; i <= 4; i++)
				OutputBuffer[i] = tolower(Path[strlen(Path) - 4 + i]);
}

void FileGetName(const char * Path, char * OutputBuffer, uint OutputBufferSize, bool WithExtension)
{
	int Start = -1;
	int End = strlen(Path);

	for (int i = End; i >= 0; i--)
	{
		if (Path[i] == '\\')
		{
			Start = i;
			break;
		}

		if ((Path[i] == '.') && (WithExtension == false))
		{
			End = i;
			WithExtension = true;
		}

	}

	memcpy(OutputBuffer, Path + (Start + 1), (End - 1) - Start);
	OutputBuffer[(End - 1) - Start] = '\0';
}

void FileGetFullName(const char * Path, char * OutputBuffer, uint OutputBufferSize)
{
	bool SearchExtension = true;
	int End = strlen(Path);

	for (int i = End; i >= 0; i--)
	{
		if (Path[i] == '\\')
			break;

		if (Path[i] == '.' && SearchExtension == true)
		{
			End = i;
			SearchExtension = false;
		}

	}

	memcpy(OutputBuffer, Path, End);
	OutputBuffer[End] = '\0';
}

bool CheckFile(char * FileName)
{
	FILE * ptrTestFile;
	bool Result = false;

	fopen_s(&ptrTestFile, FileName, "r");
	if (ptrTestFile != NULL)
	{
		Result = true;
		fclose(ptrTestFile);
	}

	return Result;
}