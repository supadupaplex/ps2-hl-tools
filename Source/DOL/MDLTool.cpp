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
// This file contains model conversion and texture extraction functions
//

////////// Includes //////////
#include "main.h"

////////// Global variables //////////
// Configuration variables
char WarningMode[10]{ "stop\n" };	// Warning processing mode
char PatchMode[10]{ "ask\n" };		// Experimental patch handling mode

////////// Functions //////////
uint PSIProperSize(uint Size, bool ToLower);																		// Calculate nearest appropriate size of PS2 DOL texture
void ExtractDOLTextures(const char * FileName);																		// Extract textures from PS2 model
void ExtractMDLTextures(const char * FileName);																		// Extract textures from PC model
void ConvertMDLToDOL(const char * FileName);																		// Convert model from PC to PS2 format
void ConvertDOLToMDL(const char * FileName);																		// Convert model from PS2 to PC format
void ConvertSubmodel(const char * FileName, char * OriginalExtension, char * TargetExtension);						// Convert submodel
void ConvertDummySubmodel(const char * FileName, char * OriginalExtension, char * TargetExtension);					// Convert submodel which consists of signature and name only
void PatchSubmodelRef(char * ModelData, ulong ModelDataSize, char * OriginalExtension, char * TargetExtension);		// Patch internal submodel references
bool DOLCheckLODs(char * ModelData, ulong ModelDataSize, char * ModelHeader);										// Check if model has LODs (hacky stuff)
void MDLPatchSwitch(char * ModelData, ulong ModelDataSize);															// Apply patch (hack) that allows correct submesh switching and stops crashing on PS2.
int CheckModel(const char * FileName);																				// Check model type



bool DOLCheckLODs(char * ModelData, ulong ModelDataSize, char * ModelHeader)	// Check if model has LODs (hacky stuff)
{
	ulong SubmeshTabOffset;

	if (ModelDataSize < 6)
		return false;

	if (ModelData[4] != 0 && ModelData[5] != 0)		// Check if LOD data is present in PS2 model
		if (ModelHeader[sizeof(sModelHeader) - 4] == ModelData[0] &&		// Check if this model is original PS2 model (not reconverted)
			ModelHeader[sizeof(sModelHeader) - 3] == ModelData[1] &&
			ModelHeader[sizeof(sModelHeader) - 2] == ModelData[2] &&
			ModelHeader[sizeof(sModelHeader) - 1] == ModelData[3])
		{
			puts("\nWarning: detected LODs. Consider decompiling *.MDL file and removing \nLOD submeshes from *.qc file to make this model fully usable in PC version \nPress any key to continue ...");
			
			if (!strcmp(WarningMode, "stop\n") == true)					// Stop program to show warning if configuration says to do it
				_getch();

			return true;
		}

	return false;
}

void MDLPatchSwitch(char * ModelData, ulong ModelDataSize)		// Apply patch (hack) that allows correct submesh switching and stops crashing on PS2.
{
	if (ModelDataSize < 6)
		return;

	if (!strcmp(PatchMode, "no\n") == true)
		return;

	puts("Applying experimental patch to overlaying data ...");
	ModelData[4] = 0;
	ModelData[5] = 0;
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
	PatchSubmodelRef((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), ".dol", ".mdl");		// Patch internal submodel references
	DOLCheckLODs((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), (char *) &ModelHeader);
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

	puts("Done\n");
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
	PatchSubmodelRef((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader), ".mdl", ".dol");		// Patch internal submodel references
	MDLPatchSwitch((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader));	// Experimental patch (hack)
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

	// Update model size field
	ModelSize = FileSize(&ptrOutFile);
	FileWriteBlock(&ptrOutFile, &ModelSize, 0x48, sizeof(ModelSize));	// 0x48 - address of model size field

	// Free memory
	free(ModelTextureTable);
	free(Textures);

	// Close files
	fclose(ptrInFile);
	fclose(ptrOutFile);

	puts("Done\n");
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
	if (ModelHeader.CheckModel() != NOTEXTURES_MODEL && ModelHeader.CheckModel() != SUB_MODEL)
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
	PatchSubmodelRef(ModelData, ModelDataSize, OriginalExtension, TargetExtension);
	if (!strcmp(OriginalExtension, ".mdl") == true)
		if (ModelHeader.CheckModel() == NOTEXTURES_MODEL)	// Apply patch to "IDST" models only
			MDLPatchSwitch((char *)ModelData, ModelHeader.TextureTableOffset - sizeof(sModelHeader));	// Experimental patch (hack)
	FileWriteBlock(&ptrOutputFile, ModelData, ModelDataSize);

	// Free memory
	free(ModelData);

	// Close file
	fclose(ptrModelFile);

	puts("Done\n");
}

