// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// This file contains music patch and unpatch functions
//

////////// Includes //////////
#include "util.h"
#include "main.h"

////////// Functions //////////
void PatchVAG(const char * FileName);						// Add header to VAG file (normal format)
void UnpatchVAG(const char * FileName);						// Remove header from VAG file (PS2 HL music format)
uchar CheckVAG(const char * FileName, bool PrintInfo);		// Check VAG audio file type
void UnpatchWAV(const char * FileName);						// Remove header from WAV file
void PatchWAV(const char * FileName);						// Add header to WAV file
uchar CheckWAV(const char * FileName, bool PrintInfo);		// Check WAV audio file type


void UnpatchVAG(const char * FileName)		// Remove header from VAG file (PS2 HL music format)
{
	FILE * ptrInFile;		// Input file stream
	FILE * ptrOutFile;		// Output file stream

	sVAGHeader VAGHeader;	// VAG file header
	uchar * AudioData;		// Audio data pointer
	ulong AudioDataSize;	// Size of audio data

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Get header from file
	VAGHeader.UpdateFromFile(&ptrInFile);
	VAGHeader.SwapEndian();

	// Check VAG
	if (VAGHeader.CheckType() == VAG_NORMAL)
	{
		printf("Internal name: \"%s\", Channels: %i, Sampling frequency: %i \n", VAGHeader.Name, (VAGHeader.Channels == 0 ? 1 : VAGHeader.Channels), VAGHeader.SamplingF);
	}
	else if (VAGHeader.CheckType() == VAG_UNSUPPORTED)
	{
		printf("Internal name: \"%s\", Channels: %i, Sampling frequency: %i \n", VAGHeader.Name, (VAGHeader.Channels == 0 ? 1 : VAGHeader.Channels), VAGHeader.SamplingF);
		printf("Warning: PS2 HL supports only 1 channel 44100 Hz audio. \nYou may encounter problems with this file. \n");
		UTIL_WAIT_KEY("Press any key to confirm ...");
	}
	else if (VAGHeader.CheckType() == VAG_PS2)
	{
		puts("File already in PS2 music format ...");
		return;
	}
	else
	{
		puts("Incorrect VAG music file ...");
		return;
	}
	puts("Unpatching VAG music file ...");

	// Read audio data
	AudioDataSize = FileSize(&ptrInFile) - sizeof(sVAGHeader);
	AudioData = (uchar *)malloc(AudioDataSize);
	FileReadBlock(&ptrInFile, AudioData, sizeof(sVAGHeader), AudioDataSize);

	// Close input file
	fclose(ptrInFile);

	// Open output file (same as input)
	SafeFileOpen(&ptrOutFile, FileName, "wb");

	// Write audio data only
	FileWriteBlock(&ptrOutFile, AudioData, AudioDataSize);

	// Free memory
	free(AudioData);

	// Close output file
	fclose(ptrOutFile);

	puts("Done \n");
}

void PatchVAG(const char * FileName)	// Add header to VAG file (normal format)
{
	FILE * ptrInFile;		// Input file stream
	FILE * ptrOutFile;		// Output file stream

	sVAGHeader VAGHeader;	// VAG file header
	char cNewVAGName[64];	// New internal VAG Name
	uchar * AudioData;		// Audio data pointer
	ulong AudioDataSize;	// Size of audio data

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Get header from file
	VAGHeader.UpdateFromFile(&ptrInFile);
	VAGHeader.SwapEndian();

	// Check VAG
	if (VAGHeader.CheckType() == VAG_PS2)
	{
		puts("Patching PS2 VAG music file ...");
	}
	else if (VAGHeader.CheckType() == VAG_NORMAL || VAGHeader.CheckType() == VAG_UNSUPPORTED)
	{
		puts("This file is already patched ...");
		return;
	}
	else
	{
		puts("Incorrect VAG music file ...");
		return;
	}

	// Read audio data
	AudioDataSize = FileSize(&ptrInFile);
	AudioData = (uchar *)malloc(AudioDataSize);
	FileReadBlock(&ptrInFile, AudioData, 0, AudioDataSize);

	// Close input file
	fclose(ptrInFile);

	// Open output file (same as input)
	SafeFileOpen(&ptrOutFile, FileName, "wb");

	// Update header
	FileGetName(FileName, cNewVAGName, sizeof(cNewVAGName), false);
	VAGHeader.Update(AudioDataSize, cNewVAGName);
	VAGHeader.SwapEndian();

	// Write header and audio data
	FileWriteBlock(&ptrOutFile, &VAGHeader, sizeof(sVAGHeader));
	FileWriteBlock(&ptrOutFile, AudioData, AudioDataSize);

	// Free memory
	free(AudioData);

	// Close output file
	fclose(ptrOutFile);

	puts("Done \n");
}

