// Author:	supadupaplex
// License:	cheack readme.txt and license.txt

//
// This file contains functions that perform various file operations
//

#include "fops.h"

//#define FDEBUG // Enable/disable debug
#ifdef FDEBUG
	#define DPRINT(...) printf(__VA_ARGS__);
#else
	#define DPRINT(...)
#endif

size_t FileSize(FILE **ptrFile)
{
	fseek(*ptrFile, 0, SEEK_END);						// Move pointer to the file's end
	return ftell(*ptrFile);								// Return pointer position
}

void FileReadBlock(FILE **ptrSrcFile, void * DstBuff, size_t Addr, size_t Size)
{
	fseek(*ptrSrcFile, Addr, SEEK_SET);					// Seek to specified address
	fread(DstBuff, (size_t)1, Size, *ptrSrcFile);		// 
}

void FileWriteBlock(FILE **ptrDstFile, const void * SrcBuff, size_t Addr, size_t Size)
{
	fseek(*ptrDstFile, Addr, SEEK_SET);					// Seek to specified address
	fwrite(SrcBuff, (size_t)1, Size, *ptrDstFile);		// Write block
	fseek(*ptrDstFile, 0, SEEK_END);					// Set pointer to file's end
}

void FileWriteBlock(FILE **ptrDstFile, const void * SrcBuff, size_t Size)
{
	fseek(*ptrDstFile, 0, SEEK_END);					// Set pointer to file's end
	fwrite(SrcBuff, (size_t)1, Size, *ptrDstFile);		// Write block
}

void SafeFileOpen(FILE **ptrFile, const char * FileName, char * Mode)
{
	fopen_s(ptrFile, FileName, Mode);
	if (*ptrFile == NULL)
	{
		printf("Error: can't open file: %s \n\n", FileName);
		exit(EXIT_FAILURE);
	}
}

void FileGetExtension(const char * Path, char * OutputBuffer, int OutputBufferSize)
{
	if (OutputBufferSize > 4)
		for (int i = 0; i <= 4; i++)
			OutputBuffer[i] = tolower(Path[strlen(Path) - 4 + i]);
}

void FileGetName(const char * Path, char * OutputBuffer, int OutputBufferSize, bool WithExtension)
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

void FileGetFullName(const char * Path, char * OutputBuffer, int OutputBufferSize)
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

