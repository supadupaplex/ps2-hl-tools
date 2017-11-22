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
// This file contains all definitions and declarations
//

#pragma once

////////// Includes??? //////////
#include <stdio.h>		// puts(), printf(), sscanf(), snprintf()
#include <conio.h>		// _getch()
#include <direct.h>		// _mkdir()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round(), ceil()
#include <ctype.h>		// tolower()
#include <sys\stat.h>	// stat()
#include <windows.h>	// CreateDitectoryA()
#include <intrin.h>		// Byte swap functions (swap endian for VAG)

////////// Definitions //////////
#define PROG_VERSION "1.1"
#define VAG_PS2 0
#define VAG_NORMAL 1
#define VAG_UNSUPPORTED 2
#define WAV_PS2 0
#define WAV_NORMAL 1
#define WAV_UNSUPPORTED 2
#define UNKNOWN_FILE -1

////////// Typedefs //////////
typedef unsigned short int ushort;
typedef unsigned long int ulong;
typedef unsigned int uint;
typedef unsigned char uchar;

////////// Functions //////////
ulong FileSize(FILE **ptrFile);																			// Get size of file
void FileReadBlock(FILE **ptrSrcFile, void * DstBuff, ulong Addr, ulong Size);							// Read block from file to buffer
void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Addr, ulong Size);							// Write data from buffer to file
void FileWriteBlock(FILE **ptrDstFile, void * SrcBuff, ulong Size);										// Write data from buffer to file
void SafeFileOpen(FILE **ptrFile, const char * FileName, char * Mode);									// Try to open file, if problem oocur then exit
void FileGetExtension(const char * Path, char * OutputBuffer, uint OutputBufferSize);					// Get file extension
void FileGetName(const char * Path, char * OutputBuffer, uint OutputBufferSize, bool WithExtension);	// Get name of file with or without extension
void FileGetFullName(const char * Path, char * OutputBuffer, uint OutputBufferSize);					// Get full file name (with folders) without extension
void FileGetPath(const char * Path, char * OutputBuffer, uint OutputBufferSize);						// Get file path
bool CheckFile(char * FileName);																		// Check existance of file
void GenerateFolders(char * cPath);																		// Make sure, that all folders in path are existing
void PatchSlashes(char * cPathBuff, ulong BuffSize, bool SlashToBackslash);								// Patch slashes when transitioning between PAK and Windows file names
bool CheckDir(const char * Path);																		// Check if path is directory
void NewDir(const char * DirName);																		// Create directory
void ProgGetPath(char * OutputBuffer, uint OutputBufferSize);											// Get path of this program

////////// Structures //////////

// VAG music file header
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sVAGHeader
{
	ulong Signature;		// “VAGp” (0x56414770) signature
	ulong Version;			// Should be 0x20 for PS2 HL
	ulong Magic1;			// = 0
	ulong DataSize;			// Size of file without header
	ulong SamplingF;		// Sampling frequency. Should be 44100 (0xAC44) for PS2 Half-life
	uchar Magic2[10];		// Filled with zeroes
	uchar Channels;			// 0-1 – one channel (mono), 2 – two channels (stereo). PS2 HL supports mono only
	uchar Magic3;			// = 0
	char Name[16];			// Internal file mane

	void UpdateFromFile(FILE ** ptrFile)	// Update header from file
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sVAGHeader));
	}

	void SwapEndian()						// Swap endian after reading and before writing to file
	{
		Signature = _byteswap_ulong(Signature);
		Version = _byteswap_ulong(Version);
		Magic1 = _byteswap_ulong(Magic1);
		DataSize = _byteswap_ulong(DataSize);
		SamplingF = _byteswap_ulong(SamplingF);
	}

	uchar CheckType()
	{
		if (this->Signature == 0x56414770 && this->Version == 0x20 && this->Magic1 == 0 && this->Magic3 == 0)
		{
			if ((this->Channels == 0 || this->Channels == 1) && this->SamplingF == 44100)
				return VAG_NORMAL;
			else
				return VAG_UNSUPPORTED;
		}
		else
		{
			if (this->Signature == 0 && this->Version == 0 && this->Magic1 == 0 && this->DataSize == 0)		// Check if first 16 bytes are filled with zeroes (like in PS2 VAG files)
				return VAG_PS2;
			else
				return UNKNOWN_FILE;
		}
	}

	void Update(ulong NewDataSize, const char * NewName)
	{
		// Clear memory
		memset(this, 0x00, sizeof(sVAGHeader));

		// Update structure
		this->Signature = 0x56414770;	// “VAGp” (0x56414770) signature
		this->Version = 0x20;			// Should be 0x20 for PS2 HL
		this->Magic1 = 0;				// = 0
		this->DataSize = NewDataSize;	// Size of file without header
		this->SamplingF = 44100;		// Sampling frequency. Should be 44100 (0xAC44) for PS2 Half-life
		this->Channels = 0;				// 0-1 – one channel (mono), 2 – two channels (stereo). PS2 HL supports mono only
		this->Magic3 = 0;				// = 0
		snprintf(this->Name, sizeof(Name), "%s", NewName);		// Internal file mane
	}
};

