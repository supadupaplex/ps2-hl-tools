// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

//
// Zlib library is used within this module to perform DEFLATE\INFLATE operations
//
// This module can extract RAMFS contents from EE RAM dump or PCSX2 save state
//

////////// Includes //////////
#include "util.h"
#include "rfstool.h"

////////// Functions //////////
int ramfs_extract_raw(const char * fname);
int ramfs_extract_pcsx2(const char * fname);
bool ZDecompress(uchar * InputData, uint InputDataSize, uchar ** OutputData, uint * OutputDataSize, uint StartSize);		// Compress data with zlib

bool ZDecompress(uchar * InputData, uint InputDataSize, uchar ** OutputData, uint * OutputDataSize, uint StartSize)
{
	// Setting up zlib variables for decomression
	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;

	// Setting up other variables
	uchar * NewData;
	uint NewDataSize;
	bool Finish;

	// Set starting size of decompressed data (would be increased if bigger)
	NewDataSize = StartSize;

	// Decompression loop
	do
	{
		// Allocate memory for decompressed data
		UTIL_MALLOC(uchar*, NewData, NewDataSize, exit(EXIT_FAILURE));

		infstream.next_in = (Bytef *)InputData;			// Input data pointer (compressed data)
		infstream.avail_in = (uint)InputDataSize;		// Size of input data
		infstream.next_out = (Bytef *)NewData;			// Output data pointer (decompressed data)
		infstream.avail_out = (uint)NewDataSize;		// Size of output data

		// Decompression work
		if (inflateInit(&infstream) != Z_OK)
		{
			puts("Zlib: can't decompress data ...");
			return false;
		}
		inflate(&infstream, Z_FINISH);
		inflateEnd(&infstream);

		// if buffer is full then increase buffer size and retry decompression
		if (NewDataSize == infstream.total_out)
		{
			free(NewData);
			NewDataSize = NewDataSize * 2 + 1;		// +1 to avoid infinite loop, when StartSize = 0

			Finish = false;
		}
		else
		{
			Finish = true;
		}
	} while (Finish == false);

	// Check if output data has zero size
	if (infstream.total_out == 0)
		return false;

	// Return data pointer, data size and result
	*OutputData = NewData;
	*OutputDataSize = infstream.total_out;
	return true;
}

#define HL1_SIGN	0x564C4156 // LE 'VALV'
void check_hl1(const char *fname, uchar *fdata, uint fsz)
{
	char ext[5];
	bool is_ok;
	struct {
		uint sign;
		uint idk1;
		uint sz1;
		uint idk2[2];
		uint sz2;
	} header;

	// Check ext
	FileGetExtension(fname, ext, sizeof(ext));
	if (strcmp(ext, ".hl1"))
		return;

	// Check header
	memcpy(&header, fdata, sizeof(header));

	is_ok = false;
	if (header.sign == HL1_SIGN && fsz == (header.sz1 + header.sz2 + sizeof(header)))
		is_ok = true;

	printf(" Check HL1: %s\n", is_ok ? "OK" : "BAD");
}