void FileGetPath(const char * Path, char * OutputBuffer, int OutputBufferSize)
{
	// Set initial end position at start to return nothing if argument is file name without path
	int End = 0;
	int Len = strlen(Path);
	
	for (int i = 0; i < Len; i++)
	{
		if (Path[i] == '\\')
			End = i + 1;
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

void FileSafeRename(char * OldName, char * NewName)
{
	//char Action;

	// Check if file exists
	if (CheckFile(NewName) == true)
	{
		remove(NewName);

		/*
		printf("File \"%s\" already exists. Overwrite (y\\n)? \n", NewName);
		do
		{
			Action = _getch();
		} while (Action != 'y' && Action != 'n');

		if (Action == 'y')
		{
			remove(NewName);
		}
		else
		{
			puts("Current operation would be terminated. Press any key to exit ... \n");
			_getch();
			exit(EXIT_FAILURE);
		}
		*/
	}

	// Rename
	rename(OldName, NewName);
}

void PatchSlashes(char * cPathBuff, int BuffSize, bool SlashToBackslash)
{
	if (SlashToBackslash == true)
	{
		for (int i = 0; i < BuffSize; i++)
			if (cPathBuff[i] == '/')
				cPathBuff[i] = '\\';
	}
	else
	{
		for (int i = 0; i < BuffSize; i++)
			if (cPathBuff[i] == '\\')
				cPathBuff[i] = '/';
	}
}

//// PLATFORM-DEPENDENT CODE BELOW ////

bool CheckDir(const char * Path)
{
	struct stat DirStat;

	stat(Path, &DirStat);

	if (DirStat.st_mode & S_IFDIR)
		return true;
	else
		return false;
}

void GenerateFolders(char * cPath)
{
	char PathBuffer[PATH_LEN];
	char * cDir;
	char cCurrent[PATH_LEN] = "";
	char Counter = 0;

	strcpy(PathBuffer, cPath);			// Save cPath to buffer, to avoid error, when parsing (const char *)

	cDir = strtok(PathBuffer, "\\");	// First run: count words
	while (cDir != NULL)
	{
		Counter++;
		cDir = strtok(NULL, "\\");
	}

	strcpy(PathBuffer, cPath);			// Give strtok() fresh string
	cDir = strtok(PathBuffer, "\\");	// Second run: create folders
	while (cDir != NULL)
	{
		strcat(cCurrent, cDir);
		if (Counter > 1)
		{
			NewDir(cCurrent);
			Counter--;
		}
		strcat(cCurrent, "\\");
		cDir = strtok(NULL, "\\");
	}
}


#ifdef _WIN32

void NewDir(const char * DirName)
{
	CreateDirectoryA(DirName, NULL);
}

void ProgGetPath(char * OutputBuffer, int OutputBufferSize)
{
	HMODULE hModule;
	char cFileName[PATH_LEN];

	// Get program file name
	hModule = GetModuleHandle(NULL);
	GetModuleFileNameA(hModule, cFileName, sizeof(cFileName));

	// Get program path
	FileGetPath(cFileName, OutputBuffer, OutputBufferSize);
}

// Some older compilers have bad time with `#include <filesystem>`, so I added alternative iterator
#define NUM_LEV 20 // How deep dir iterator can go
static struct
{
	int CurLev;						// Current dir level
	HANDLE hFind[NUM_LEV];			// Stores search progress
	WIN32_FIND_DATA data[NUM_LEV];	// Stores file data
	char BaseDir[PATH_LEN];			// Stores base dir for current session
	char RetPath[PATH_LEN];			// Stores resulting file name
} GI;
static void DirGetBase(char * buf) // Gets base dir for currrent level (internal func)
{
	// Base dir
	strcpy(buf, GI.BaseDir);

	// Next dir levels
	for (int lv = 0; lv < GI.CurLev; lv++)
	{
		strcat(buf, GI.data[lv].cFileName);
		strcat(buf, DIR_SDELIM);
	}

	//DPRINT("[iter] get base: %s\n", buf);
}
void DirIterClose()
{
	// Close active searches
	for (int lv = 0; lv <= GI.CurLev; lv++)
	{
		if (GI.hFind != INVALID_HANDLE_VALUE)
			FindClose(GI.hFind[lv]);
	}

	// Reset list
	GI.CurLev = 0;
	GI.hFind[0] = INVALID_HANDLE_VALUE;
	GI.BaseDir[0] = '\0';
	GI.RetPath[0] = '\0';
}
void DirIterInit(const char * Dir)
{
	// Close prev session
	DirIterClose();

	// Store base dir (with deliminer at the end)
	strcpy(GI.BaseDir, Dir);
	int len = strlen(GI.BaseDir);
	if (!len || (len && GI.BaseDir[len-1] != DIR_CDELIM))
		strcat(GI.BaseDir, DIR_SDELIM);

	DPRINT("[iter]->(re)init, base: %s\n", GI.BaseDir);
}
const char * DirIterGet()
{
	char SearchStr[PATH_LEN] = "";

	bool Found = false;
	if (GI.hFind[GI.CurLev] == INVALID_HANDLE_VALUE)
	{
		DPRINT("[iter] get first\n");

		// Get base dir for current level
		DirGetBase(SearchStr);
		strcat(SearchStr, "*.*");

		// Find first entry
		GI.hFind[GI.CurLev] = FindFirstFile(SearchStr, &GI.data[GI.CurLev]);
		if (GI.hFind[GI.CurLev] != INVALID_HANDLE_VALUE)
			Found = true;
	}
	else
	{
		DPRINT("[iter] get next\n");

		// Find next entry
		if (FindNextFile(GI.hFind[GI.CurLev], &GI.data[GI.CurLev]))
			Found = true;
	}

	if (Found)
	{
		// Found something
		if (GI.data[GI.CurLev].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			/// Dir ///

			// Skip '.' and '..' recursively
			if (!strcmp(GI.data[GI.CurLev].cFileName, ".") || !strcmp(GI.data[GI.CurLev].cFileName, ".."))
				return DirIterGet();

			DPRINT("[iter] entering dir: %s\n", GI.data[GI.CurLev].cFileName);

			// Go one level up (inside directory)
			if (++(GI.CurLev) > NUM_LEV)
			{
				DPRINT("[%s] subdir level overflow!\n", __func__);
				exit(1);
			}
			GI.hFind[GI.CurLev] = INVALID_HANDLE_VALUE;
			return DirIterGet();
		}

		/// File ///
		DirGetBase(GI.RetPath);
		strcat(GI.RetPath, GI.data[GI.CurLev].cFileName);
		DPRINT("[iter]<-return file: %s\n", GI.RetPath);
		return GI.RetPath;
	}

	// Not found anything
	if (GI.CurLev)
	{
		DPRINT("[iter] leaving dir\n");
		
		// Go one level down (outside)
		if (GI.hFind[GI.CurLev] != INVALID_HANDLE_VALUE)
			FindClose(GI.hFind[GI.CurLev]);
		(GI.CurLev)--;
		return DirIterGet();
	}

	// End - stop
	DPRINT("[iter]<-not found, stop\n");
	return NULL;
}

#else // ToDo

// <filesystem> should be good for other OSes
	/*#include <filesystem>			// Directory iterators
	using namespace std;
	using namespace std::tr2::sys;

	// Count files in folder
	recursive_directory_iterator it1(cFolder), it2(cFolder), end;	// Recursive iterators
	FileCounter = 0;
	for (; it1 != end; ++it1)
		if (!is_directory(it1->path()))
			FileCounter++;

	// Allocate memory for list of files
	FileList = (sFileListEntry *)malloc(sizeof(sFileListEntry)*FileCounter);
	printf("Found %i files. \nPacking ...\n", FileCounter);

	// Fill list 
	FileCounter = 0;
	for (; it2 != end; ++it2)
	{
		if (!is_directory(it2->path()))
		{
			strcpy(cFile, (it2->path().parent_path().string().c_str()));
			strcat(cFile, "\\");
			strcat(cFile, (it2->path().filename().string().c_str()));

			SafeFileOpen(&ptrInputF, cFile, "rb");
			//printf("%s, %i\n", cFile, FileSize(&ptrInputF));

			FileList[FileCounter].Update(cFile, FileSize(&ptrInputF));
			FileCounter++;

			fclose(ptrInputF);
		}
	}*/

#endif