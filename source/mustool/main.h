// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This file contains all definitions and declarations
//

#ifndef MAIN_H
#define MAIN_H

////////// Includes??? //////////
#include <stdio.h>		// puts(), printf(), sscanf(), snprintf()
#include <string.h>		// strcpy(), strcat(), strlen(), strtok(), strncpy()
#include <malloc.h>		// malloc(), free()
#include <stdlib.h>		// exit()
#include <math.h>		// round(), ceil()
#include <ctype.h>		// tolower()

////////// Definitions //////////
#define PROG_VERSION "1.23"
#define VAG_PS2 0
#define VAG_NORMAL 1
#define VAG_UNSUPPORTED 2
#define WAV_PS2 0
#define WAV_NORMAL 1
#define WAV_UNSUPPORTED 2
#define UNKNOWN_FILE -1
#define PS2_WAV_NOLOOP 0xFFFFFFFF

////////// Typedefs //////////
#include "types.h"

////////// Functions //////////
#include "fops.h"

////////// Structures //////////

// VAG music file header
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sVAGHeader
{
	ulong Signature;		// "VAGp" (0x56414770) signature
	ulong Version;			// Should be 0x20 for PS2 HL
	ulong Magic1;			// = 0
	ulong DataSize;			// Size of file without header
	ulong SamplingF;		// Sampling frequency. Should be 44100 (0xAC44) for PS2 Half-life
	uchar Magic2[10];		// Filled with zeroes
	uchar Channels;			// 0-1 - one channel (mono), 2 - two channels (stereo). PS2 HL supports mono only
	uchar Magic3;			// = 0
	char Name[16];			// Internal file mane

	void UpdateFromFile(FILE ** ptrFile)	// Update header from file
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sVAGHeader));
	}

	void SwapEndian()						// Swap endian after reading and before writing to file
	{
		Signature = UTIL_BSWAP32(Signature);
		Version = UTIL_BSWAP32(Version);
		Magic1 = UTIL_BSWAP32(Magic1);
		DataSize = UTIL_BSWAP32(DataSize);
		SamplingF = UTIL_BSWAP32(SamplingF);
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
		this->Signature = 0x56414770;	// "VAGp" (0x56414770) signature
		this->Version = 0x20;			// Should be 0x20 for PS2 HL
		this->Magic1 = 0;				// = 0
		this->DataSize = NewDataSize;	// Size of file without header
		this->SamplingF = 44100;		// Sampling frequency. Should be 44100 (0xAC44) for PS2 Half-life
		this->Channels = 0;				// 0-1 - one channel (mono), 2 - two channels (stereo). PS2 HL supports mono only
		this->Magic3 = 0;				// = 0
		snprintf(this->Name, sizeof(Name), "%s", NewName);		// Internal file mane
	}
};

// Chunks of WAV file header
// "RIFF" chunk
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sRIFF
{
	ulong RiffSignature;	// "RIFF" (LE: 0x46464952) chunk signature	
	ulong RiffSize;			// Riff chunk size (File size - 8)
};

// "WAVEfmt " chunk
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sWAVEFMT
{
	ulong WaveSignature;	// "WAVE" (LE: 0x45564157) chunk signature
	ulong FmtSignature;		// "fmt " (LE: 0x20746d66) subchunk signature
	ulong FmtSize;			// Fmt subchunk size (16 for PCM)
	ushort Format;			// Audio format (1 for PCM)
	ushort Channels;		// Number of channels (1 for mono, 2 for stereo)
	ulong SamplingF;		// Sampling frequency (11025/22050 for PS2 HL wav's)
	ulong ByteRate;			// Byte rate (SamplingF * Channels * BitsPerSample / 8)
	ushort BytesPerSample;	// Bytes fer sample for all channels (Channels * BitsPerSample / 8)
	ushort BitsPerSample;	// Bits per sample for each channel (8, 16, etc)
};

// "data" chunk
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sDATA
{
	ulong DataSignature;	// "data" (LE: 0x61746164) subchunk signature
	ulong DataSize;			// Data subchunk size (SamplesNum = DataSize \ (Channels * (BitsPerSample / 8)))
};

