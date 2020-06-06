/*
=====================================================================
Copyright (c) 2017-2018, Alexey Leushin
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
// This file contains model conversion and texture extraction functions
//

////////// Includes //////////
#include "main.h"

////////// Global variables //////////


////////// Functions //////////
uint PSIProperSize(uint Size, bool ToLower);																		// Calculate nearest appropriate size of PS2 DOL texture
void ExtractDOLTextures(const char * FileName);																		// Extract textures from PS2 model
void ExtractMDLTextures(const char * FileName);																		// Extract textures from PC model
void ConvertMDLToDOL(const char * FileName);																		// Convert model from PC to PS2 format
void ConvertDOLToMDL(const char * FileName);																		// Convert model from PS2 to PC format
void ConvertSubmodel(const char * FileName, char * OriginalExtension, char * TargetExtension);						// Convert submodel
void ConvertDummySubmodel(const char * FileName, char * OriginalExtension, char * TargetExtension);					// Convert submodel which consists of signature and name only
void GetExtraDOLData(const char * FileName);																		// Extract extra data from DOL model
bool AddTerminator(char * Buffer, char Symbol);																		// Helper for CheckExtraFile()
ushort CountSymbols(char * Buffer, char Symbol);																	// Counts symbols in line
bool CheckExtraFile(const char * FileName);																			// Check if extra *.INF file is valid
void GetValues(char * Buffer, ulong * Values, uchar ValuesCount);													// Helper for TranslateExtraFile()
bool TranslateExtraFile(const char * FileName, sDOLExtraSection * DOLExtraSect, sDOLLODEntry ** LODTable);			// Fetch data from extra *.INF file
void PatchSubmodelRef(sModelHeader * MdlHdr, char * ModelData, ulong ModelDataSize, char * NewExtension);			// Patch internal submodel references
int CheckModel(const char * FileName);																				// Check model type
void PatchDOLExtraSection(char * ModelData, ulong ModelDataSize, ulong LODDataOffseet, uchar MaxBodyParts, uchar NumBodyGroups, ulong FadeStart, ulong FadeEnd);
		// Write extra data to DOL model file (this is needed to allow correct body part part switching and to stop crashing on PS2).
void SeqReport(const char * FileName);																				// Sequence report


// Write extra data to DOL model file (this is needed to allow correct body part switching and to stop crashing on PS2)
void PatchDOLExtraSection(char * ModelData, ulong ModelDataSize, ulong LODDataOffseet, uchar MaxBodyParts, uchar NumBodyGroups, ulong FadeStart, ulong FadeEnd)
{
	sDOLExtraSection * DOLExtraSection;

	// Check if model is empty
	if (ModelDataSize < sizeof(sDOLExtraSection))
		return;
	
	// Set pointer to a begining of extra section
	DOLExtraSection = (sDOLExtraSection *)ModelData;

	// Write data
	DOLExtraSection->LODDataOffset = LODDataOffseet;
	DOLExtraSection->MaxBodyParts = MaxBodyParts;
	DOLExtraSection->NumBodyGroups = NumBodyGroups;
	DOLExtraSection->Magic[0] = 0;
	DOLExtraSection->Magic[1] = 0;
	DOLExtraSection->FadeStart = FadeStart;
	if (FadeEnd >= FadeStart)
		DOLExtraSection->FadeEnd = FadeEnd;
	else
		DOLExtraSection->FadeEnd = FadeStart;
}

int CheckModel(const char * FileName)	// Check model type
{
	FILE * ptrModelFile;

	sModelHeader ModelHeader;

	int ModelType;

	// Open model file
	SafeFileOpen(&ptrModelFile, FileName, "rb");

	// Check for dummy model (Signature, Name and FileSize only)
	if (FileSize(&ptrModelFile) < sizeof(sModelHeader))
	{
		ModelType = DUMMY_MODEL;
	}
	else
	{
		// Load header
		ModelHeader.UpdateFromFile(&ptrModelFile);

		// Get model type
		ModelType = ModelHeader.CheckModel();
	}

	fclose(ptrModelFile);

	return ModelType;
}

void ConvertDOLToMDL(const char * FileName)		// Convert model from PS2 to PC format 
{
	sModelHeader ModelHeader;					// Model file header
	sModelTextureEntry * ModelTextureTable;		// Model texture table
	ulong ModelTextureTableSize;				// Model texture table size (how many textures)
	sTexture * Textures;						// Pointer to textures data

	FILE * ptrInFile;
	char cNewModelName[64];
	FILE * ptrOutFile;
	char cOutFileName[255];

	ulong ModelSize;

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Get header from file
	ModelHeader.UpdateFromFile(&ptrInFile);

	// Check model
	if (ModelHeader.CheckModel() == NORMAL_MODEL)
	{
		printf("Internal name: %s \nTextures: %i, Texture table offset: 0x%X \n", ModelHeader.Name, ModelHeader.TextureCount, ModelHeader.TextureTableOffset);
	}
	else
	{
		puts("Incorrect model file.");
		return;
	}

	// Save extra *.DOL data to *.INF file
	if (ModelHeader.TextureTableOffset - sizeof(sModelHeader) > sizeof(sDOLExtraSection))	// Do not extract data from texture submodels
		GetExtraDOLData(FileName);

	// Allocate memory for textures
	ModelTextureTableSize = ModelHeader.TextureCount * sizeof(sModelTextureEntry);
	ModelTextureTable = (sModelTextureEntry *)malloc(ModelTextureTableSize);
	Textures = (sTexture *)malloc(sizeof(sTexture) * ModelHeader.TextureCount);

	// Load and convert textures
	uint BitmapOffset;
	uint BitmapSize;
	uint PaletteOffset;
	uint PaletteSize;
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		ModelTextureTable[i].UpdateFromFile(&ptrInFile, ModelHeader.TextureTableOffset, i);
		//printf(" Texture #%i \n Name: %s \n Width: %i \n Height: %i \n Offset: 0x%X \n\n", i + 1, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height, ModelTextureTable[i].Offset);

		BitmapOffset = ModelTextureTable[i].Offset + DOL_TEXTURE_HEADER_SIZE + EIGHT_BIT_PALETTE_ELEMENTS_COUNT * DOL_BMP_PALETTE_ELEMENT_SIZE;
		BitmapSize = ModelTextureTable[i].Height * ModelTextureTable[i].Width;
		PaletteOffset = ModelTextureTable[i].Offset + DOL_TEXTURE_HEADER_SIZE;
		PaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * DOL_BMP_PALETTE_ELEMENT_SIZE;

		// Load texture
		Textures[i].Initialize();
		Textures[i].UpdateFromFile(&ptrInFile, BitmapOffset, BitmapSize, PaletteOffset, PaletteSize, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height);
		
		// Convert texture
		Textures[i].PaletteReformat(DOL_BMP_PALETTE_ELEMENT_SIZE);
		Textures[i].PaletteRemoveSpacers();
	}

	// Write results to output file
	// Open output file
	FileGetFullName(FileName, cOutFileName, sizeof(cOutFileName));
	strcat(cOutFileName, ".mdl");
	SafeFileOpen(&ptrOutFile, cOutFileName, "wb");

	// Write modified header
	FileGetName(cOutFileName, cNewModelName, sizeof(cNewModelName), false);
	strcat(cNewModelName, ".mdl");
	ModelHeader.Rename(cNewModelName);
	ModelHeader.TextureDataOffset = ModelHeader.TextureTableOffset + sizeof(sModelTextureEntry) * ModelHeader.TextureCount + ModelHeader.SkinCount * ModelHeader.SkinEntrySize * 2; // HotFix
	FileWriteBlock(&ptrOutFile, (char *)&ModelHeader, sizeof(sModelHeader));

	// Write patched model data
	uchar * ModelData;
	ModelData = (uchar *)malloc(ModelHeader.TextureTableOffset - sizeof(sModelHeader));
	FileReadBlock(&ptrInFile, (char *)ModelData, sizeof(sModelHeader), ModelHeader.TextureTableOffset - sizeof(sModelHeader));
	PatchDOLExtraSection((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), 0x00504453, 0, 0, 0, 0);		// Clear extra field
	PatchSubmodelRef(&ModelHeader, (char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), ".mdl");			// Patch internal submodel references
	FileWriteBlock(&ptrOutFile, (char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader));
	free(ModelData);

	// Write modified texture table
	uint Offset = ModelHeader.TextureTableOffset + sizeof(sModelTextureEntry) * ModelHeader.TextureCount + ModelHeader.SkinCount * ModelHeader.SkinEntrySize * 2;
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		ModelTextureTable[i].Width = Textures[i].Width;
		ModelTextureTable[i].Height = Textures[i].Height;
		ModelTextureTable[i].Offset = Offset;

		Offset += Textures[i].Width * Textures[i].Height + Textures[i].PaletteSize;
	}
	FileWriteBlock(&ptrOutFile, (char *)ModelTextureTable, ModelTextureTableSize);

	// Write skin data
	uchar * SkinTable;
	ulong SkinTableSize = ModelHeader.SkinCount * ModelHeader.SkinEntrySize * 2;
	SkinTable = (uchar *)malloc(SkinTableSize);
	FileReadBlock(&ptrInFile, SkinTable, ModelHeader.SkinTableOffset, SkinTableSize);
	FileWriteBlock(&ptrOutFile, SkinTable, SkinTableSize);
	free(SkinTable);

	// Write textures
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		FileWriteBlock(&ptrOutFile, (char *)Textures[i].Bitmap, Textures[i].Width * Textures[i].Height);		
		FileWriteBlock(&ptrOutFile, (char *)Textures[i].Palette, Textures[i].PaletteSize);
	}

	// Update model size field
	ModelSize = FileSize(&ptrOutFile);
	FileWriteBlock(&ptrOutFile, &ModelSize, 0x48, sizeof(ModelSize));	// 0x48 - address of model size field

	// Free memory
	free(ModelTextureTable);
	free(Textures);
	
	// Close files
	fclose(ptrInFile);
	fclose(ptrOutFile);

	puts("Done!\n\n");
}

void ConvertMDLToDOL(const char * FileName)	// Convert model from PC to PS2 format 
{
	sModelHeader ModelHeader;					// Model file header
	sModelTextureEntry * ModelTextureTable;		// Model texture table
	ulong ModelTextureTableSize;				// Model texture table size (how many textures)
	sDOLTextureHeader DOLTextureHeader;			// DOL Texture Header
	sTexture * Textures;						// Pointer to textures data
	
	FILE * ptrInFile;
	FILE * ptrOutFile;
	char cOutFileName[255];
	char cNewModelName[64];
	char cTextureName[64];

	ulong ModelSize;

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Get header from file
	ModelHeader.UpdateFromFile(&ptrInFile);

	// Check model
	if (ModelHeader.CheckModel() == NORMAL_MODEL)
	{
		printf("Internal name: %s \nTextures: %i, Texture table offset: 0x%X \n", ModelHeader.Name, ModelHeader.TextureCount, ModelHeader.TextureTableOffset);
	}
	else
	{
		puts("Incorrect model file.");
		return;
	}

	// Allocate memory for texture tables
	ModelTextureTableSize = ModelHeader.TextureCount * sizeof(sModelTextureEntry);
	ModelTextureTable = (sModelTextureEntry *)malloc(ModelTextureTableSize);
	Textures = (sTexture *)malloc(sizeof(sTexture) * ModelHeader.TextureCount);

	// Convert textures
	uint BitmapOffset;
	uint BitmapSize;
	uint PaletteOffset;
	uint PaletteSize;
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		ModelTextureTable[i].UpdateFromFile(&ptrInFile, ModelHeader.TextureTableOffset, i);
		//printf(" Texture #%i \n Name: %s \n Width: %i \n Height: %i \n Offset: 0x%X \n\n", i + 1, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height, ModelTextureTable[i].Offset);

		// PVR check
		char TexExtension[5];
		FileGetExtension(ModelTextureTable[i].Name, TexExtension, sizeof(TexExtension));
		if (!strcmp(TexExtension, ".pvr") == true)
		{
			puts("Dreamcast HL model conversion is not suppotred ...");
			getch();
			exit(EXIT_FAILURE);
		}

		BitmapOffset = ModelTextureTable[i].Offset + MDL_TEXTURE_HEADER_SIZE;
		BitmapSize = ModelTextureTable[i].Height * ModelTextureTable[i].Width;
		PaletteOffset = ModelTextureTable[i].Offset + ModelTextureTable[i].Width * ModelTextureTable[i].Height;
		PaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * MDL_PALETTE_ELEMENT_SIZE;

		// Load texture
		Textures[i].Initialize();
		Textures[i].UpdateFromFile(&ptrInFile, BitmapOffset, BitmapSize, PaletteOffset, PaletteSize, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height);
		
		// Resize texture
		Textures[i].TileResize(PSIProperSize(Textures[i].Width, false), PSIProperSize(Textures[i].Height, false));
		
		// Convert texture
		Textures[i].PaletteReformat(MDL_PALETTE_ELEMENT_SIZE);
		Textures[i].PaletteAddSpacers(0x80);
	}

	// Write results to output file
	// Open output file
	FileGetFullName(FileName, cOutFileName, sizeof(cOutFileName));
	strcat(cOutFileName, ".dol");
	SafeFileOpen(&ptrOutFile, cOutFileName, "wb");

	// Write modified header
	FileGetName(cOutFileName, cNewModelName, sizeof(cNewModelName), false);
	strcat(cNewModelName, ".dol");
	ModelHeader.Rename(cNewModelName);
	ModelHeader.TextureDataOffset = ModelHeader.TextureTableOffset + sizeof(sModelTextureEntry) * ModelHeader.TextureCount + ModelHeader.SkinCount * ModelHeader.SkinEntrySize * 2; // HotFix
	ModelHeader.TextureDataOffset = (((ModelHeader.TextureDataOffset / 16) + ((ModelHeader.TextureDataOffset % 16) && 1)) * 16); // Fix for hotfix
	FileWriteBlock(&ptrOutFile, (char *)&ModelHeader, sizeof(sModelHeader));

	// Write patched model data
	uchar * ModelData;
	ModelData = (uchar *) malloc(ModelHeader.TextureTableOffset - sizeof(sModelHeader));
	FileReadBlock(&ptrInFile, (char *) ModelData, sizeof(sModelHeader), ModelHeader.TextureTableOffset - sizeof(sModelHeader));
	PatchDOLExtraSection((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), 0, 0, 0, 0, 0);			// Reset extra section to it's default state
	PatchSubmodelRef(&ModelHeader, (char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), ".dol");		// Patch internal submodel references
	FileWriteBlock(&ptrOutFile, (char *) ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader));
	free(ModelData);

	// Write modified texture table
	uint Offset = ModelHeader.TextureTableOffset + sizeof(sModelTextureEntry) * ModelHeader.TextureCount + ModelHeader.SkinCount * ModelHeader.SkinEntrySize * 2;
	Offset = ((Offset / 16) + ((Offset % 16) && 1)) * 16; // Hotfix
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		ModelTextureTable[i].Width = Textures[i].Width;
		ModelTextureTable[i].Height = Textures[i].Height;
		ModelTextureTable[i].Offset = Offset;

		Offset += sizeof(sDOLTextureHeader) + Textures[i].PaletteSize + Textures[i].Width * Textures[i].Height;
	}
	FileWriteBlock(&ptrOutFile, (char *) ModelTextureTable, ModelTextureTableSize);

	// Write skin data
	uchar * SkinTable;
	ulong SkinTableSize = ModelHeader.SkinCount * ModelHeader.SkinEntrySize * 2;
	SkinTable = (uchar *)malloc(SkinTableSize);
	FileReadBlock(&ptrInFile, SkinTable, ModelHeader.SkinTableOffset, SkinTableSize);
	FileWriteBlock(&ptrOutFile, SkinTable, SkinTableSize);
	free(SkinTable);

	// Write blank bytes to fill 16-byte block (PS2 HL likes everything to be alligned)
	char Spacer = 0x00;
	int SpacersCount = (FileSize(&ptrOutFile) / 16 + ((FileSize(&ptrOutFile) % 16) && 1)) * 16 - FileSize(&ptrOutFile);	// Fix for hotfix
	for (int i = 0; i < SpacersCount ; i++)
		FileWriteBlock(&ptrOutFile, &Spacer, 1);

	// Write textures
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		// Remove ".bmp" in texture name
		FileGetName(Textures[i].Name, cTextureName, sizeof(cTextureName), false);
		DOLTextureHeader.Update(cTextureName, Textures[i].Width, Textures[i].Height);

		FileWriteBlock(&ptrOutFile, &DOLTextureHeader, sizeof(sDOLTextureHeader));
		FileWriteBlock(&ptrOutFile, (char *) Textures[i].Palette, Textures[i].PaletteSize);
		FileWriteBlock(&ptrOutFile, (char *) Textures[i].Bitmap, Textures[i].Width * Textures[i].Height);
	}

	// Fetch data from external *.INI file (if present) and write it to DOL file
	sDOLExtraSection DOLXS;
	sDOLLODEntry * LODTable;
	if (CheckExtraFile(FileName) == true)
	{
		// Get data from *.INI
		TranslateExtraFile(FileName, &DOLXS, &LODTable);

		// Rewrite extra section
		DOLXS.LODDataOffset = FileSize(&ptrOutFile);
		FileWriteBlock(&ptrOutFile, &DOLXS, sizeof(sModelHeader), sizeof(DOLXS));

		// Write LOD table
		if (LODTable != NULL)
		{
			FileWriteBlock(&ptrOutFile, LODTable, FileSize(&ptrOutFile), DOLXS.NumBodyGroups * DOLXS.MaxBodyParts * sizeof(sDOLLODEntry));
			free(LODTable);
		}

		// Align data
		uchar Align = (FileSize(&ptrOutFile) % 16) == 0 ? 0 : 16 - (FileSize(&ptrOutFile) % 16);
		for (uchar i = 0; i < Align; i++)
			fputc(0x11, ptrOutFile);
	}

	// Update model size field
	ModelSize = FileSize(&ptrOutFile);
	FileWriteBlock(&ptrOutFile, &ModelSize, 0x48, sizeof(ModelSize));	// 0x48 - address of model size field

	// Free memory
	free(ModelTextureTable);
	free(Textures);

	// Close files
	fclose(ptrInFile);
	fclose(ptrOutFile);

	puts("Done!\n\n");
}

void ConvertSubmodel(const char * FileName, char * OriginalExtension, char * TargetExtension)	// Convert submodel
{
	FILE * ptrModelFile;
	FILE * ptrOutputFile;

	sModelHeader ModelHeader;
	char * ModelData;
	ulong ModelDataSize;

	int ModelType;
	char NewModelName[64];
	char OutputFile[255];

	puts("Patching submodel ...");

	// Open model file
	SafeFileOpen(&ptrModelFile, FileName, "rb");

	// Load header
	ModelHeader.UpdateFromFile(&ptrModelFile);

	// Check header
	if (ModelHeader.CheckModel() != NOTEXTURES_MODEL && ModelHeader.CheckModel() != SEQ_MODEL)
	{
		puts("Invalid submodel ...");
		return;
	}

	// Create new model file
	FileGetFullName(FileName, OutputFile, sizeof(OutputFile));
	strcat(OutputFile, TargetExtension);
	SafeFileOpen(&ptrOutputFile, OutputFile, "wb");

	// Update and write model header
	FileGetName(FileName, NewModelName, sizeof(NewModelName), false);
	strcat(NewModelName, TargetExtension);
	ModelHeader.Rename(NewModelName);
	FileWriteBlock(&ptrOutputFile, (char *)&ModelHeader, sizeof(sModelHeader));

	// Write patched model data
	ModelDataSize = FileSize(&ptrModelFile) - sizeof(sModelHeader);
	ModelData = (char *)malloc(ModelDataSize);
	FileReadBlock(&ptrModelFile, ModelData, sizeof(sModelHeader), ModelDataSize);
	if (ModelHeader.CheckModel() == NOTEXTURES_MODEL)	// Apply patch to "IDST" models only
	{
		// Patch references
		PatchSubmodelRef(&ModelHeader, ModelData, ModelDataSize, TargetExtension);

		// Clear extra field
		if (!strcmp(OriginalExtension, ".mdl") == true)
			PatchDOLExtraSection((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), 0, 0, 0, 0, 0);
		else
			PatchDOLExtraSection((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), 0x00504453, 0, 0, 0, 0);
	}
	FileWriteBlock(&ptrOutputFile, ModelData, ModelDataSize);

	// Free memory
	free(ModelData);

	// Close file
	fclose(ptrModelFile);

	puts("Done!\n\n");
}

void ConvertDummySubmodel(const char * FileName, char * OriginalExtension, char * TargetExtension)	// Convert submodel which consists of signature and name only
{
	FILE * ptrModelFile;
	FILE * ptrOutputFile;

	char * ModelData;
	ulong ModelDataSize;

	char OutputFile[255];
	char NewInternalName[64];

	puts("Patching dummy submodel ...");

	// Open model file
	SafeFileOpen(&ptrModelFile, FileName, "rb");

	// Create new model file
	FileGetFullName(FileName, OutputFile, sizeof(OutputFile));
	strcat(OutputFile, TargetExtension);
	SafeFileOpen(&ptrOutputFile, OutputFile, "wb");
	FileGetName(OutputFile, NewInternalName, sizeof(NewInternalName), true);

	// Write patched model data
	ModelDataSize = FileSize(&ptrModelFile);
	ModelData = (char *)malloc(ModelDataSize);
	FileReadBlock(&ptrModelFile, ModelData, 0, ModelDataSize);
	for (uchar c = 8; ModelData[c] != '\0'; c++)	// Clear old name, 8 - offset of internal name
		ModelData[c] = '\0';
	strcpy(&ModelData[8], NewInternalName);			// Copy new name, 8 - offset of internal name
	FileWriteBlock(&ptrOutputFile, ModelData, ModelDataSize);

	// Free memory
	free(ModelData);

	// Close file
	fclose(ptrModelFile);

	puts("Done!\n\n");
}

void GetExtraDOLData(const char * FileName)
{
	FILE * ptrInFile;
	FILE * ptrOutFile;
	char cOutFileName[256];
	sDOLExtraSection DOLExtraSect;

	// Open input file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Read extra section
	FileReadBlock(&ptrInFile, &DOLExtraSect, sizeof(sModelHeader), sizeof(sDOLExtraSection));
	
	// Check if *.INF file is needed
	ulong LODTableSize = DOLExtraSect.MaxBodyParts * DOLExtraSect.NumBodyGroups * sizeof(sDOLLODEntry);
	if ((DOLExtraSect.FadeStart != 0 || DOLExtraSect.FadeEnd != 0) || LODTableSize != 0)
	{
		sDOLLODEntry * LODTable;

		puts("Fetching extra data ...");

		//// Open output *.INF file
		FileGetFullName(FileName, cOutFileName, sizeof(cOutFileName));
		strcat(cOutFileName, ".inf");
		SafeFileOpen(&ptrOutFile, cOutFileName, "wb");


		//// Write output file
		fprintf(ptrOutFile, "\\\\ General data\r\n\r\n");

		// Write fade data
		if (DOLExtraSect.FadeStart != 0)
			fprintf(ptrOutFile, "%s[%d]\r\n", KWD_FADESTART, DOLExtraSect.FadeStart);

		if (DOLExtraSect.FadeEnd != 0)
			fprintf(ptrOutFile, "%s[%d]\r\n", KWD_FADEEND, DOLExtraSect.FadeEnd);
		
		// Write LOD data
		if (LODTableSize != 0)
		{
			//// Write LOD data from header
			fprintf(ptrOutFile, "%s[%d]\r\n", KWD_NUMGROUPS, DOLExtraSect.NumBodyGroups);
			fprintf(ptrOutFile, "%s[%d]\r\n\r\n\r\n", KWD_MAXPARTS, DOLExtraSect.MaxBodyParts);
			

			//// Write data from LOD table
			fprintf(ptrOutFile, "\\\\ LOD table. Distances for each LOD are inside [].\r\n");
			fprintf(ptrOutFile, "\\\\ If you plan to use this model on PC then consider\r\n");
			fprintf(ptrOutFile, "\\\\ decompiling the model and removing LOD body parts.\r\n\r\n");

			// Read LOD table
			LODTable = (sDOLLODEntry *)malloc(LODTableSize);
			if (LODTable == NULL)
			{
				puts("Can't allocate memory!");
				fclose(ptrInFile);
				return;
			}
			FileReadBlock(&ptrInFile, LODTable, DOLExtraSect.LODDataOffset, LODTableSize);

			// Parse LOD table
			for (ushort Group = 0, Entry = 0; Group < DOLExtraSect.NumBodyGroups; Group++)
			{
				fprintf(ptrOutFile, "%s\r\n{\r\n", KWD_GROUP);

				for (ushort Part = 0; Part < DOLExtraSect.MaxBodyParts; Part++, Entry++)
				{
					if (LODTable[Entry].LODCount != 0)
					{
						fprintf(ptrOutFile, "%s[%d", KWD_PART, LODTable[Entry].LODDistances[0]);
						for (uint Dist = 1; Dist < LODTable[Entry].LODCount; Dist++)
							fprintf(ptrOutFile, ",%d", LODTable[Entry].LODDistances[Dist]);
						fprintf(ptrOutFile, "]\r\n");
					}
					else
					{
						fprintf(ptrOutFile, "%s\r\n", KWD_BLANK);
					}
				}

				fprintf(ptrOutFile, "}\r\n\r\n");
			}
		}

		//// Close output file
		fclose(ptrOutFile);
	}

	// Close input file
	fclose(ptrInFile);
}

///////////////////////////////////////
//////////////////////////////////////////
////////////////////////////////////////////

bool AddTerminator(char * Buffer, char Symbol)
{
	bool Result = false;

	for (ushort i = 0; Buffer[i] != '\0'; i++)
	{
		if (Buffer[i] == Symbol)
		{
			Buffer[i] = '\0';
			Result = true;
			break;
		}
	}

	return Result;
}

ushort CountSymbols(char * Buffer, char Symbol)
{
	ushort Result = 0;

	for (ushort i = 0; Buffer[i] != '\0'; i++)
	{
		if (Buffer[i] == Symbol)
			Result++;
	}

	return Result;
}

bool CheckExtraFile(const char * FileName)
{
	char cInFileName[256];	// Input file name
	FILE * ptrInFile;		// Input file stream
	char Buffer[128];		// Text buffer
	ushort Open = 0;		// Open brackets count
	ushort Close = 0;		// Close brackets count

	puts("Checking *.INF file ...");

	// Open input *.INF file
	FileGetFullName(FileName, cInFileName, sizeof(cInFileName));
	strcat(cInFileName, ".inf");
	ptrInFile = fopen(cInFileName, "rb");
	if (ptrInFile == NULL)
		return false;

	// Count '[' and ']'
	Open = Close = 0;
	while (fgets(Buffer, sizeof(Buffer), ptrInFile) != NULL)
	{
		ushort Len = strlen(Buffer);

		// Skip comments
		if (Buffer[0] == '\\' && Buffer[1] == '\\')
			continue;

		// Ckeck '[' and ']'
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
					fclose(ptrInFile);
					return false;
				}
			}
		}

		if (Open != Close)
		{
			puts("Both \'[\' and \']\' should be on one line !");
			fclose(ptrInFile);
			return false;
		}
	}

	// Check if empty
	if (Open == 0 && Close == 0)
	{
		puts("*.INF file is empty, skipping ...");
		fclose(ptrInFile);
		return false;
	}

	// Check for parity
	if (Open != Close)
	{
		puts("Number of \'[\' not matches \']\' ...");
		fclose(ptrInFile);
		return false;
	}

	printf("Found %d active lines \n", Open);

	// Rewind file pointer
	fseek(ptrInFile, 0, SEEK_SET);

	// Count '{' and '}'
	Open = Close = 0;
	while (fgets(Buffer, sizeof(Buffer), ptrInFile) != NULL)
	{
		// Remove newline symbols
		AddTerminator(Buffer, '\r');
		AddTerminator(Buffer, '\n');

		// Skip comments
		if (Buffer[0] == '\\' && Buffer[1] == '\\')
			continue;

		// Ckeck '{' and '}'
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
				fclose(ptrInFile);
				return false;
			}
		}
	}

	// Check for parity
	if (Open != Close)
	{
		puts("Number of \'{\' not matches \'}\' !");
		fclose(ptrInFile);
		return false;
	}

	printf("Found %d body groups \n", Open);

	// Close file
	fclose(ptrInFile);

	return true;
}

void GetValues(char * Buffer, ulong * Values, uchar ValuesCount)
{
	// Find string length
	ushort Len = strlen(Buffer);

	// Clear Values
	memset(Values, 0x00, ValuesCount * sizeof(ulong));

	// Fetch values
	char NumBuffer[80];
	bool NumFound = false;
	uchar NumStart = 0;
	uchar CurrentVal = 0;
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
				// Check for values
				if (CurrentVal >= ValuesCount)
				{
					puts("Too many values, skipping the rest of them ...\n");
					break;
				}

				// Get value from string
				memcpy(NumBuffer, &Buffer[NumStart], i - NumStart);
				sscanf(NumBuffer, "%d", &Values[CurrentVal]);

				// Increnemt current value number
				CurrentVal++;

				// Current end = next start
				NumStart = i + 1;
			}
		}

		if (Buffer[i] == ']')
		{
			break;
		}
	}
}

bool TranslateExtraFile(const char * FileName, sDOLExtraSection * DOLExtraSect, sDOLLODEntry ** LODTable)
{
	FILE * ptrInFile;					// Input file stream
	char cInFileName[256];				// Input file name
	char Buffer[128] = "Text";			// Text buffer
	char PrevBuffer[128] = "Text";		// Previous state of text buffer
	bool Group = true;					// Map or model

	//// Open input *.INF file
	FileGetFullName(FileName, cInFileName, sizeof(cInFileName));
	strcat(cInFileName, ".inf");
	SafeFileOpen(&ptrInFile, cInFileName, "rb");

	//// Clear extra section
	memset(DOLExtraSect, 0x00, sizeof(sDOLExtraSection));

	//// Fetch parameters from file (first pass, two passes are needed in case someone places "bodys" and "maxparts" parameters in the end of a file)
	Group = false;
	while (fgets(Buffer, sizeof(Buffer), ptrInFile) != NULL)
	{
		// Remove newline symbols
		AddTerminator(Buffer, '\r');
		AddTerminator(Buffer, '\n');

		// Skip comments and empty lines
		if ((Buffer[0] == '\\' && Buffer[1] == '\\') || Buffer[0] == '\0')
		{
			PrevBuffer[0] = '\0';
			continue;
		}

		// Fetch general parameters (outside of {})
		if (Buffer[0] == '{' && Buffer[1] == '\0')
		{
			Group = true;
		}
		else if (Buffer[0] == '}' && Buffer[1] == '\0')
		{
			Group = false;
		}
		else
		{
			if (Group == false)
			{
				if (CountSymbols(Buffer, '[') != 0)
				{
					ulong Value = 0;

					GetValues(Buffer, &Value, 1);
					AddTerminator(Buffer, '[');

					if (!strcmp(Buffer, KWD_FADESTART) == true)
					{
						// Fade start distance
						DOLExtraSect->FadeStart = Value;
						if (DOLExtraSect->FadeEnd == 0)
							DOLExtraSect->FadeEnd = Value;
						puts("Got fade start value ...");
					}
					else if (!strcmp(Buffer, KWD_FADEEND) == true)
					{
						// Fade end distance
						DOLExtraSect->FadeEnd = Value;
						if (DOLExtraSect->FadeStart == 0)
							DOLExtraSect->FadeStart = Value;
						puts("Got fade end value ...");
					}
					else if (!strcmp(Buffer, KWD_NUMGROUPS) == true)
					{
						// Number of body groups
						DOLExtraSect->NumBodyGroups = Value;
						puts("Got number of body groups ...");
					}
					else if (!strcmp(Buffer, KWD_MAXPARTS) == true)
					{
						// Max number of body parts
						DOLExtraSect->MaxBodyParts = Value;
						puts("Got maximum number of body parts ...");
					}
				}
			}
		}
	}

	
	//// Fetch LOD table (second pass)

	// Check if LOD table is needed
	if (DOLExtraSect->NumBodyGroups * DOLExtraSect->MaxBodyParts == 0)
	{
		*LODTable = NULL;
		fclose(ptrInFile);
		return true;
	}

	puts("Fetching LOD table ...");

	// Rewind input file
	rewind(ptrInFile);

	// Allocate memory for LOD table
	ulong LODTableSz = DOLExtraSect->NumBodyGroups * DOLExtraSect->MaxBodyParts * sizeof(sDOLLODEntry);
	*LODTable = (sDOLLODEntry *) calloc(1, LODTableSz);
	if (*LODTable == NULL)
	{
		puts("Can't allocate memory!");
		return false;
	}

	// Fill table
	uchar GroupCount = 0;				// Number of body groups
	uchar PartCount = 0;				// Number of body parts
	ushort EntryNum = 0;				// Current entry number
	Group = false;						// Group flag
	while (fgets(Buffer, sizeof(Buffer), ptrInFile) != NULL)
	{
		// Remove newline symbols
		AddTerminator(Buffer, '\r');
		AddTerminator(Buffer, '\n');

		// Skip comments and empty lines
		if ((Buffer[0] == '\\' && Buffer[1] == '\\') || Buffer[0] == '\0')
		{
			PrevBuffer[0] = '\0';
			continue;
		}
			

		// Fetch LOD table (inside of {})
		if (Buffer[0] == '{' && Buffer[1] == '\0')
		{
			printf("Fetching group #%d: \t\"%s\"\n", GroupCount, PrevBuffer);
			
			// Check if out of bounds
			if (GroupCount >= DOLExtraSect->NumBodyGroups)
			{
				printf("Group #%d is out of bounds, ignoring the rest of the file\n", GroupCount);
				break;
			}

			// Set body group flag and reset parts count for this group
			Group = true;
			PartCount = 0;

			// Bring entry number to the beginning of the new group (needed to allow not putting "blank" lines in the end of groups with part count < max part count)
			EntryNum = GroupCount * DOLExtraSect->MaxBodyParts;
		}
		else if (Buffer[0] == '}' && Buffer[1] == '\0')
		{
			// Unset body group flag
			Group = false;

			// Increnent group count
			GroupCount++;
		}
		else
		{
			if (Group == true)
			{
				if (CountSymbols(Buffer, '[') != 0)
				{
					// Check if out of bounds
					if (PartCount >= DOLExtraSect->MaxBodyParts)
					{
						printf("Part #%d is out of bounds, skipping ...\n", PartCount);
						PartCount++;
						EntryNum++;
						continue;
					}

					// Fetch
					uchar ValuesCnt = CountSymbols(Buffer, ',') + 1;
					if (ValuesCnt > 4)	// Prevent going out of bounds
						ValuesCnt = 4;
					(*LODTable)[EntryNum].LODCount = ValuesCnt;
					GetValues(Buffer, (*LODTable)[EntryNum].LODDistances, ValuesCnt);
					AddTerminator(Buffer, '[');
					printf("Fetched part #%d: \t\"%s\"\n", PartCount, Buffer);
					
					// Increnent part and entry counters
					PartCount++;
					EntryNum++;
				}
				else
				{
					//if (!strcmp(Buffer, KWD_BLANK) == true)
					//{
						// Check if out of bounds
						if (PartCount >= DOLExtraSect->MaxBodyParts)
						{
							printf("Part #%d is out of bounds, skipping ...\n", PartCount);
							PartCount++;
							EntryNum++;
							continue;
						}
						
						// Fetch (in this case do nothing as entry already should be nulled out)
						printf("Fetched blank part #%d\n", PartCount);

						// Increnent part and entry counters
						PartCount++;
						EntryNum++;
					//}
				}
			}
		}

		strcpy(PrevBuffer, Buffer);
	}

	//// Close input file
	fclose(ptrInFile);

	return true;
}
///////////////////////////////////////
///////////////////////////////////
//////////////////////////////

void PatchSubmodelRef(sModelHeader * MdlHdr, char * ModelData, ulong ModelDataSize, char * NewExtension)		// Patch internal submodel references
{
	// Check if patching is needed
	if (MdlHdr->SubmodelCount <= 1)
		return;

	// Don't patch empty model
	if (ModelDataSize < 4 || strlen(NewExtension) != 4)
		return;

	printf("Found %i internal submodel reference(s), patching ...\n", MdlHdr->SubmodelCount - 1);
	
	// Calculate offset to first submodel reference
	ulong Offset = MdlHdr->SubmodelTableOffset - sizeof(sModelHeader) + MDL_DEF_REF_SZ + MDL_FILE_REF_SPACE;
	
	// Patch
	char * RefName;
	for (ulong i = 0; i < MdlHdr->SubmodelCount - 1; i++, Offset += (MDL_FILE_REF_SZ + MDL_FILE_REF_SPACE))
	{
		// Redundant check?
		if (Offset + MDL_FILE_REF_SZ > ModelDataSize)
		{
			puts("Oops, model data is too small. Patching failed ...");
			return;
		}

		// Set pointer to the beginning of reference string
		RefName = &ModelData[Offset];

		// Rewind pointer to the end of reference string
		char Counter = 0;
		while (*RefName != '\0')
		{
			RefName++;
			Counter++;
			if (Counter > MDL_FILE_REF_SZ)
			{
				puts("Oops, erroneous reference. Patching failed ...");
				return;
			}
		}

		// Replace four last characters with new extension letters
		RefName -= 4;
		memcpy(RefName, NewExtension, 4);
	}
}

uint PSIProperSize(uint Size, bool ToLower)	// Function returns closest proper dimension. PS2 HL proper PSI dimensions: 8 (min), 16, 32, 64, 128, 256, 512, ...
{
	uint CurrentSize;

	// Calculate closest (bigger or equal) proper dimension
	CurrentSize = PSI_MIN_DIMENSION;
	while (CurrentSize < Size)
		CurrentSize <<= 1; // *= 2;

	// Return proper dimension
	if (Size <= PSI_MIN_DIMENSION)
	{
		return PSI_MIN_DIMENSION;
	}
	else
	{
		if (Size == CurrentSize)
		{
			return CurrentSize;
		}
		else
		{
			if (ToLower == true)
				return CurrentSize >>= 1; // / 2;
			else
				return CurrentSize;
		}
	}
}

void ExtractDOLTextures(const char * FileName)	// Extract textures from PS2 model
{
	sModelHeader ModelHeader;					// Model file header
	sModelTextureEntry * ModelTextureTable;		// Model texture table
	ulong ModelTextureTableSize;				// Model texture table size (how many textures)
	sTexture * Textures;						// Pointer to texturs data

	FILE * ptrInFile;
	sBMPHeader BMPHeader;						// BMP header
	FILE * ptrBMPOutput;
	char cOutFileName[255];
	char cOutFolderName[255];

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Load model header
	ModelHeader.UpdateFromFile(&ptrInFile);

	// Check model
	if (ModelHeader.CheckModel() == NORMAL_MODEL)
	{
		printf("Internal name: %s \nTextures: %i, Texture table offset: 0x%X \n", ModelHeader.Name, ModelHeader.TextureCount, ModelHeader.TextureTableOffset);
	}
	else
	{
		puts("Can't extract textures.");
		return;
	}

	// Allocate memory for textures
	ModelTextureTable = (sModelTextureEntry *)malloc(ModelHeader.TextureCount * sizeof(sModelTextureEntry));
	Textures = (sTexture *)malloc(sizeof(sTexture) * ModelHeader.TextureCount);

	// Prepare folder for output files
	strcpy(cOutFolderName, FileName);
	strcat(cOutFolderName, "-textures\\");
	NewDir(cOutFolderName);

	uint BitmapOffset;
	uint BitmapSize;
	uint PaletteOffset;
	uint PaletteSize;
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		ModelTextureTable[i].UpdateFromFile(&ptrInFile, ModelHeader.TextureTableOffset, i);
		printf(" Texture #%i \n Name: %s \n Width: %i \n Height: %i \n Offset: %x \n\n", i + 1, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height, ModelTextureTable[i].Offset);

		BitmapOffset = ModelTextureTable[i].Offset + DOL_TEXTURE_HEADER_SIZE + EIGHT_BIT_PALETTE_ELEMENTS_COUNT * DOL_BMP_PALETTE_ELEMENT_SIZE;
		BitmapSize = ModelTextureTable[i].Height * ModelTextureTable[i].Width;
		PaletteOffset = ModelTextureTable[i].Offset + DOL_TEXTURE_HEADER_SIZE;
		PaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * DOL_BMP_PALETTE_ELEMENT_SIZE;

		// Load texture
		Textures[i].Initialize();
		Textures[i].UpdateFromFile(&ptrInFile, BitmapOffset, BitmapSize, PaletteOffset, PaletteSize, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height);

		// Convert texture
		Textures[i].FlipBitmap();
		Textures[i].PaletteReformat(DOL_BMP_PALETTE_ELEMENT_SIZE);
		Textures[i].PaletteRemoveSpacers();
		Textures[i].PaletteAddSpacers(0x00);
		Textures[i].PaletteSwapRedAndGreen(DOL_BMP_PALETTE_ELEMENT_SIZE);

		// Save texture to *.bmp
		strcpy(cOutFileName, cOutFolderName);
		strcat(cOutFileName, ModelTextureTable[i].Name);
		SafeFileOpen(&ptrBMPOutput, cOutFileName, "wb");

		BMPHeader.Update(Textures[i].Width, Textures[i].Height);
		FileWriteBlock(&ptrBMPOutput, (char *) &BMPHeader, sizeof(sBMPHeader));
		FileWriteBlock(&ptrBMPOutput, (char *) Textures[i].Palette, Textures[i].PaletteSize);
		FileWriteBlock(&ptrBMPOutput, (char *) Textures[i].Bitmap, Textures[i].Width * Textures[i].Height);

		fclose(ptrBMPOutput);
	}

	// Free memory
	free(ModelTextureTable);
	free(Textures);

	// Close files
	fclose(ptrInFile);

	puts("Done!\n\n");
}

void ExtractMDLTextures(const char * FileName)	// Extract textures from PC model
{
	sModelHeader ModelHeader;					// Model file header
	sModelTextureEntry * ModelTextureTable;		// Model texture table
	ulong ModelTextureTableSize;				// Model texture table size (how many textures)
	sTexture * Textures;						// Pointer to textures data

	FILE * ptrInFile;
	sBMPHeader BMPHeader;						// BMP header
	FILE * ptrBMPOutput;
	char cOutFileName[255];
	char cOutFolderName[255];

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Load model header
	ModelHeader.UpdateFromFile(&ptrInFile);

	// Check model
	if (ModelHeader.CheckModel() == NORMAL_MODEL)
	{
		printf("Internal name: %s \nTextures: %i, Texture table offset: 0x%X \n", ModelHeader.Name, ModelHeader.TextureCount, ModelHeader.TextureTableOffset);
	}
	else
	{
		puts("Can't extract textures.");
		return;
	}

	// Allocate memory for textutes
	ModelTextureTable = (sModelTextureEntry *)malloc(ModelHeader.TextureCount * sizeof(sModelTextureEntry));
	Textures = (sTexture *)malloc(sizeof(sTexture) * ModelHeader.TextureCount);

	// Prepare folder for output files
	strcpy(cOutFolderName, FileName);
	strcat(cOutFolderName, "-textures\\");
	NewDir(cOutFolderName);

	uint BitmapOffset;
	uint BitmapSize;
	uint PaletteOffset;
	uint PaletteSize;
	bool RawExtract = false;
	char TexExtension[5];
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		ModelTextureTable[i].UpdateFromFile(&ptrInFile, ModelHeader.TextureTableOffset, i);
		printf(" Texture #%i \n Name: %s \n Width: %i \n Height: %i \n Offset: %x \n\n", i + 1, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height, ModelTextureTable[i].Offset);

		// PVR check
		if (RawExtract == false)
		{
			FileGetExtension(ModelTextureTable[i].Name, TexExtension, sizeof(TexExtension));
			if (!strcmp(TexExtension, ".pvr") == true)
			{
				RawExtract = true;
				puts("Dreamcast textures found ...");
			}
		}

		// Extract texture
		if (RawExtract == false)
		{
			// Normal texture //

			BitmapOffset = ModelTextureTable[i].Offset + MDL_TEXTURE_HEADER_SIZE;
			BitmapSize = ModelTextureTable[i].Height * ModelTextureTable[i].Width;
			PaletteOffset = ModelTextureTable[i].Offset + ModelTextureTable[i].Width * ModelTextureTable[i].Height;
			PaletteSize = EIGHT_BIT_PALETTE_ELEMENTS_COUNT * MDL_PALETTE_ELEMENT_SIZE;

			// Load texture
			Textures[i].Initialize();
			Textures[i].UpdateFromFile(&ptrInFile, BitmapOffset, BitmapSize, PaletteOffset, PaletteSize, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height);

			// Convert texture
			Textures[i].FlipBitmap();
			Textures[i].PaletteSwapRedAndGreen(MDL_PALETTE_ELEMENT_SIZE);
			Textures[i].PaletteAddSpacers(0x00);

			// Save texture to *.bmp file
			strcpy(cOutFileName, cOutFolderName);
			strcat(cOutFileName, ModelTextureTable[i].Name);
			SafeFileOpen(&ptrBMPOutput, cOutFileName, "wb");

			BMPHeader.Update(Textures[i].Width, Textures[i].Height);
			FileWriteBlock(&ptrBMPOutput, (char *)&BMPHeader, sizeof(sBMPHeader));
			FileWriteBlock(&ptrBMPOutput, (char *)Textures[i].Palette, Textures[i].PaletteSize);
			FileWriteBlock(&ptrBMPOutput, (char *)Textures[i].Bitmap, Textures[i].Width * Textures[i].Height);
		}
		else
		{
			// PVR texture (raw extract) //

			uchar * pPVR;
			uint PVRSize;

			// Get size
			if (i != ModelHeader.TextureCount - 1)
			{
				ModelTextureTable[i + 1].UpdateFromFile(&ptrInFile, ModelHeader.TextureTableOffset, i + 1);
				PVRSize = ModelTextureTable[i + 1].Offset - ModelTextureTable[i].Offset;
			}
			else
			{
				PVRSize = FileSize(&ptrInFile) - ModelTextureTable[i].Offset;	// Last entry in the texture table
			}

			// Allocate memory
			pPVR = (uchar *)malloc(PVRSize);
			if (pPVR == NULL)
			{
				puts("Unable to allocate memory ...");
				exit(EXIT_FAILURE);
			}
			
			// Read texture
			FileReadBlock(&ptrInFile, pPVR, ModelTextureTable[i].Offset, PVRSize);

			// Open output file
			strcpy(cOutFileName, cOutFolderName);
			strcat(cOutFileName, ModelTextureTable[i].Name);
			SafeFileOpen(&ptrBMPOutput, cOutFileName, "wb");

			// Write texture
			FileWriteBlock(&ptrBMPOutput, pPVR, PVRSize);

			// Free memory
			free(pPVR);
		}

		// Close output file
		fclose(ptrBMPOutput);
	}

	// Free memory
	free(ModelTextureTable);
	free(Textures);

	// Close files
	fclose(ptrInFile);

	puts("Done!\n\n");
}

void SeqReport(const char * FileName)
{
	sModelHeader ModelHeader;	// Model file header
	sModelSeq * SeqTable;		// Sequences table
	ulong SeqTableSz;			// Sequences table size
	int SeqCount;				// Sequences count
	FILE * ptrInFile;
	FILE * ptrOutFile;
	char cOutFileName[255];

	// Open input file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Load model header
	ModelHeader.UpdateFromFile(&ptrInFile);

	// Check model
	if (ModelHeader.CheckModel() == NORMAL_MODEL)
	{
		printf("Internal name: %s \nSequences: %i \n", ModelHeader.Name, ModelHeader.SeqCount);
	}
	else
	{
		puts("Bad model file");
		return;
	}

	// Allocate memory for sequence table
	SeqCount = ModelHeader.SeqCount;
	SeqTableSz = sizeof(sModelSeq) * SeqCount;
	SeqTable = (sModelSeq *) malloc(SeqTableSz);

	// Read qequence table
	FileReadBlock(&ptrInFile, SeqTable, ModelHeader.SeqTableOffset, SeqTableSz);

	// Close input file
	fclose(ptrInFile);

	// Open output file
	FileGetFullName(FileName, cOutFileName, sizeof(cOutFileName));
	strcat(cOutFileName, "_seq.txt");
	SafeFileOpen(&ptrOutFile, cOutFileName, "w");

	// Print report
	fprintf(ptrOutFile, "File: %s\nSequences: %d\n\n", FileName, SeqCount);
	fprintf(ptrOutFile, "[#]\t[File]\t[Sequence]\n", FileName, SeqCount);
	for (int sq = 0; sq < SeqCount; sq++)
		fprintf(ptrOutFile, "%d\t%d\t%s\n", sq, SeqTable[sq].Num, SeqTable[sq].Name);

	// Close output file
	fclose(ptrOutFile);

	// Free memory
	free(SeqTable);

	puts("Done!\n\n");
}

int main(int argc, char * argv[])
{
	FILE * ptrInputFile;
	FILE * ptrConfigFile;
	char ConfigFilePath[255];
	char Line[80];
	char cFileExtension[5];

	// Output info
	printf("\nPS2 HL model tool v%s \n", PROG_VERSION);

	// Check arguments
	if (argc == 1)
	{
		// No arguments - show help screen
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2017-2018, Alexey Leushin. All rights reserved.\n");
		puts("How to use: \n1) Windows explorer - drag and drop model file on mdltool.exe \n2) Command line/Batch - mdltool [model_file_name] \nOptional features:\n - extract textures: mdltool extract [filename]\n - report sequences: mdltool seqrep [filename]  \n\nFor more info read ReadMe.txt \n");
		puts("Press any key to exit ...");

		_getch();
	}
	else if (argc == 2)		// Convert model
	{
		FileGetExtension(argv[1], cFileExtension, 5);

		printf("\nProcessing file: %s\n", argv[1]);

		if (!strcmp(".mdl", cFileExtension))
		{
			if (CheckModel(argv[1]) == NORMAL_MODEL)
			{
				ConvertMDLToDOL(argv[1]);
			}
			else if (CheckModel(argv[1]) == SEQ_MODEL || CheckModel(argv[1]) == NOTEXTURES_MODEL)
			{
				ConvertSubmodel(argv[1], ".mdl", ".dol");
			}
			else if (CheckModel(argv[1]) == DUMMY_MODEL)
			{
				// Patch dummy model's extension and internal name
				ConvertDummySubmodel(argv[1], ".mdl", ".dol");
			}
			else
			{
				puts("Can't recognise model file ...");
			}
		}
		else if (!strcmp(".dol", cFileExtension))
		{
			if (CheckModel(argv[1]) == NORMAL_MODEL)
			{
				ConvertDOLToMDL(argv[1]);
			}
			else if (CheckModel(argv[1]) == SEQ_MODEL || CheckModel(argv[1]) == NOTEXTURES_MODEL)
			{
				ConvertSubmodel(argv[1], ".dol", ".mdl");
			}
			else if (CheckModel(argv[1]) == DUMMY_MODEL)
			{
				// Patch dummy model's extension and internal name
				ConvertDummySubmodel(argv[1], ".dol", ".mdl");
			}
			else
			{
				puts("Can't recognise model file ...");
			}
		}
		else
		{
			puts("Wrong file extension.");
		}
	}
	else if (argc == 3 && !strcmp(argv[1], "extract") == true)		// Extract textures from model
	{
		FileGetExtension(argv[2], cFileExtension, 5);

		printf("\nProcessing file: %s\n", argv[2]);

		if (!strcmp(".mdl", cFileExtension))
		{
			if (CheckModel(argv[2]) == NORMAL_MODEL)
				ExtractMDLTextures(argv[2]);
			else
				puts("Can't find texture data ...");
		}
		else if (!strcmp(".dol", cFileExtension))
		{
			if (CheckModel(argv[2]) == NORMAL_MODEL)
				ExtractDOLTextures(argv[2]);
			else
				puts("Can't find texture data ...");
		}
		else
		{
			puts("Wrong file extension.");
		}
	}
	else if (argc == 3 && !strcmp(argv[1], "seqrep") == true)		// Report sequences
	{
		FileGetExtension(argv[2], cFileExtension, 5);

		printf("\nProcessing file: %s\n", argv[2]);

		if (!strcmp(".mdl", cFileExtension) || !strcmp(".dol", cFileExtension))
			SeqReport(argv[2]);
		else
			puts("Wrong file extension.");
	}
	else
	{
		puts("Can't recognise arguments.");
	}

	//getchar();
	
	return 0;
}
