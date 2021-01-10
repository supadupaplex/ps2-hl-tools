// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This module contains functions that perform assembly of PS2 HL *.epc precache files
//

////////// Includes //////////
#include "util.h"
#include "main.h"				// Main header

////////// Defines /////////////

// !!! Check for dulicate models too!!!!!

////////// Functions //////////
bool AddTerminator(char * Buffer, char Symbol);	// Replace specified symbol with null terminator
bool FindSymbol(char * Buffer, char Symbol);	// Find symbol in string
uint SetSubmodels(char * Buffer);				// Set up submodels flags
bool SetBit(uint * Input, uchar Bit);			// Set bit in 4-byte variable
bool ValidateInputFile(const char * cFile);		// Validate input file
bool TranslateInputFile(const char * cFile);	// Translate input file
bool TranslateSourceFile(const char * cFile);	// Translate source file obtained from "YA PS2HL" mod

///////// Variables /////////


///////// Code /////////
bool AddTerminator(char * Buffer, char Symbol)
{
	bool Result = false;

	ushort Len = strlen(Buffer);
	for (ushort i = 0; i < Len; i++)
	{
		if (Buffer[i] == Symbol)
		{
			Buffer[i] = '\0';
			Result = true;
		}
	}
	
	return Result;
}

bool FindSymbol(char * Buffer, char Symbol)
{
	bool Result = false;

	ushort Len = strlen(Buffer);
	for (ushort i = 0; i < Len; i++)
		if (Buffer[i] == Symbol)
			Result = true;
	
	return Result;
}

uint SetSubmodels(char * Buffer)
{
	uint Result = 0;

	// Find string length
	ushort Len = strlen(Buffer);

	// Find set up submodels flags
	char NumBuffer[80];
	bool NumFound = false;
	uchar NumStart = 0;
	ulong Number;
	for (ushort i = 0; i < Len; i++)
	{
		if (Buffer[i] == '[')
		{
			NumStart = i + 1;
		}
		else if (Buffer[i] == ',' || Buffer[i] == ']')
		{
			// Clear buffer
			memset(NumBuffer, 0x00, sizeof(NumBuffer));

			if ((i - NumStart) != 0)
			{
				// Get submodel number from string
				memcpy(NumBuffer, &Buffer[NumStart], i - NumStart);
				sscanf(NumBuffer, "%d", &Number);

				// Give warning if submodel number is wrong
				if (Number > 31)
				{
					puts("Waning: found submodel number > 31, ignoring.\n");
					UTIL_WAIT_KEY("Press any key to confirm ...");
					continue;
				}
				else if (Number == 0)
				{
					puts("Waning: submodel number 0 can result in unforseen consequences.\n");
					UTIL_WAIT_KEY("Press any key to confirm ...");
				}

				// Set up submodel flag
				SetBit(&Result, (uchar) Number);

				// Current end = next start
				NumStart = i + 1;
			}
		}

		if (Buffer[i] == ']')
		{
			break;
		}
	}
	
	return Result;
}

bool SetBit(uint * Input, uchar Bit)
{
	uint Mask = 1;

	// Check bounds
	if (Bit > 31)
		return false;

	// Set bit
	Mask = Mask << Bit;
	*Input |= Mask;
	
	return true;
}

bool ValidateInputFile(const char * cFile)
{
	FILE * ptrInputF;		// Input file stream
	char Buffer[128];		// Text buffer
	ushort Open = 0;		// Open brackets count
	ushort Close = 0;		// Close brackets count

	// Open file
	SafeFileOpen(&ptrInputF, cFile, "rb");

	// Count '{' and '}'
	Open = Close = 0;
	while (fgets(Buffer, sizeof(Buffer), ptrInputF) != NULL)
	{
		AddTerminator(Buffer, '\r');
		AddTerminator(Buffer, '\n');
		if (!strcmp(Buffer, "{") == true)
		{
			Open++;
		}
		if (!strcmp(Buffer, "}") == true)
		{
			if (Open == (Close + 1))
			{
				Close++;
			}
			else
			{
				puts("Invalid {} order !");
				fclose(ptrInputF);
				return false;
			}
		}
	}

	// Check for empty
	if (Open == 0 && Close == 0)
	{
		puts("Found zero maps ...");
		fclose(ptrInputF);
		return false;
	}

	// Check for parity
	if (Open != Close)
	{
		puts("Number of \'{\' not matches \'}\' !");
		fclose(ptrInputF);
		return false;
	}

	printf("Found %d maps \n", Open);
	
	// Rewind file pointer
	fseek(ptrInputF, 0, SEEK_SET);

	// Count '[' and ']'
	Open = Close = 0;
	while (fgets(Buffer, sizeof(Buffer), ptrInputF) != NULL)
	{
		ushort Len = strlen(Buffer);

		for (ushort i = 0; i < Len; i++)
		{
			if (Buffer[i] == '[')
			{
				Open++;
			}
			
			if (Buffer[i] == ']')
			{
				if (Open == (Close + 1))
				{
					Close++;
				}
				else
				{
					puts("Invalid [] order !");
					fclose(ptrInputF);
					return false;
				}
			}
		}

		if (Open != Close)
		{
					puts("Both \'[\' and \']\' should be on one line !");
					fclose(ptrInputF);
					return false;
		}
	}

	// Check for empty
	if (Open == 0 && Close == 0)
	{
		puts("Found zero models ...");
		fclose(ptrInputF);
		return false;
	}

	// Check for parity
	if (Open != Close)
	{
		puts("Number of \'[\' not matches \']\' ...");
		fclose(ptrInputF);
		return false;
	}

	printf("Found %d model references \n", Open);

	// Close file
	fclose(ptrInputF);

	return true;
}