// My poorly assembled loop chunk
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sLOOP
{
	//uchar Spacer;
	ulong Cue;
	ulong CueSz;
	ulong CueVal1;
	ulong CueVal2;
	ulong CueVal3;
	ulong Data;
	ulong DataSz;
	ulong DataVal1;
	ulong DataVal2;
	ulong List;
	ulong ListSz;
	ulong Adtl;
	ulong Ltxt;
	ulong LtxtSz;
	ulong LtxtVal1;
	ulong LtxtVal2;
	ulong Mark;
	ulong MarkSz;
	ulong MarkVal;

	void Init(ulong DataSize, ulong LoopStart)
	{
		//Spacer = 0;				// 00
		Cue = 0x20657563;		// 63 75 65 20, "cue "
		CueSz = 0x1C;			// 1C 00 00 00, size from here to LIST
		CueVal1 = 1;			// 01 00 00 00
		CueVal2 = LoopStart;	// F8 02 00 00, [Loop?, 2F8]
		CueVal3 = LoopStart;	// F8 02 00 00, [Loop?, 2F8]
		Data = 0x61746164;		// 64 61 74 61, "data"
		DataSz = 0;				// 00 00 00 00
		DataVal1 = 0;			// 00 00 00 00
		DataVal2 = LoopStart;	// 00 00 00 00, this should be loop start
		List = 0x5453494C;		// 4C 49 53 54, "LIST"
		ListSz = 0x20;			// 20 00 00 00, size from here to end
		Adtl = 0x6C746461;		// 61 64 74 6C, "adtl"
		Ltxt = 0x7478746C;		// 6C 74 78 74, "ltxt"
		LtxtSz = 0x14;			// 14 00 00 00, size from here to end
		LtxtVal1 = LoopStart;	// F8 02 00 00, [Loop?, 2F8]
		LtxtVal2 = DataSize;	// 11 2B 00 00, [Size of data chunk]
		Mark = 0x6B72616D;		// 6D 61 72 6B, "mark"
		MarkSz = 0;				// 00 00 00 00
		MarkVal = 0;			// 00 00 00 00
	}
};

// Normal WAV audio file header
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sNormalWAVHeader
{
	sRIFF RiffChunk;
	sWAVEFMT WaveChunk;
	sDATA DataChunk;

	// Not present in WAV files, added for the sake of convinience
	bool Looped;
	ulong DataOffset;
	ulong LoopStart;
};

// PS2 HL WAV audio file compressed header
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
struct sPS2WAVHeader
{
	ulong DataSize;			// Same as in normal WAV file
	ulong LoopStart;			// = 0xFFFFFFFF
	ulong SamplingF;		// Same as in normal WAV file
	ulong Magic1;			// = 1, number of channels?
	ulong Magic2;			// = 0x00000000
};

// Unified WAV audio file header
#pragma pack(1)				// Eliminate unwanted 0x00 bytes
union uWAVHeader
{
	sNormalWAVHeader Normal;
	sPS2WAVHeader PS2;