uchar CheckVAG(const char * FileName, bool PrintInfo)	// Check VAG audio file type
{
	FILE * ptrInputFile;
	sVAGHeader VAGHeader;
	uchar VAGType;

	// Open file
	SafeFileOpen(&ptrInputFile, FileName, "rb");

	// Check type
	VAGHeader.UpdateFromFile(&ptrInputFile);
	VAGHeader.SwapEndian();
	VAGType = VAGHeader.CheckType();

	// Output info
	if (PrintInfo == true)
	{
		if (VAGType == VAG_NORMAL || VAGType == VAG_UNSUPPORTED)
		{
			puts("Type: normal VAG music file.");
			printf("Internal name: \"%s\", Channels: %i, Sampling frequency: %i \n", VAGHeader.Name, (VAGHeader.Channels == 0? 1 : VAGHeader.Channels), VAGHeader.SamplingF);
		}
		else if (VAGType == VAG_PS2)
		{
			puts("Type: PS2 Half-Life VAG music file.");
			puts("PS2 HL supports only 1 channel 44100 Hz audio.");
		}
		else
		{
			puts("Type: incorrect VAG music file.");
		}
	}

	// Close file
	fclose(ptrInputFile);

	return VAGType;
}

void UnpatchWAV(const char * FileName)		// Remove header from WAV file
{
	FILE * ptrInFile;		// Input file stream
	FILE * ptrOutFile;		// Output file stream

	uWAVHeader WAVHeader;	// WAV file header
	uchar * AudioData;		// Audio data pointer
	ulong AudioDataSize;	// Size of audio data

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Get header from file
	WAVHeader.UpdateFromNormal(&ptrInFile);

	// Check type
	if (WAVHeader.CheckType(FileSize(&ptrInFile)) == WAV_NORMAL)
	{
		printf("Normal WAV: Channels: %i, Sampling frequency: %i, BitsPerSample: %i \n",
			WAVHeader.Normal.WaveChunk.Channels,
			WAVHeader.Normal.WaveChunk.SamplingF,
			WAVHeader.Normal.WaveChunk.BitsPerSample);
	}
	else if (WAVHeader.CheckType(FileSize(&ptrInFile)) == WAV_UNSUPPORTED)
	{
		printf("Channels: %i, Sampling frequency: %i, BitsPerSample: %i \n",
			WAVHeader.Normal.WaveChunk.Channels,
			WAVHeader.Normal.WaveChunk.SamplingF,
			WAVHeader.Normal.WaveChunk.BitsPerSample);
		printf("Warning: PS2 HL supports only 8-bit 1 channel 11025/22050/44100 Hz audio. \nYou may encounter problems with this file. \n");
		UTIL_WAIT_KEY("Press any key to confirm ...");
	}
	else
	{
		puts("Bad WAV file ...");
		return;
	}

	if (WAVHeader.Normal.Looped == true)
		printf("Looped sound detected, start sample: %d \n", WAVHeader.Normal.LoopStart);

	puts("Unpatching WAV music file ...");

	// Read audio data
	AudioDataSize = WAVHeader.Normal.DataChunk.DataSize + 16 -
		(WAVHeader.Normal.DataChunk.DataSize + sizeof(sPS2WAVHeader)) % 16;	// Audio data should be aligned within 16-byte blocks
	AudioData = (uchar *)malloc(AudioDataSize);
	memset(AudioData, 0x00, AudioDataSize);
	FileReadBlock(&ptrInFile, AudioData, WAVHeader.Normal.DataOffset, WAVHeader.Normal.DataChunk.DataSize);

	// Close input file
	fclose(ptrInFile);

	// Open output file (same as input)
	SafeFileOpen(&ptrOutFile, FileName, "wb");
	
	// Convert header
	WAVHeader.ConvertToPS2();

	// Convert audio data
	for (ulong Byte = 0; Byte < AudioDataSize; Byte++)
		AudioData[Byte] += 0x80;

	// Write data
	FileWriteBlock(&ptrOutFile, &WAVHeader, sizeof(sPS2WAVHeader));
	FileWriteBlock(&ptrOutFile, AudioData, AudioDataSize);

	// Free memory
	free(AudioData);

	// Close output file
	fclose(ptrOutFile);

	puts("Done \n");
}