bool TranslateInputFile(const char * cFile)
{
	FILE * ptrInputF;					// Input file stream
	FILE * ptrOutputF;					// Output file stream
	char Buffer[128] = "Text";			// Text buffer
	char PrevBuffer[128] = "Text";		// Previous state of text buffer
	uint ItemCnt;						// Open brackets count
	uint MapCnt;						// Close brackets count
	uchar RefCount;						// Number of model references
	uchar * MapRefCount;				// Pointer to map model reference count
	bool Map = true;					// Map or model
	sPrecacheList List;					// Item list
	sPS2HL_EPCMapEntry MapEntry;		// Map entry
	sPS2HL_EPCModelEntry ModelEntry;	// Model entry

	//// Read items ////

	// Open input file
	SafeFileOpen(&ptrInputF, cFile, "rb");

	// Fetch items from file
	Map = true;
	RefCount = 0;
	while (fgets(Buffer, sizeof(Buffer), ptrInputF) != NULL)
	{
		AddTerminator(Buffer, '\r');
		AddTerminator(Buffer, '\n');

		if (!strcmp(Buffer, "{") == true)
		{
			if (List.Add(PrevBuffer, Map) == true)						// Add map
			{
				Map = false;
				MapRefCount = &List.RefCount[List.ListSz - 1];
			}
			else
			{
				printf("Error: found at least two identical maps. \nMap name: %s \n", PrevBuffer);
				fclose(ptrInputF);
				return false;
			}
		}
		else if (!strcmp(Buffer, "}") == true)
		{
			Map = true;
			*MapRefCount = RefCount;
			RefCount = 0;
		}
		else
		{
			if (Map == false)
			{
				if (AddTerminator(Buffer, '[') == true)
				{
					RefCount++;
					List.Add(Buffer, Map);					// Add model
				}
			}
		}

		strcpy(PrevBuffer, Buffer);
	}

	//// Write item list ////

	puts("Writing item List ...");

	// Open output file
	char OutFileName[PATH_LEN];
	FileGetPath(cFile, OutFileName, sizeof(OutFileName));
	strcat(OutFileName, "extraprecache.epc");
	SafeFileOpen(&ptrOutputF, OutFileName, "wb");

	// Write item count
	ItemCnt = List.ListSz;
	FileWriteBlock(&ptrOutputF, &ItemCnt, sizeof(ItemCnt));
	
	// Wtrite strings
	for (uint i = 0; i < List.ListSz; i++)
	{
		// Allocate memory
		uint Size = (strlen(List.List[i]) / PS2HL_EPC_ALLIGNMENT + 1) * PS2HL_EPC_ALLIGNMENT;
		char * ItemBuf = (char *) malloc(Size);
		memset(ItemBuf, 0xFD, Size);
		strcpy(ItemBuf, List.List[i]);

		// Write size
		FileWriteBlock(&ptrOutputF, &Size, sizeof(Size));

		// Write string
		FileWriteBlock(&ptrOutputF, ItemBuf, Size);

		// Free mamory
		free(ItemBuf);
	}

	// Write map count
	MapCnt = List.CountMaps();
	FileWriteBlock(&ptrOutputF, &MapCnt, sizeof(MapCnt));
	
	//// Read connections ////
	
	puts("Writing connections ...\n");

	// Rewind input file pointer
	fseek(ptrInputF, 0, SEEK_SET);

	// Read connections
	Map = true;
	int Result = 0;
	while (fgets(Buffer, sizeof(Buffer), ptrInputF) != NULL)
	{
		AddTerminator(Buffer, '\r');
		AddTerminator(Buffer, '\n');

		if (!strcmp(Buffer, "{") == true)
		{
			// Find map index
			Result = List.Find(PrevBuffer);
			if (Result == -1)
			{
				puts("Unexpected error: map isn't found in the internal list ...");
				fclose(ptrInputF);
				fclose(ptrOutputF);
				return false;
			}

			// Set up map entry
			MapEntry.MapIndex = Result;
			MapEntry.ModelsCount = List.RefCount[Result];

			// Write map entry to file
			FileWriteBlock(&ptrOutputF, &MapEntry, sizeof(MapEntry));

			Map = false;
		}
		else if (!strcmp(Buffer, "}") == true)
		{
			Map = true;
		}
		else
		{
			if (Map == false)
			{
				// Find model index
				strcpy(PrevBuffer, Buffer);
				if (AddTerminator(PrevBuffer, '[') == false)	// Continue if line isn't recognised as model entry
					continue;
				Result = List.Find(PrevBuffer);
				if (Result == -1)
				{
					puts("Unexpected error: model isn't found in the internal list ...");
					fclose(ptrInputF);
					fclose(ptrOutputF);
					return false;
				}

				// Set up model entry
				ModelEntry.ModelIndex = Result;
				ModelEntry.Flag = FindSymbol(Buffer, '!');
				ModelEntry.Submodels = SetSubmodels(Buffer);

				// Write model entry to file
				FileWriteBlock(&ptrOutputF, &ModelEntry, sizeof(ModelEntry));

				Map = false;
			}
		}

		strcpy(PrevBuffer, Buffer);
	}

	// Close input file
	fclose(ptrInputF);

	// Close output file
	fclose(ptrOutputF);

	return true;
}

