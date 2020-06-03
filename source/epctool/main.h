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
// This file contains all definitions and declarations
//

#pragma once

////////// Includes //////////
#include <stdio.h>		// puts(), printf(), sscanf(), snprintf(), rename(), remove()
#include <conio.h>		// _getch()
#include <direct.h>		// _mkdir()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy(), memset()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round(), sqrt(), ceil()
#include <ctype.h>		// tolower()
#include <sys\stat.h>	// stat()
#include <windows.h>	// CreateDitectoryA()

////////// Definitions //////////
#define PROG_VERSION "0.8"
#define PS2HL_EPC_ALLIGNMENT 4

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////
#include "fops.h"

////////// Structures //////////

// List of pecache items
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPrecacheList
{
	char * List[256];				// Stores pointers to list items strings
	bool Type[256];					// true - map, false - model
	uchar RefCount[256];			// Number of references
	uchar ListSz = 0;				// How many items are currently in the list

	void Init()
	{
		memset(this, 0x00, sizeof(this));
	}

	bool Add(const char * Item, bool ItemType) //, uchar References
	{
		// Check if item is already on the list
		if (Find(Item) != -1)
			return false;

		// Check if list is full
		if (ListSz == 0xFF)
		{
			puts("256 items limit overflow!");
			return false;
		}

		// Find str length
		ushort Len = strlen(Item);

		// Allocate memory
		ushort Size = Len + 1;
		char * NewItem = (char *) malloc(Size);
		strcpy(NewItem, Item);

		// Add item to the list
		List[ListSz] = NewItem;
		Type[ListSz] = ItemType;
		RefCount[ListSz] = 0; // References
		ListSz++;

		return true;
	}

	// Searches for string and returns it's index or (-1 of not found)
	short Find(const char * Item)
	{
		// Search
		for (uchar i = 0; i < ListSz; i++)
		{
			if (!strcmp(List[i], Item) == true)
				return i;
		}

		// Not found
		return -1;
	}

	// Clear list
	void Clear()
	{
		// Free memory
		for (uchar i = 0; i < ListSz; i++)
			free(List[i]);

		// Fill this structure with 0x00
		memset(this, 0x00, sizeof(this));
	}

	uchar CountMaps()
	{
		uchar Result = 0;

		for (uchar i = 0; i < ListSz; i++)
			if (Type[i] == true)
				Result++;

		return Result;
	}

	/*uchar CountModels()
	{
		uchar Result = 0;

		for (uchar i = 0; i < ListSz; i++)
			if (Type[i] == false)
				Result++;

		return Result;
	}*/
};

// Map entry in precache file
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2HL_EPCMapEntry
{
	uint MapIndex;
	uint ModelsCount;
};

// Model entry in precache file
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2HL_EPCModelEntry
{
	uint Flag;
	uint ModelIndex;
	uint Submodels;
};

/*
// PS2 HL *.epc precache file structure
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2HL_EPC
{
uint ItemCount;
sPrecacheList List;
uint MapCount;
sPS2HL_EPCMapEntry * Maps;
};

// Map entry in precache file
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2HL_EPCMapEntry
{
	uint MapIndex;
	uint ModelsCount;
	sPS2HL_EPCModelEntry * Models;
};

// Model entry in precache file
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPS2HL_EPCModelEntry
{
	bool Flag;
	uint ModelIndex;
	uint Submodels;
};
*/