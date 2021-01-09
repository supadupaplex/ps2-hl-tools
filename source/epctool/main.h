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

////////// Definitions //////////
#define PROG_VERSION "0.9"
#define PS2HL_EPC_ALLIGNMENT 4
#define MAX_ITEMS 256

// Keywords
#define KWD_MAP		"mapchange:"
#define KWD_MODEL	"model:"
#define KWD_SEQ		"sequence:"
#define KWD_DIR		"mdldir:"
#define KWD_DIR2	"mdldir2:"
#define KWD_INDEX	"forcedindex:"
#define KEY_COMMENT	'#'
#define SEQ_FAIL	-1

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////
#include "fops.h"

////////// Structures //////////

// List of pecache items
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
struct sPrecacheList
{
	char * List[MAX_ITEMS];			// Stores pointers to list items strings
	bool IsMap[MAX_ITEMS];			// true - map, false - model
	uchar RefCount[MAX_ITEMS];		// Number of references
	uchar ListSz = 0;				// How many items are currently in the list



	void Init()
	{
		memset(this, 0x00, sizeof(*this));
	}

	bool Add(const char * Item, bool ItemIsMap) //, uchar References
	{
		// Check if item is already on the list
		if (Find(Item) != -1)
			return false;

		// Check if list is full
		if (ListSz == MAX_ITEMS)
		{
			printf("%d item(s) limit overflow!\n", MAX_ITEMS);
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
		IsMap[ListSz] = ItemIsMap;
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

	short FindOrAdd(const char * Item, bool ItemIsMap)
	{
		// Check if item is already on the list
		short Result = Find(Item);
		if (Result != -1)
			return Result;

		// Check if list is full
		if (ListSz == MAX_ITEMS)
		{
			printf("%d item(s) limit overflow!\n", MAX_ITEMS);
			return -1;
		}

		// Find str length
		ushort Len = strlen(Item);

		// Allocate memory
		ushort Size = Len + 1;
		char * NewItem = (char *)malloc(Size);
		strcpy(NewItem, Item);

		// Add item to the list
		Result = ListSz;
		List[ListSz] = NewItem;
		IsMap[ListSz] = ItemIsMap;
		RefCount[ListSz] = 0; // References
		ListSz++;

		return Result;
	}

	// Clear list
	void Clear()
	{
		// Free memory
		for (uchar i = 0; i < ListSz; i++)
			free(List[i]);

		// Fill this structure with 0x00
		memset(this, 0x00, sizeof(*this));
	}

	uchar CountMaps()
	{
		uchar Result = 0;

		for (uchar i = 0; i < ListSz; i++)
			if (IsMap[i] == true)
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

// Simplified *.mdl model header
#define UNKNOWN_MODEL		-1
#define NOTEXTURES_MODEL	0
#define NORMAL_MODEL		1
#define SEQ_MODEL			2
#pragma pack(1)					// Eliminate unwanted 0x00 bytes
struct sModelHeader
{
	char Signature[4];			// "IDST"
	ulong Version;				// 0xA - GoldSrc model
	char Name[64];				// Internal model name
	ulong FileSize;				// Model file size
	char SomeData1[88];			// Data that is not important for conversion
	ulong SeqCount;				// How many sequences
	ulong SeqTableOffset;		// Location of sequence table 
	ulong SubmodelCount;		// How many submodels
	ulong SubmodelTableOffset;	// Location of submodel table 
	ulong TextureCount;			// How many textures
	ulong TextureTableOffset;	// Texture table location
	ulong TextureDataOffset;	// Texture data location
	ulong SkinCount;			// How many skins
	ulong SkinEntrySize;		// Size of entry in skin table (measured in shorts)
	ulong SkinTableOffset;		// Location of skin table
	ulong SubmeshCount;			// How many submeshes
	ulong SubmeshTableOffset;	// Location of submesh table
	char SomeData2[32];			// Data that is not important for conversion



	void UpdateFromFile(FILE ** ptrFile)	// Update header from file
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sModelHeader));

		// I found some models that have long non null terminated internal name string
		// that was causing creepy beeping during printf(), so there is a fix for that
		Name[63] = '\0';
	}

	int CheckModel()					// Check model type
	{
		if (this->Signature[0] == 'I' && this->Signature[1] == 'D' && this->Signature[2] == 'S' && this->Version == 0xA)
		{
			if (this->Signature[3] == 'T')
			{
				if (this->TextureCount > 0)
					return NORMAL_MODEL;
				else
					return NOTEXTURES_MODEL;
			}
			else if (this->Signature[3] == 'Q')
			{
				return SEQ_MODEL;
			}
		}

		return UNKNOWN_MODEL;
	}
};
// Simplified MDL/DOL sequence descriptor
#pragma pack(1)				// No padding/spacers
struct sModelSeq
{
	char Name[32];			// Sequence name
	char SomeData1[124];
	int Num;				// Sequence file number
	char SomeData2[16];
};

// List of source file pecache items
extern bool SetBit(uint * Input, uchar Bit);
#pragma pack(1)				// Fix unwanted 0x00 bytes in structure
class cSourceList : public sPrecacheList
{
public:
	uint Submodels[MAX_ITEMS][MAX_ITEMS];	// (dirthack, rework needed) Each bit inside element represents 
											// submodel (sequence group), each model has its own row to remember maps
	sModelSeq * SeqTabArr[MAX_ITEMS];		// Array of sequence tables
	ulong SeqTabItems[MAX_ITEMS];			// Array of item counts for sequence tables
	int Types [MAX_ITEMS];					// Model types



	// Init
	void Init()
	{
		memset(this, 0x00, sizeof(*this));
	}