bool TranslateSourceFile(const char * cFile)
{
	sModelHeader ModelHeader;	// Model file header
	sModelSeq * SeqTable;		// Sequences table
	ulong SeqTableSz;			// Sequences table size
	int SeqCount;				// Sequences count
	FILE * ptrInFile;			// Input file
	FILE * ptrOutFile;			// Output file
	char cOutFileName[PATH_LEN]; // Output file name

	cSourceList srcList;		// List to store references
	char LineBuf[PATH_LEN];		// Text line buffer

	char Dir1[PATH_LEN] = ".";
	char Dir2[PATH_LEN] = ".";
	short MapIndex = -1;
	short ModelIndex = -1;
	char cSeqBuf[PATH_LEN] = "";
	char cSeqNumBuf[PATH_LEN] = "";

	// Init list
	srcList.Init();

	// Open input file
	SafeFileOpen(&ptrInFile, cFile, "r");
	fseek(ptrInFile, 0, SEEK_SET);

	// Parse file
	while (!feof(ptrInFile))
	{
		// Read line
		fgets(LineBuf, sizeof(LineBuf), ptrInFile);
		AddTerminator(LineBuf, '\n');

		// Check line
		if (LineBuf[0] == KEY_COMMENT || LineBuf[0] == '\0')
			continue;
		else if (!strcmp(LineBuf, KWD_SEQ))
		{
			// Sequence
			if (ModelIndex == -1)
			{
				puts("Error: sequence before model");
				continue;
			}
			if (MapIndex == -1)
			{
				puts("Error: sequence before map");
				continue;
			}

			fgets(LineBuf, sizeof(LineBuf), ptrInFile);
			AddTerminator(LineBuf, '\n');
			if (LineBuf[0] != '\0')
			{
				int Submodel = srcList.FindSequence(ModelIndex, LineBuf);
				if (Submodel == -1)
				{
					puts("Error adding sequence");
					continue;
				}
				if (Submodel)
					srcList.SetSubmodel(ModelIndex, srcList.List[MapIndex], Submodel);
			}
		}
		else if (!strcmp(LineBuf, KWD_INDEX))
		{
			// Submodel index
			if (ModelIndex == -1)
			{
				puts("Error: submodel before model");
				continue;
			}
			if (MapIndex == -1)
			{
				puts("Error: submodel before map");
				continue;
			}

			fgets(LineBuf, sizeof(LineBuf), ptrInFile);
			AddTerminator(LineBuf, '\n');
			if (LineBuf[0] != '\0')
			{
				int Submodel = atoi(LineBuf);
				if (Submodel)
					srcList.SetSubmodel(ModelIndex, srcList.List[MapIndex], Submodel);
			}
		}
		else if (!strcmp(LineBuf, KWD_MODEL))
		{
			// Model
			if (MapIndex == -1)
			{
				puts("Error: model before map");
				continue;
			}

			fgets(LineBuf, sizeof(LineBuf), ptrInFile);
			AddTerminator(LineBuf, '\n');
			if (LineBuf[0] != '\0')
			{
				ModelIndex = srcList.FindOrAdd(LineBuf, false, Dir1, Dir2);
				if (ModelIndex == -1)
					puts("Error adding model");
				else
					if (srcList.Types[ModelIndex] == NOTEXTURES_MODEL)
						srcList.SetSubmodel(ModelIndex, srcList.List[MapIndex], 0);	// Mark texture submodel
			}
		}
		else if (!strcmp(LineBuf, KWD_MAP))
		{
			fgets(LineBuf, sizeof(LineBuf), ptrInFile);
			AddTerminator(LineBuf, '\n');
			if (LineBuf[0] != '\0')
			{
				MapIndex = srcList.FindOrAdd(LineBuf, true, Dir1, Dir2);
				if (MapIndex == -1)
					puts("Error adding map");
			}
		}
		else if (!strcmp(LineBuf, KWD_DIR))
		{
			fgets(Dir1, sizeof(Dir1), ptrInFile);
			AddTerminator(Dir1, '\n');
			strcat(Dir1, DIR_DELIM);
		}
		else if (!strcmp(LineBuf, KWD_DIR2))
		{
			fgets(Dir2, sizeof(Dir2), ptrInFile);
			AddTerminator(Dir2, '\n');
			strcat(Dir2, DIR_DELIM);
		}
	}

	// Close input file
	fclose(ptrInFile);

	// Open output file
	FileGetFullName(cFile, cOutFileName, sizeof(cOutFileName));
	strcat(cOutFileName, ".txt");
	SafeFileOpen(&ptrOutFile, cOutFileName, "w");

	// Write output file
	for (short map = 0; map < srcList.ListSz; map++)
	{
		// Skip models
		if (srcList.IsMap[map] == false)
			continue;

		// Write map entry
		fprintf(ptrOutFile, "%s\n{\n", srcList.List[map]);

		// Write models
		for (short mdl = 0; mdl < srcList.ListSz; mdl++)
		{
			// Skip maps
			if (srcList.IsMap[mdl] == true)
				continue;

			if (srcList.Submodels[mdl][map] != 0)
			{
				// Write main model and indexes
				if ((srcList.Submodels[mdl][map] & (~1)))
				{
					FileGetFullName(srcList.List[mdl], cSeqBuf, sizeof(cSeqBuf));
					strcat(cSeqBuf, ".dol[");
					srcList.SequncesToStr(mdl, map, cSeqNumBuf);
					strcat(cSeqBuf, cSeqNumBuf);
					strcat(cSeqBuf, "]");
					fprintf(ptrOutFile, "%s\n", cSeqBuf);
				}

				// Include textures if model is textureless
				if ( (srcList.Submodels[mdl][map] & 1) )
				{
					FileGetFullName(srcList.List[mdl], cSeqBuf, sizeof(cSeqBuf));
					strcat(cSeqBuf, "t.dol[]");
					fprintf(ptrOutFile, "%s\n", cSeqBuf);
				}
			}
		}

		// Close map entry
		fprintf(ptrOutFile, "}\n\n", srcList.List[map]);
	}

	// Close output file
	fclose(ptrOutFile);

	// Free memory
	srcList.Clear();

	puts("Done!\n\n");
	return true;
}

int main(int argc, char * argv[])
{
	char cExtension[5];

	puts(PROG_TITLE);

	if (argc == 1)
	{
		puts(PROG_INFO);
		UTIL_WAIT_KEY("Press any key to exit ...");
	}
	else if (argc == 2)
	{
		FileGetExtension(argv[1], cExtension, sizeof(cExtension));

		if (!strcmp(cExtension, ".txt") == true)
		{
			printf("Processing file: %s \n", argv[1]);

			if (ValidateInputFile(argv[1]) == true)			// File is OK
			{
				if (TranslateInputFile(argv[1]) == true)
				{
					puts("Done! \n");
					return 0;
				}
				else
				{
					puts("Translation error! \n");
				}
			}
			else											// Bad file
			{
				puts("Validation failed! \n");
			}
		}
		if (!strcmp(cExtension, ".inf") == true)
		{
			if (TranslateSourceFile(argv[1]) == true)
				puts("Done! \n");
			else
				puts("Translation error! \n");
		}
		else												// Unsupported file
		{
			puts("Unsupported file extension ...");
		}
	}
	else
	{
		puts("Too many arguments ...");
	}

	//getch();
	
	return 1;
}
