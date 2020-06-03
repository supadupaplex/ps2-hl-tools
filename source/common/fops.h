// Author:	supadupaplex
// License:	cheack readme.txt and license.txt

#ifndef FOPS_H
#define FOPS_H

#include <stdio.h>
//#include <direct.h>		// _mkdir()
#include <sys\stat.h>	// stat()
#include <windows.h>	// CreateDitectoryA()

#define PATH_LEN 256
#define DIR_SDELIM "\\" // <= PLATFORM-DEPENDENT
#define DIR_CDELIM '\\' // <= PLATFORM-DEPENDENT

size_t FileSize(FILE **ptrFile); // Reads file size
void FileReadBlock(FILE **ptrSrcFile, void * DstBuff, size_t Addr, size_t Size); // Reads chunk from file
void FileWriteBlock(FILE **ptrDstFile, const void * SrcBuff, size_t Addr, size_t Size); // Writes chunk to file
void FileWriteBlock(FILE **ptrDstFile, const void * SrcBuff, size_t Size); // Writes chunk to file from prev. pos
void SafeFileOpen(FILE **ptrFile, const char * FileName, char * Mode); // Opens file, checks that everything is alright 
void FileGetExtension(const char * Path, char * OutputBuffer, int OutputBufferSize); // Fetches extension from file name
void FileGetName(const char * Path, char * OutputBuffer, int OutputBufferSize, bool WithExtension); // Fetches short name from full file name
void FileGetFullName(const char * Path, char * OutputBuffer, int OutputBufferSize); // Cuts extension from full file name
void FileGetPath(const char * Path, char * OutputBuffer, int OutputBufferSize); // Fetches path from full file name
bool CheckFile(char * FileName); // Checks that file exists
bool CheckDir(const char * Path); // Check if Path is directory
void NewDir(const char * DirName); // Makes dir
void GenerateFolders(char * cPath); // Makes all dirs from the path if they don't exist
void PatchSlashes(char * cPathBuff, int BuffSize, bool SlashToBackslash); // Fixes slashes in path
void ProgGetPath(char * OutputBuffer, int OutputBufferSize); // Gets path to the executable file
void FileSafeRename(char * OldName, char * NewName); // Safe file rename
void DirIterInit(const char * Dir); // Init/reset dir iterator
void DirIterClose(); // Deinit dir iterator
const char * DirIterGet(); // Dir iterator, returns NULL on end

#endif
