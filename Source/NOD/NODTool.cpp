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
#include "Main.h"				// Main header

////////// Functions //////////


void main(int argc, char * argv[])
{
	char cExtension[5];

	printf("\nPS2 HL NOD tool v%s \n", PROG_VERSION);

	if (argc == 1)
	{
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2018, Alexey Leushin. All rights reserved.");
		puts("How to use: \n1) Windows explorer - drag and drop *.nod file on nodtool.exe \n2) Command line\\Batch - nodtool [file_name] \n\nFor more info read ReadMe.txt \n");
		puts("Press any key to exit ...");
		_getch();
	}
	else if (argc == 2)
	{
		FileGetExtension(argv[1], cExtension, sizeof(cExtension));

		if (!strcmp(cExtension, ".nod") == true)
		{
			FILE * ptrFile;
			sPS2NOD PS2NOD;
			int Result;

			printf("\nProcessing file: %s \n", argv[1]);

			// Open file for reading
			SafeFileOpen(&ptrFile, argv[1], "rb");

			// Load data
			PS2NOD.Init();
			Result = PS2NOD.UpdateFromPCFile(&ptrFile);
			if (Result != NO_ERRORS)
			{
				if (Result == ERR_NOD_VERSION)
					puts("Wrong graph version!");
				if (Result == ERR_NOD_CORRUPTED)
					puts("Node file size mismatch! (probably corrupted)");
				if (Result == ERR_NOD_ALREADY_PS2)
					puts("Node file already in PS2 format!");
				getch();
				exit(EXIT_FAILURE);
			}

			// Show some info
			printf("Properties: \n Nodes: %d \n Links: %d \n Routes: %d \n Hashes: %d \n", PS2NOD.CGraph.NodeCount, PS2NOD.CGraph.LinkCount, PS2NOD.CGraph.RouteCount, PS2NOD.CGraph.HashCount);

			// Close file
			fclose(ptrFile);

			// Open file for writing
			SafeFileOpen(&ptrFile, argv[1], "wb"); 

			// Write data
			PS2NOD.SaveToFile(&ptrFile);

			// Close file
			fclose(ptrFile);

			puts("\nDone! \n");
			//getch();
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