	// Deinit
	void Clear()
	{
		sPrecacheList::Clear();

		// Clean sequence tables
		for (int sq = 0; sq < ListSz; sq++)
			free(SeqTabArr[sq]);

		memset(this, 0xFF, sizeof(*this));
	}

	// Add submodel
	bool SetSubmodel(short ModelIndex, char * MapName, uchar Submodel)
	{
		short MapIndex = Find(MapName);
		if (MapIndex == -1)
		{
			printf("Oops, unexpexted error: can't find map %s\n", MapName);
			exit(EXIT_FAILURE);
		}

		//if (Submodel)
			return SetBit(&Submodels[ModelIndex][MapIndex], Submodel);

		return false;
	}

	// Find or add item
	short FindOrAdd(const char * Item, bool ItemIsMap, char * ModelDir, char * ModelDir2)
	{
		short Index = sPrecacheList::FindOrAdd(Item, ItemIsMap);

		if (!IsMap[Index] && SeqTabArr[Index] == NULL)
		{
			// Read sequence table from model file

			sModelHeader ModelHeader;	// Model file header
			sModelSeq * SeqTable;		// Sequences table
			ulong SeqTableSz;			// Sequences table size

			FILE * ptrInFile;
			char FileName[PATH_LEN];
			char FullName[PATH_LEN];

			// Try to open input file in primary directory
			FileGetName(List[Index], FileName, sizeof(FileName), false);
			strcat(FileName, ".dol");		// Try .dol
			strcpy(FullName, ModelDir);
			strcat(FullName, FileName);
			SafeFileOpen(&ptrInFile, FullName, "rb");
			if (ptrInFile == NULL)
			{
				strcpy(FileName, FullName);
				FileGetFullName(FileName, FullName, sizeof(FullName));
				strcat(FullName, ".mdl");	// Try .mdl
				SafeFileOpen(&ptrInFile, FullName, "rb");
			}

			// Try to open input file in secondary directory
			if (ptrInFile == NULL)
			{
				
				FileGetName(List[Index], FileName, sizeof(FileName), false);
				strcat(FileName, ".dol");		// Try .dol
				strcpy(FullName, ModelDir2);
				strcat(FullName, FileName);
				SafeFileOpen(&ptrInFile, FullName, "rb");
				if (ptrInFile == NULL)
				{
					strcpy(FileName, FullName);
					FileGetFullName(FileName, FullName, sizeof(FullName));
					strcat(FullName, ".mdl");	// Try .mdl
					SafeFileOpen(&ptrInFile, FullName, "rb");
				}
			}

			// Can't find file - exit
			if (ptrInFile == NULL)
			{
				printf("Error: can't open file: %s \n", List[Index]);
				if (!strcmp(ModelDir, ".") || !strcmp(ModelDir2, "."))
				{
					puts("You can specify model directories by adding those lines to .ini file:");
					printf("\n%s:\nYOUR_MOD_DIR\\models\n\n%s:\nYOUR_VALVE_DIR\\models\n\n", KWD_DIR, KWD_DIR2);
				}
				puts("Press any key to exit ...");
				getchar();
				exit(EXIT_FAILURE);
			}

			// Load model header
			ModelHeader.UpdateFromFile(&ptrInFile);

			// Check model
			Types[Index] = ModelHeader.CheckModel();
			if (Types[Index] == NORMAL_MODEL || Types[Index] == NOTEXTURES_MODEL)
			{
				printf("Reading sequences from %s \nInternal name: %s \nSequences: %i \n", FullName, ModelHeader.Name, ModelHeader.SeqCount);
			}
			else
			{
				puts("Bad model file");
				exit(EXIT_FAILURE);
			}

			// Allocate memory for sequence table
			SeqTableSz = sizeof(sModelSeq) * ModelHeader.SeqCount;
			SeqTable = (sModelSeq *)malloc(SeqTableSz);
			SeqTabArr[Index] = SeqTable;
			SeqTabItems[Index] = ModelHeader.SeqCount;

			// Read sequence table
			FileReadBlock(&ptrInFile, SeqTable, ModelHeader.SeqTableOffset, SeqTableSz);

			// Close input file
			fclose(ptrInFile);
		}

		return Index;
	}

	int FindSequence(short ModelIndex, char * SeqName)
	{
		sModelSeq * SeqTab = SeqTabArr[ModelIndex];
		ulong SeqItems = SeqTabItems[ModelIndex];
		
		if (!SeqTab)
		{
			printf("Error: uninitialized sequence table in model \"%s\" \n", List[ModelIndex]);
			return -1;
		}

		int Result = SEQ_FAIL;
		for (int sq = 0; sq < SeqItems; sq++)
		{
			if (!strcmp(SeqTab[sq].Name, SeqName))
			{
				Result = SeqTab[sq].Num;
				break;
			}
		}

		if (Result == SEQ_FAIL)
			printf("Warning: can't find sequence \"%s\" in \"%s\" \n", SeqName, List[ModelIndex]);

		return Result;
	}

	void SequncesToStr(short ModelIndex, short MapIndex, char * Buf)
	{
		uint Flags = Submodels[ModelIndex][MapIndex];
		uint Flag = 1;
		int Num = 0;
		char NumBuf[10];

		strcpy(Buf, "");
		if (!Flags)
			return;

		// Convert flags to string
		while (Flag)
		{
			Flag = Flag << 1;
			Num++;

			if (Flags & Flag)
			{
				sprintf(NumBuf, "%d,", Num);
				strcat(Buf, NumBuf);
			}
		}

		// Erase ',' from string
		int Last = strlen(Buf) - 1;
		Buf[Last] = '\0';
	}
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

#endif // MAIN_H
