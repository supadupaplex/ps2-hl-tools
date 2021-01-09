// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This file contains all definitions and declarations
//

#ifndef MAIN_H
#define MAIN_H

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), sscanf(), snprintf(), rename(), remove()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy(), memset()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round(), sqrt(), ceil()
#include <ctype.h>		// tolower()

////////// Zlib stuff //////////
#include "zlib.h"
#include <assert.h>

////////// Definitions //////////
#define PROG_VERSION "1.01"

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////
#include "fops.h"

////////// Structures //////////

// PS2 compressed *.txt header
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2CmpTxtHeader
{
	char Signature[10];		// "COMPRESSED" - signature

	void UpdateFromFile(FILE **ptrFile)
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sPS2CmpTxtHeader));
	}
	
	void Update()
	{
		strncpy(this->Signature, "COMPRESSED", 10);
	}

	bool Check()
	{
		char Temp[11];

		memset(Temp, 0x00, sizeof(Temp));
		memcpy(Temp, this->Signature, sizeof(Signature));

		if (!strcmp(Temp, "COMPRESSED") == true)
			return true;
		else
			return false;
	}


};

#endif // MAIN_H
