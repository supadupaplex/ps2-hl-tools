// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This file contains functions that perform various file operations
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <dirent.h>
#endif

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

void SafeFileOpen(FILE **ptrFile, const char * FileName, const char * Mode)
{
	*ptrFile = fopen(FileName, Mode);

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
#ifdef _WIN32
		if (Path[i] == DIR_DELIM_CH || Path[i] == DIR_NOT_DELIM_CH) // windows can understand both
#else
		if (Path[i] == DIR_DELIM_CH)
#endif
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
#ifdef _WIN32
		if (Path[i] == DIR_DELIM_CH || Path[i] == DIR_NOT_DELIM_CH) // windows can understand both
#else
		if (Path[i] == DIR_DELIM_CH)
#endif
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
#ifdef _WIN32
		if (Path[i] == DIR_DELIM_CH || Path[i] == DIR_NOT_DELIM_CH) // windows can understand both
#else
		if (Path[i] == DIR_DELIM_CH)
#endif
			End = i + 1;
	}

	memcpy(OutputBuffer, Path, End);
	OutputBuffer[End] = '\0';
}

bool CheckFile(char * FileName)
{
	FILE * ptrTestFile;
	bool Result = false;

	ptrTestFile = fopen(FileName, "r");
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

void PatchSlashes(char * cPathBuff, int BuffSize, bool PakToFs)
{
	if (PakToFs == true)
	{
		// Convert slashes in path from PAK to FS
		for (int i = 0; i < BuffSize; i++)
			if (cPathBuff[i] == DIR_NOT_DELIM_CH)
				cPathBuff[i] = DIR_DELIM_CH;
	}
	else
	{
		// Convert slashes in path from FS to PAK
		for (int i = 0; i < BuffSize; i++)
			if (cPathBuff[i] == '\\')
				cPathBuff[i] = '/';
	}
}

void GenerateFolders(char * cPath)
{
	char PathBuffer[PATH_LEN];
	char * cDir;
	char cCurrent[PATH_LEN] = "";
	char Counter = 0;

	strcpy(PathBuffer, cPath);			// Save cPath to buffer, to avoid error, when parsing (const char *)

	cDir = strtok(PathBuffer, DIR_DELIM);	// First run: count words
	while (cDir != NULL)
	{
		Counter++;
		cDir = strtok(NULL, DIR_DELIM);
	}

	strcpy(PathBuffer, cPath);			// Give strtok() fresh string
	cDir = strtok(PathBuffer, DIR_DELIM);	// Second run: create folders
	while (cDir != NULL)
	{
		strcat(cCurrent, cDir);
		if (Counter > 1)
		{
			NewDir(cCurrent);
			Counter--;
		}
		strcat(cCurrent, DIR_DELIM);
		cDir = strtok(NULL, DIR_DELIM);
	}
}

//// Basic ZIP lookup functionality ////

#define ZIP_SIGN_LOCAL		0x04034B50 // LE PK\0x03\0x04
#define ZIP_SIGN_CENTRAL	0x02014B50 // LE PK\0x01\0x02
#define ZIP_SIGN_FOOTER		0x06054B50 // LE PK\0x05\0x06

#pragma pack(1)
typedef struct
{
	unsigned int sign;
	short ver;
	short flag;
	short compr_type;
	short last_time;
	short last_date;
	unsigned int crc32;
	int compr_sz;
	int decompr_sz;
	short name_sz;
	short extra_sz;
	// name, extra
} zip_local;

#pragma pack(1)
typedef struct
{
	unsigned int sign;
	short ver;
	short ver_min;
	short flag;
	short compr_type;
	short last_time;
	short last_date;
	unsigned int crc32;
	int compr_sz;
	int decompr_sz;
	short name_sz;
	short extra_sz;
	short comment_sz;
	short disk_start;
	short attr_int;
	int attr_ext;
	int loc_offset;
	// name, extra, comment
} zip_central;

#pragma pack(1)
typedef struct
{
	unsigned int sign;
	short disk_cur;
	short disk_central;
	short disk_records;
	short num_records;
	int central_sz;
	int central_offset;
	short comment_sz;
	// comment
} zip_footer;

bool ZipLookupFile(char *zip, char *file, int *offset, int *csz, int *dsz, ztype *type)
{
	FILE *pf;
	int fsz;
	bool found;
	int rec, fpos;
	char buf[PATH_LEN];

	zip_local zloc;
	zip_central zcent;
	zip_footer zfoot;

	SafeFileOpen(&pf, zip, "rb");
	fsz = FileSize(&pf);

	// Lookup footer and check //
	FileReadBlock(&pf, &zfoot, fsz - sizeof(zfoot), sizeof(zfoot));
	if (zfoot.sign != ZIP_SIGN_FOOTER ||
		zfoot.disk_records != zfoot.num_records)
		goto err;

	// Lookup central dir entry //
	fpos = zfoot.central_offset;
	found = false;
	for (rec = 0; rec < zfoot.num_records; rec++) {
		// Check next central header
		FileReadBlock(&pf, &zcent, fpos, sizeof(zcent));

		// Check signature
		if (zcent.sign != ZIP_SIGN_CENTRAL)
			goto err;

		// Check name
		fpos += sizeof(zcent);
		FileReadBlock(&pf, buf, fpos, zcent.name_sz);
		if (!strncmp(file, buf, zcent.name_sz))
		{
			found = true;
			break;
		}

		fpos += zcent.name_sz + zcent.extra_sz + zcent.comment_sz;
	}

	if (!found)
		goto err;

	// Lookup local dir entry //
	fpos = zcent.loc_offset;
	FileReadBlock(&pf, &zloc, fpos, sizeof(zloc));

	// Check signature
	if (zloc.sign != ZIP_SIGN_LOCAL)
		goto err;

	// Check name
	fpos += sizeof(zloc);
	FileReadBlock(&pf, buf, fpos, zloc.name_sz);
	if (strncmp(file, buf, zcent.name_sz))
		goto err;

	// Found the match, prepare the data to return
	*offset = fpos + zloc.name_sz + zloc.extra_sz;
	*csz = zloc.compr_sz;
	*dsz = zloc.decompr_sz;
	switch (zloc.compr_type)
	{
		case 0x00: *type = ZIP_NONE;    break;
		case 0x08: *type = ZIP_DEFLATE; break;
		default:   *type = ZIP_BAD;     break;
	};

	fclose(pf);
	return true;

err:
	fclose(pf);
	printf("Can't find %s in %s\n", file, zip);
	return false;
}

//// PLATFORM-DEPENDENT CODE BELOW ////

#ifdef _WIN32

bool CheckDir(const char * Path)
{
	DWORD Attr = GetFileAttributesA(Path);

	return (Attr != INVALID_FILE_ATTRIBUTES) && (Attr & FILE_ATTRIBUTE_DIRECTORY);
}

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
		strcat(buf, DIR_DELIM);
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
	if (!len)
		strcpy(GI.BaseDir, DIR_DELIM);		// empty
	else if (GI.BaseDir[len-1] == DIR_NOT_DELIM_CH)
		GI.BaseDir[len-1] = DIR_DELIM_CH;	// wrong delim
	else if (GI.BaseDir[len-1] != DIR_DELIM_CH)
		strcat(GI.BaseDir, DIR_DELIM);		// no delim

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

#else // linux

bool CheckDir(const char * Path)
{
	struct stat DirStat;

	stat(Path, &DirStat);

	if (DirStat.st_mode & S_IFDIR)
		return true;
	else
		return false;
}

void NewDir(const char * DirName)
{
	mkdir(DirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void ProgGetPath(char * OutputBuffer, int OutputBufferSize)
{
	// https://stackoverflow.com/questions/758018/path-to-binary-in-c
	readlink("/proc/self/exe", OutputBuffer, OutputBufferSize);
}

// Some older compilers have bad time with `#include <filesystem>`, so I added alternative iterator
#define NUM_LEV 20 // How deep dir iterator can go
static struct
{
	int CurLev;						// Current dir level
	DIR * dir[NUM_LEV];				// Stores search progress
	struct dirent * ent[NUM_LEV];	// Stores file data
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
		strcat(buf, GI.ent[lv]->d_name);
		strcat(buf, DIR_DELIM);
	}

	//DPRINT("[iter] get base: %s\n", buf);
}
void DirIterClose()
{
	// Close active searches
	for (int lv = 0; lv <= GI.CurLev; lv++)
	{
		if (GI.dir != NULL)
			closedir(GI.dir[lv]);
	}

	// Reset list
	GI.CurLev = 0;
	GI.dir[0] = NULL;
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
	if (!len || (len && GI.BaseDir[len-1] != DIR_DELIM_CH))
		strcat(GI.BaseDir, DIR_DELIM);

	DPRINT("[iter]->(re)init, base: %s\n", GI.BaseDir);
}
const char * DirIterGet()
{
	char SearchStr[PATH_LEN] = "";

	bool Found = false;
	if (GI.dir[GI.CurLev] == NULL)
	{
		DPRINT("[iter] get first\n");

		// Get base dir for current level
		DirGetBase(SearchStr);

		// Find first entry
		GI.dir[GI.CurLev] = opendir(SearchStr);
		if (GI.dir[GI.CurLev])
		{
			GI.ent[GI.CurLev] = readdir(GI.dir[GI.CurLev]);
			if (GI.ent[GI.CurLev] != NULL)
				Found = true;
		}
	}
	else
	{
		DPRINT("[iter] get next\n");

		// Find next entry
		GI.ent[GI.CurLev] = readdir(GI.dir[GI.CurLev]);
		if (GI.ent[GI.CurLev] != NULL)
			Found = true;
	}

	if (Found)
	{
		// Found something
		if (GI.ent[GI.CurLev]->d_type == DT_DIR)
		{
			/// Dir ///

			// Skip '.' and '..' recursively
			if (!strcmp(GI.ent[GI.CurLev]->d_name, ".") || !strcmp(GI.ent[GI.CurLev]->d_name, ".."))
				return DirIterGet();

			DPRINT("[iter] entering dir: %s\n", GI.ent[GI.CurLev]->d_name);

			// Go one level up (inside directory)
			if (++(GI.CurLev) > NUM_LEV)
			{
				DPRINT("[%s] subdir level overflow!\n", __func__);
				exit(1);
			}
			GI.dir[GI.CurLev] = NULL;
			return DirIterGet();
		}

		/// File ///
		DirGetBase(GI.RetPath);
		strcat(GI.RetPath, GI.ent[GI.CurLev]->d_name);
		DPRINT("[iter]<-return file: %s\n", GI.RetPath);
		return GI.RetPath;
	}

	// Not found anything
	if (GI.CurLev)
	{
		DPRINT("[iter] leaving dir\n");

		// Go one level down (outside)
		if (GI.dir[GI.CurLev] != NULL)
			closedir(GI.dir[GI.CurLev]);
		(GI.CurLev)--;
		return DirIterGet();
	}

	// End - stop
	DPRINT("[iter]<-not found, stop\n");
	return NULL;
}

#endif