void ConvertDummySubmodel(const char * FileName, char * OriginalExtension, char * TargetExtension)	// Convert submodel which consists of signature and name only
{
	FILE * ptrModelFile;
	FILE * ptrOutputFile;

	char * ModelData;
	ulong ModelDataSize;

	char OutputFile[255];

	puts("Patching dummy submodel ...");

	// Open model file
	SafeFileOpen(&ptrModelFile, FileName, "rb");

	// Create new model file
	FileGetFullName(FileName, OutputFile, sizeof(OutputFile));
	strcat(OutputFile, TargetExtension);
	SafeFileOpen(&ptrOutputFile, OutputFile, "wb");

	// Write patched model data
	ModelDataSize = FileSize(&ptrModelFile);
	ModelData = (char *)malloc(ModelDataSize);
	FileReadBlock(&ptrModelFile, ModelData, 0, ModelDataSize);
	PatchSubmodelRef(ModelData, ModelDataSize, OriginalExtension, TargetExtension);
	FileWriteBlock(&ptrOutputFile, ModelData, ModelDataSize);

	// Free memory
	free(ModelData);

	// Close file
	fclose(ptrModelFile);

	puts("Done\n");
}

void PatchSubmodelRef(char * ModelData, ulong ModelDataSize, char * OriginalExtension, char * TargetExtension)		// Patch internal submodel references
{
	uint Counter = 0;

	if (ModelDataSize < 4)
		return;

	for (ulong i = 0; i < ModelDataSize - 4; i++)
	{
		if (ModelData[i] == OriginalExtension[0])
		{
			if (ModelData[i + 1] == OriginalExtension[1] && ModelData[i + 2] == OriginalExtension[2] && ModelData[i + 3] == OriginalExtension[3])
			{
				ModelData[i] = TargetExtension[0];
				ModelData[i + 1] = TargetExtension[1];
				ModelData[i + 2] = TargetExtension[2];
				ModelData[i + 3] = TargetExtension[3];

				Counter++;
			}
		}
	}

	if (Counter > 0)
		printf("Patched %i internal submodel references \n", Counter);
}

uint PSIProperSize(uint Size, bool ToLower)	// Function returns closest proper dimension. PS2 HL proper PSI dimensions: 16 (min), 32, 64, 128, 256, 512, ...
{
	uint CurrentSize;

	// Calculate closest (bigger or equal) proper dimension
	CurrentSize = PSI_MIN_DIMENSION;
	while (CurrentSize < Size)
		CurrentSize *= 2;

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
				return CurrentSize / 2;
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

	puts("Done\n");
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
	for (int i = 0; i < ModelHeader.TextureCount; i++)
	{
		ModelTextureTable[i].UpdateFromFile(&ptrInFile, ModelHeader.TextureTableOffset, i);
		printf(" Texture #%i \n Name: %s \n Width: %i \n Height: %i \n Offset: %x \n\n", i + 1, ModelTextureTable[i].Name, ModelTextureTable[i].Width, ModelTextureTable[i].Height, ModelTextureTable[i].Offset);

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

		fclose(ptrBMPOutput);
	}

	// Free memory
	free(ModelTextureTable);
	free(Textures);

	// Close files
	fclose(ptrInFile);

	puts("Done\n");
}

int main(int argc, char * argv[])
{
	FILE * ptrInputFile;
	FILE * ptrConfigFile;
	char ConfigFilePath[255];
	char Line[80];

	// Output info
	printf("\nPS2 HL model tool v%s \n", PROG_VERSION);

	// Load configuration from file
	ProgGetPath(ConfigFilePath, sizeof(ConfigFilePath));
	strcat(ConfigFilePath, "settings.ini");
	if (CheckFile(ConfigFilePath) == true)
	{
		puts("Loading settings ...");

		SafeFileOpen(&ptrConfigFile, ConfigFilePath, "r");

		while (fgets(Line, sizeof(Line), ptrConfigFile) != NULL)
		{
			if (!strcmp(Line, "-warning_mode\n") == true)
			{
				fgets(WarningMode, sizeof(WarningMode), ptrConfigFile);
			}
			else if (!strcmp(Line, "-apply_patch\n") == true)
			{
				fgets(PatchMode, sizeof(PatchMode), ptrConfigFile);
			}
		}
		
		fclose(ptrConfigFile);
	}

	// Check arguments
	if (argc == 1)
	{
		// No arguments - show help screen
		puts("\nDeveloped by Alexey Leusin. \nCopyright (c) 2017, Alexey Leushin. All rights reserved.\n");
		puts("How to use: \n1) Windows explorer - drag and drop model file on mdltool.exe \n2) Command line/Batch - mdltool [model_file_name] \nOptional feature: extract textures - mdltool extract [model_file_name]  \n\nFor more info read ReadMe.txt \n");
		puts("Press any key to exit ...");

		_getch();
	}
	else if (argc == 2)		// Convert model
	{
		char cFileExtension[5];

		FileGetExtension(argv[1], cFileExtension, 5);

		printf("\nProcessing file: %s\n", argv[1]);

		if (!strcmp(".mdl", cFileExtension))
		{
			if (CheckModel(argv[1]) == NORMAL_MODEL)
			{
				ConvertMDLToDOL(argv[1]);
			}
			else if (CheckModel(argv[1]) == SUB_MODEL || CheckModel(argv[1]) == NOTEXTURES_MODEL)
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
			else if (CheckModel(argv[1]) == SUB_MODEL || CheckModel(argv[1]) == NOTEXTURES_MODEL)
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
		char cFileExtension[5];

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
	else
	{
		puts("Can't recognise arguments.");
	}
}