void PatchWAV(const char * FileName)	// Add header to WAV file
{
	FILE * ptrInFile;		// Input file stream
	FILE * ptrOutFile;		// Output file stream

	uWAVHeader WAVHeader;	// WAV file header
	uchar * AudioData;		// Audio data pointer
	ulong AudioDataSize;	// Size of audio data

	// Open file
	SafeFileOpen(&ptrInFile, FileName, "rb");

	// Get header from file
	WAVHeader.UpdateFromPS2(&ptrInFile);

	// Check type
	if (WAVHeader.CheckType(FileSize(&ptrInFile)) == WAV_PS2)
	{
		printf("PS2 WAV: Sampling frequency: %i \n", WAVHeader.PS2.SamplingF);
		if (WAVHeader.PS2.LoopStart != PS2_WAV_NOLOOP)
			printf("Looped sound detected, start sample: %d \n", WAVHeader.PS2.LoopStart);
	}
	else
	{
		puts("Bad PS2 WAV file ...");
		return;
	}
	puts("Patching PS2 WAV audio file ...");

	// Read audio data
	AudioDataSize = WAVHeader.PS2.DataSize;
	AudioData = (uchar *)malloc(AudioDataSize);
	FileReadBlock(&ptrInFile, AudioData, sizeof(sPS2WAVHeader), AudioDataSize);

	// Close input file
	fclose(ptrInFile);

	// Open output file (same as input)
	SafeFileOpen(&ptrOutFile, FileName, "wb");

	// Convert header
	WAVHeader.ConvertToNormal();

	// Convert audio data
	for (ulong Byte = 0; Byte < AudioDataSize; Byte++)
		AudioData[Byte] += 0x80;

	// Check if looped
	bool Spacer;
	sLOOP * LoopChunk = NULL;
	if (WAVHeader.Normal.Looped == true)
	{
		LoopChunk = (sLOOP *) malloc(sizeof(sLOOP));
		if (LoopChunk == NULL)
		{
			puts("Can't allocate memory ...");
			exit(EXIT_FAILURE);
		}
		LoopChunk->Init(WAVHeader.Normal.DataChunk.DataSize, WAVHeader.Normal.LoopStart);
		WAVHeader.Normal.RiffChunk.RiffSize += sizeof(sLOOP);
		Spacer = WAVHeader.Normal.DataChunk.DataSize % 2;
	}

	// Write header and audio data
	FileWriteBlock(&ptrOutFile, &WAVHeader, WAVHeader.Normal.DataOffset);	// DataOffset = size of WAV header
	FileWriteBlock(&ptrOutFile, AudioData, AudioDataSize);
	if (LoopChunk != NULL)
	{
		if (Spacer)
			fputc(0x00, ptrOutFile);
		FileWriteBlock(&ptrOutFile, LoopChunk, sizeof(sLOOP));
		free(LoopChunk);
	}

	// Free memory
	free(AudioData);

	// Close output file
	fclose(ptrOutFile);

	puts("Done \n");
}

uchar CheckWAV(const char * FileName, bool PrintInfo)	// Check WAV audio file type
{
	FILE * ptrInputFile;
	uWAVHeader NormWAVHeader, PS2WAVHeader;
	uchar NormWAVType, PS2WAVType;

	// Open file
	SafeFileOpen(&ptrInputFile, FileName, "rb");

	// Check type
	NormWAVHeader.UpdateFromNormal(&ptrInputFile);
	PS2WAVHeader.UpdateFromPS2(&ptrInputFile);
	NormWAVType = NormWAVHeader.CheckType(FileSize(&ptrInputFile));
	PS2WAVType = PS2WAVHeader.CheckType(FileSize(&ptrInputFile));

	// Close file
	fclose(ptrInputFile);

	// Output info
	if (NormWAVType == WAV_NORMAL || NormWAVType == WAV_UNSUPPORTED)
	{
		if (PrintInfo == true)
		{
			puts("Type: normal WAV audio file.");
			printf("Channels: %i, Sampling frequency: %i, BitsPerSample: %i \n",
				NormWAVHeader.Normal.WaveChunk.Channels,
				NormWAVHeader.Normal.WaveChunk.SamplingF,
				NormWAVHeader.Normal.WaveChunk.BitsPerSample);
		}

		return NormWAVType;
	}
	else if (PS2WAVType == WAV_PS2)
	{
		if (PrintInfo == true)
		{
			puts("Type: PS2 Half-Life WAV audio file with compressed header.");
			printf("Sampling frequency: %i \n", PS2WAVHeader.PS2.SamplingF);
		}

		return WAV_PS2;
	}
	else
	{
		if (PrintInfo == true)
			puts("Type: bad WAV file");

		return WAV_UNSUPPORTED;
	}

	return 0;
}