	void UpdateFromPS2(FILE ** ptrFile)	// Update header from PS2 file
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sPS2WAVHeader));
	}

	void UpdateFromNormal(FILE ** ptrFile)	// Update header from PC file
	{
		// Clear struct (if some chunks are missing then zeroes would not pass ChechType())
		memset(this, 0x00, sizeof(sNormalWAVHeader));

		// Find all teh chunks
		int Current = 0;
		uchar Temp[3];
		Normal.Looped = false;				// Loop check
		Normal.LoopStart = PS2_WAV_NOLOOP;	// Loop start
		while (Current != EOF)
		{
			// Check first letters
			Current = fgetc(*ptrFile);

			if (Current == 'R')
			{
				// Check other letters
				fread(Temp, sizeof(Temp), 1, *ptrFile);
				if (Temp[0] == 'I' && Temp[1] == 'F' && Temp[2] == 'F')
				{
					// Load chunk
					this->Normal.RiffChunk.RiffSignature = 0x46464952;
					fread(&this->Normal.RiffChunk.RiffSize, sizeof(sRIFF) - 4, 1, *ptrFile);
				}
			}
			if (Current == 'W')
			{
				// Check other letters
				fread(Temp, sizeof(Temp), 1, *ptrFile);
				if (Temp[0] == 'A' && Temp[1] == 'V' && Temp[2] == 'E')
				{
					// Load chunk
					this->Normal.WaveChunk.WaveSignature = 0x45564157;
					fread(&this->Normal.WaveChunk.FmtSignature, sizeof(sWAVEFMT) - 4, 1, *ptrFile);
				}
			}
			if (Current == 'd')
			{
				// Check other letters
				fread(Temp, sizeof(Temp), 1, *ptrFile);
				if (Temp[0] == 'a' && Temp[1] == 't' && Temp[2] == 'a')
				{
					// Update main data fields only once
					if (this->Normal.DataChunk.DataSignature == 0)
					{
						// Load chunk
						this->Normal.DataChunk.DataSignature = 0x61746164;
						fread(&this->Normal.DataChunk.DataSize, sizeof(sDATA) - 4, 1, *ptrFile);
						Normal.DataOffset = ftell(*ptrFile);
					}
					else if (this->Normal.LoopStart == PS2_WAV_NOLOOP)
					{
						// Fetch loop start second time
						ulong SomeData[3];
						fread(&SomeData, sizeof(SomeData), 1, *ptrFile);
						if (SomeData[0] == 0 && SomeData[1] == 0)
							Normal.LoopStart = SomeData[2];
					}
				}
			}

			if (Current == 'm' || Current == 'M')
			{
				// Check other letters
				fread(Temp, sizeof(Temp), 1, *ptrFile);
				if ((Temp[0] == 'a' && Temp[1] == 'r' && Temp[2] == 'k')
					|| (Temp[0] == 'A' && Temp[1] == 'R' && Temp[2] == 'K'))
				{
					// Set loop flag
					Normal.Looped = true;
				}
			}
		}

		// If loop start is missing then assume that is is 0
		if (Normal.Looped == true)
			if (Normal.LoopStart == PS2_WAV_NOLOOP)
				Normal.LoopStart = 0;
	}

	void ConvertToNormal()
	{
		// Save values
		ulong SamplingF = PS2.SamplingF;
		ulong DataSize = PS2.DataSize;
		ulong BitsPerSample = 8;
		ulong Channels = PS2.Magic1;
		ulong LoopStart = PS2.LoopStart;

		// Clear memory
		memset(this, 0x00, sizeof(sNormalWAVHeader));

		// Update extra stuff
		this->Normal.DataOffset = sizeof(sNormalWAVHeader) - sizeof(bool) - 2 * sizeof(ulong);	// - sizeof(Looped) - sizeod(DataOffset) - sizeof(LoopStart)
		if (LoopStart == PS2_WAV_NOLOOP)
		{
			this->Normal.Looped = false;
			this->Normal.LoopStart = 0;
		}
		else
		{
			this->Normal.Looped = true;
			this->Normal.LoopStart = LoopStart;
		}

		// Update main structure
		this->Normal.RiffChunk.RiffSignature = 0x46464952;
		this->Normal.RiffChunk.RiffSize = DataSize	+ Normal.DataOffset
													- (sizeof(Normal.RiffChunk.RiffSignature)
													+ sizeof(Normal.RiffChunk.RiffSize));
		this->Normal.WaveChunk.WaveSignature = 0x45564157;
		this->Normal.WaveChunk.FmtSignature = 0x20746d66;
		this->Normal.WaveChunk.FmtSize = 16;		// 16 for PCM
		this->Normal.WaveChunk.Format = 1;			// 1 for PCM
		this->Normal.WaveChunk.Channels = Channels;
		this->Normal.WaveChunk.SamplingF = SamplingF;
		this->Normal.WaveChunk.ByteRate = SamplingF * Channels * BitsPerSample / 8;
		this->Normal.WaveChunk.BytesPerSample = Channels * BitsPerSample / 8;
		this->Normal.WaveChunk.BitsPerSample = BitsPerSample;
		this->Normal.DataChunk.DataSignature = 0x61746164;
		this->Normal.DataChunk.DataSize = DataSize;
	}

	void ConvertToPS2()
	{
		// Save values
		ulong SamplingF = Normal.WaveChunk.SamplingF;
		ulong DataSize = Normal.DataChunk.DataSize;
		ulong Channels = Normal.WaveChunk.Channels;
		ulong LoopStart = Normal.Looped ? Normal.LoopStart : PS2_WAV_NOLOOP;

		// Clear memory
		memset(this, 0x00, sizeof(sPS2WAVHeader));

		// Update structure
		this->PS2.DataSize = DataSize;
		this->PS2.LoopStart = LoopStart;
		this->PS2.SamplingF = SamplingF;
		this->PS2.Magic1 = Channels;
		this->PS2.Magic2 = 0;
	}

	uchar CheckType(ulong FileSize)
	{
		if (this->Normal.RiffChunk.RiffSignature == 0x46464952
			&& this->Normal.WaveChunk.WaveSignature == 0x45564157
			&& this->Normal.WaveChunk.FmtSignature == 0x20746d66
			&& this->Normal.DataChunk.DataSignature == 0x61746164)
		{
			if (this->Normal.WaveChunk.Channels == 1
				&& this->Normal.WaveChunk.BitsPerSample == 8
				&& (this->Normal.WaveChunk.SamplingF == 11025
				|| this->Normal.WaveChunk.SamplingF == 22050
				|| this->Normal.WaveChunk.SamplingF == 44100))
				return WAV_NORMAL;
			else
				return WAV_UNSUPPORTED;
		}
		else
		{
			if (this->PS2.Magic2 == 0 && (FileSize - sizeof(sPS2WAVHeader) - this->PS2.DataSize) <= 32)
				return WAV_PS2;
			else
				return UNKNOWN_FILE;
		}
	}
};

#endif // MAIN_H
