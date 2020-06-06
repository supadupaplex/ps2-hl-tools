/*
=====================================================================
Copyright (c) 2018, Alexey Leushin
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
// This module contains functions that perform conversion of PC HL *.nod files to PS2 format
//

////////// Includes //////////
#include "util.h"
#include "main.h"				// Main header

////////// Functions //////////
int TestFile(const char * FileName);
void ConvertNOD(const char * FileName);

int TestFile(const char * FileName)
{
	FILE * ptrFile;
	sNodeGraph NGraph;

	printf("\nTesting file: %s \n", FileName);

	// Open file for reading
	SafeFileOpen(&ptrFile, FileName, "rb");

	// Load and check header
	NGraph.Init();
	int Result = NGraph.LoadAndCheckHeader(&ptrFile);

	// Show info
	switch (Result)
	{
	case NOD_FORMAT_PC:
		puts("Proper PC file \n");
		break;
	case NOD_FORMAT_PS2:
		puts("Proper PS2 file \n");
		break;
	case NOD_ERR_VERSION:
		printf("Unknown file: version %d, should be 16 \n\n", NGraph.Version);
		break;
	case NOD_ERR_UNKNOWN:
		puts("Unknown file: size mismatch \n");
		break;
	}

	// Close file
	fclose(ptrFile);

	// Return result
	return Result;
}

void ConvertNOD(const char * FileName)
{
	FILE * ptrFile;
	sNodeGraph NGraph;
	int Result;

	printf("\nProcessing file: %s \n", FileName);

	// Open file for reading
	SafeFileOpen(&ptrFile, FileName, "rb");

	// Load data
	NGraph.Init();
	Result = NGraph.UpdateFromFile(&ptrFile);
	if (Result == NOD_ERR_VERSION)
	{
		printf("Unknown file: version %d, should be 16 \nPress any key to exit... \n\n", NGraph.Version);
		UTIL_WAIT_KEY;
		fclose(ptrFile);
		return;
	}
	else if (Result == NOD_ERR_UNKNOWN)
	{
		puts("Unknown file: size mismatch \nPress any key to exit... \n");
		UTIL_WAIT_KEY;
		fclose(ptrFile);
		return;
	}

	// Show some info
	printf("Properties: \n Nodes: %d \n Links: %d \n Routes: %d \n Hashes: %d \n", NGraph.CGraph.NodeCount, NGraph.CGraph.LinkCount, NGraph.CGraph.RouteCount, NGraph.CGraph.HashCount);

	// Close file
	fclose(ptrFile);

	// Open file for writing
	SafeFileOpen(&ptrFile, FileName, "wb");

	// Write data
	if (Result == NOD_FORMAT_PS2)
		NGraph.SaveToFile(&ptrFile, NOD_FORMAT_PC);
	else
		NGraph.SaveToFile(&ptrFile, NOD_FORMAT_PS2);

	// Free memory
	NGraph.Deinit();

	// Close file
	fclose(ptrFile);

	puts("\nDone! \n");
}

int main(int argc, char * argv[])
{
	char cExtension[5];

	printf("\nPS2 HL NOD tool v%s \n", PROG_VERSION);

	if (argc == 1)
	{
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2018, Alexey Leushin. All rights reserved.");
		puts("How to use: \n1) Windows explorer - drag and drop *.nod file on nodtool.exe \n2) Command line\\Batch - nodtool [file_name] \n");
		puts("\tCheck file: nodtool test [file_name] \n\n");
		puts("For more info read ReadMe.txt \n");
		puts("Press any key to exit ...");
		UTIL_WAIT_KEY;
	}
	else if (argc == 2)
	{
		FileGetExtension(argv[1], cExtension, sizeof(cExtension));

		if (!strcmp(cExtension, ".nod") == true)
		{
			ConvertNOD(argv[1]);
			return 0;
		}
		else
		{
			puts("Unsupported file ...");
		}
	}
	else if (argc == 3)
	{
		if (!strcmp(argv[1], "test") == true)
		{
			FileGetExtension(argv[2], cExtension, sizeof(cExtension));
			if (!strcmp(cExtension, ".nod") == true)
			{
				TestFile(argv[2]);
				return 0;
			}
			else
			{
				puts("Unsupported file ...");
			}
		}
		else
		{
			puts("Can't recognise arguments ...");
		}
	}
	else
	{
		puts("Too many arguments ...");
	}
	
	return 1;
}