int main(int argc, char * argv[])
{
	FILE * ptrInputFile;
	FILE * ptrConfigFile;
	char ConfigFilePath[PATH_LEN];
	char Line[80];

	// Output info
	puts(PROG_TITLE);

	// Check arguments
	if (argc == 1)
	{
		// No arguments - show help screen
		puts(PROG_INFO);
		UTIL_WAIT_KEY("Press any key to exit ...");
	}
	else if (argc == 2)
	{
		char cFileExtension[5];

		FileGetExtension(argv[1], cFileExtension, 5);

		printf("\nProcessing file: %s\n", argv[1]);

		if (!strcmp(".vag", cFileExtension))
		{
			if (CheckVAG(argv[1], false) == VAG_PS2)
			{
				PatchVAG(argv[1]);
			}
			else if (CheckVAG(argv[1], false) == VAG_NORMAL || CheckVAG(argv[1], false) == VAG_UNSUPPORTED)
			{
				UnpatchVAG(argv[1]);
			}
			else
			{
				puts("Can't recognise VAG audio file ...");
			}
		}
		if (!strcmp(".wav", cFileExtension))
		{
			uchar WAVType = CheckWAV(argv[1], false);

			if (WAVType == WAV_PS2)
			{
				PatchWAV(argv[1]);
			}
			else if (WAVType == WAV_NORMAL || WAVType == WAV_UNSUPPORTED)
			{
				UnpatchWAV(argv[1]);
			}
			else
			{
				puts("Can't recognise WAV audio file ...");
			}
		}
		else
		{
			puts("Wrong file extension ...");
		}
	}
	else if (argc == 3)
	{
		char cFileExtension[5];

		FileGetExtension(argv[2], cFileExtension, 5);

		printf("\nProcessing file: %s\n", argv[2]);

		if (!strcmp(argv[1], "patch") == true)
		{
			if (!strcmp(".vag", cFileExtension))
			{
				PatchVAG(argv[2]);
			}
			else if (!strcmp(".wav", cFileExtension))
			{
				uchar WAVType = CheckWAV(argv[2], false);

				// PS2 to Normal
				if (WAVType == WAV_PS2)
					PatchWAV(argv[2]);
				else if (WAVType == WAV_NORMAL || WAV_UNSUPPORTED)
					puts("File is already in normal format ...");
				else
					puts("Bad file ...");
			}
			else
			{
				puts("Wrong file extension ...");
			}
		}
		else if (!strcmp(argv[1], "unpatch") == true)
		{
			if (!strcmp(".vag", cFileExtension))
			{
				UnpatchVAG(argv[2]);
			}
			else if (!strcmp(".wav", cFileExtension))
			{
				uchar WAVType = CheckWAV(argv[2], false);

				// Normal to PS2
				if (WAVType == WAV_NORMAL || WAVType == WAV_UNSUPPORTED)
					UnpatchWAV(argv[2]);	
				else if (WAVType == WAV_PS2)
					puts("File is already in PS2 format ...");
				else
					puts("Bad file ...");
			}
			else
			{
				puts("Wrong file extension ...");
			}
		}
		else if (!strcmp(argv[1], "test") == true)
		{
			if (!strcmp(".vag", cFileExtension))
			{
				CheckVAG(argv[2], true);
			}
			else if (!strcmp(".wav", cFileExtension))
			{
				CheckWAV(argv[2], true);
			}
			else
			{
				puts("Wrong file extension ...");
			}
		}
		else
		{
			puts("Wrong arguments ...");
		}
	}
	else
	{
		puts("Can't recognise arguments ...");
	}
	
	return 0;
}