// WAV audio file header
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sWAVHeader
{
	ulong RiffSignature;	// "RIFF" (LE: 0x46464952) chunk signature	
	ulong RiffSize;			// Riff chunk size (File size - 8)
	ulong WaveSignature;	// "WAVE" (LE: 0x45564157) chunk signature
	ulong FmtSignature;		// "fmt " (LE: 0x20746d66) subchunk signature
	ulong FmtSize;			// Fmt subchunk size (16 for PCM)
	ushort Format;			// Audio format (1 for PCM)
	ushort Channels;		// Number of channels (1 for mono, 2 for stereo)
	ulong SamplingF;		// Sampling frequency (11025 for PS2 HL wav's)
	ulong ByteRate;			// Byte rate (SamplingF * Channels * BitsPerSample / 8)
	ushort BytesPerSample;	// Bytes fer sample for all channels (Channels * BitsPerSample / 8)
	ushort BitsPerSample;	// Bits per sample for each channel (8, 16, etc)
	ulong DataSignature;	// "data" (LE: 0x61746164) subchunk signature
	ulong DataSize;			// Data subchunk size (SamplesNum = DataSize \ (Channels * (BitsPerSample / 8)))

	void UpdateFromFile(FILE ** ptrFile)	// Update header from secified file
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sWAVHeader));
	}

	void Update(ushort NewChannels, ulong NewSamplingF, ushort NewBitsPerSample, ulong NewDataSize)
	{
		// Clear memory
		memset(this, 0x00, sizeof(sWAVHeader));

		// Update structure
		this->RiffSignature = 0x46464952;
		this->RiffSize = NewDataSize + sizeof(sWAVHeader) - (sizeof(RiffSignature) + sizeof(RiffSize));
		this->WaveSignature = 0x45564157;
		this->FmtSignature = 0x20746d66;
		this->FmtSize = 16;		// 16 for PCM
		this->Format = 1;		// 1 for PCM
		this->Channels = NewChannels;
		this->SamplingF = NewSamplingF;
		this->ByteRate = this->SamplingF * this->Channels * NewBitsPerSample / 8;
		this->BytesPerSample = this->Channels * NewBitsPerSample / 8;
		this->BitsPerSample = NewBitsPerSample;
		this->DataSignature = 0x61746164;
		this->DataSize = NewDataSize;
	}

	uchar CheckType(ulong FileSize)
	{
		if (this->RiffSignature == 0x46464952 && this->WaveSignature == 0x45564157 && this->FmtSignature == 0x20746d66 && this->DataSignature == 0x61746164)
		{
			if (this->Channels == 1 && this->SamplingF == 11025 && this->BitsPerSample == 16)
				return WAV_NORMAL;
			else
				return WAV_UNSUPPORTED;
		}
		else
		{
			if (0 <= (FileSize - 4 - this->RiffSignature) && (FileSize - 4 - this->RiffSignature) <= 32)		// Check if first 4 bytes represent file size (like in PS2's unreadable WAV files)
				return WAV_PS2;
			else
				return UNKNOWN_FILE;
		}
	}


};