int ramfs_extract_raw(const char * fname)
{
	sPS2_EERAM eeram;
	ps2_ramfs_entry_t *prf;

	int f, fcount;
	uchar *fdata, *extdata;
	uint fsz, extsz;

	char ofname[PATH_LEN];
	FILE *pof;

	if (eeram.open_from_file(fname))
	{
		puts("Bad EE RAM dump ...");
		return 1;
	}

	puts("Looking up files in RAMFS...");
	fcount = 0;
	for (f = 0; f < PS2_RAMFS_FILES; f++) {
		// Get file info
		prf = eeram.ramfs_file_get(f);
		if (!prf)
			continue;

		// Get file data ptr and size
		fsz = prf->sz[1] > prf->sz[2] ? prf->sz[1] : prf->sz[2];
		fdata = (eeram.mem + prf->pdata);

		// Determine a path for extracted file
		FileGetPath(fname, ofname, sizeof(ofname));
		strcat(ofname, prf->name + 1);
		PatchSlashes(ofname, strlen(ofname), true);

		printf("Extracting: %s, size %d b\n", ofname, fsz);

		// Write the output file
		GenerateFolders(ofname);
		SafeFileOpen(&pof, ofname, "wb");
		if (prf->sz[0] && fdata[4] == 0x78)
		{
			extsz = *((uint *)fdata) - 4;
			UTIL_MALLOC(uchar*, extdata, extsz, exit(EXIT_FAILURE));
			printf(" Decompressing: %d -> %d b\n", fsz, extsz);
			if (ZDecompress(fdata+4, fsz, &extdata, &extsz, extsz))
			{
				FileWriteBlock(&pof, extdata, extsz);
				check_hl1(prf->name, extdata, extsz);
			}
			else
			{
				puts(" Unable to decompress file!");
			}
			free(extdata);
		}
		else
		{
			FileWriteBlock(&pof, fdata, fsz);
			check_hl1(prf->name, fdata, fsz);
		}
		fclose(pof);

		fcount++;
	}

	printf("\nFound %d file(s)\n", fcount);
	puts("\nDone\n");

	eeram.close();
	return 0;
}

#define PCSX2_EERAM_LUMP	"eeMemory.bin"
int ramfs_extract_pcsx2(const char * fname)
{
	int off, csz, extsz;
	ztype t;
	FILE *pf, *pof; // input and output files
	uchar *cdata, *extdata;
	char ofname[256];

	if (!ZipLookupFile((char *)fname, PCSX2_EERAM_LUMP, &off, &csz, &extsz, &t) ||
		t == ZIP_BAD || extsz != PS2_EERAM_SZ)
	{
		puts("Bad PCSX2 save state...");
		return 1;
	}

	puts("Extracting EE RAM image...");

	// Read the data
	UTIL_MALLOC(uchar*, cdata, csz+2, exit(EXIT_FAILURE)); // +2 for 0x78 0xda
	UTIL_MALLOC(uchar*, extdata, extsz, exit(EXIT_FAILURE));
	SafeFileOpen(&pf, fname, "rb");
	FileReadBlock(&pf, &cdata[2], off, csz);
	fclose(pf);

	// Decompress the data
	if (t == ZIP_NONE && extsz == csz)
	{
		memcpy(extdata, cdata+2, csz);
	}
	else
	{
		cdata[0] = 0x78; // Add the missing deflate header
		cdata[1] = 0xda;
		uint uextsz = extsz;
		if (!ZDecompress(cdata, csz+2, &extdata, &uextsz, uextsz))
		{
			free(cdata);
			free(extdata);
			return 1;
		}
	}

	// Write desompressed file
	strcpy(ofname, fname);
	strcat(ofname, ".eeram.bin");
	SafeFileOpen(&pof, ofname, "wb");
	FileWriteBlock(&pof, extdata, 0, extsz);
	fclose(pof);

	// Free mem
	free(cdata);
	free(extdata);

	// Feed the decompressed file to ramfs_extract_raw()
	return ramfs_extract_raw(ofname);
}

int main(int argc, char * argv[])
{
	char ext[5];

	puts(PROG_TITLE);

	if (argc == 1)
	{
		puts(PROG_INFO);
		UTIL_WAIT_KEY("Press any key to exit ...");
	}
	else if (argc == 2)
	{
		printf("Processing file: %s \n\n", argv[1]);

		FileGetExtension(argv[1], ext, sizeof(ext));

		if (!strcmp(ext, ".p2s"))
		{
			return ramfs_extract_pcsx2(argv[1]);
		}
		else
		{
			// Try to inperpret as raw EE memory dump
			return ramfs_extract_raw(argv[1]);
		}
	}
	else
	{
		puts("Too many arguments ...");
	}

	return 1;
}
