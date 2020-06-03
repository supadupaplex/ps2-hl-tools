#ifndef PNGTOOL_H
#define PNGTOOL_H

// Data pointer + size
struct sPNGData
{
	ulong DataSize;
	uchar * Data;
};

// PNG types
#define PNG_UNKNOWN 0
#define PNG_RGB 2
#define PNG_INDEXED 3
#define PNG_RGBA 6

// PNG Functions
sPNGData * PNGReadChunk(FILE ** ptrFile, const char * Marker);												// Read data from all PNG chunks with specified marker
void PNGWriteChunk(FILE ** ptrFile, const char * Marker, sPNGData * Chunk);									// Write chunk to PNG
void PNGWriteChunk(FILE ** ptrFile, const char * Marker, const void * Data, ulong DataSize);				// Write chunk to PNG
bool PNGDecompress(sPNGData * InData);																		// Decompress bitmap
bool PNGCompress(sPNGData * InData);																		// Compress bitmap
uchar PNGGetByteFromRow(uchar * Row, uint PixelNumber, uint BitDepth);										// Get pixel byte from row
bool PNGUnfilter(sPNGData * InData, uint Height, uint Width, uint BytesPerPixel, uint BitDepth);			// Revert filtering from bitmap
bool PNGFilter(sPNGData * InData, uint Height, uint Width, uint BytesPerPixel, uchar FilterType);			// Apply filter to bitmap
int PaethPredictor(int a, int b, int c);																	// Paeth predictor function
sPNGData * PNGReadPalette(FILE ** ptrFile);																	// Read palette from PNG file
sPNGData * PNGReadBitmap(FILE ** ptrFile, uint Width, uint Height, uchar BytesPerPixel, uint BitDepth);		// Read raw bitmap from PNG file
void PNGWritePalette(FILE ** ptrFile, sPNGData * RGBAPalette);												// Write palette to PNG file
void PNGWriteBitmap(FILE ** ptrFile, uint Width, uint Height, uchar BytesPerPixel, sPNGData * RGBABitmap);	// Write bitmap to PNG file
bool ZDecompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize, ulong StartSize);	// Compress data with Zlib
bool ZCompress(uchar * InputData, ulong InputDataSize, uchar ** OutputData, ulong * OutputDataSize);					// Decompress data with Zlib

// *.png image header
#pragma pack(1)
struct sPNGHeader
{
	ulong Signature1;			// [0x89504E47]
	ulong Signature2;			// [0x0D0A1A0A]
	ulong IHDTSize;				// 13 [0x0D]
	ulong IHDT;					// "IHDT" [0x49484452]

	ulong Width;				// Image width (in pixels)
	ulong Height;				// Image height (in pixels)
	
	uchar BitDepth;				// = 8 (From Wiki: The permitted formats encode each number as an unsigned integral value using a fixed number of bits, referred to in the PNG specification as the bit depth.)
	uchar ColorType;			// 2 - TrueColor (RGB), 3 - Indexed, 6 - TrueColor (RGBA)
	uchar Compression;			// = 0 (No compression)
	uchar Filter;				// = 0 (Per-row filtering)
	uchar Interlacing;			// = 0 (No interlacing)
	ulong CRC32;				// Checksum of header

	void SwapEndian()			// Swap endian after reading or before writing to file
	{
		this->Signature1 = _byteswap_ulong(this->Signature1);
		this->Signature2 = _byteswap_ulong(this->Signature2);
		this->IHDTSize = _byteswap_ulong(this->IHDTSize);
		this->IHDT = _byteswap_ulong(this->IHDT);
		this->Width = _byteswap_ulong(this->Width);
		this->Height = _byteswap_ulong(this->Height);
		this->CRC32 = _byteswap_ulong(this->CRC32);
	}

	void UpdateFromFile(FILE ** ptrFile)
	{
		FileReadBlock(ptrFile, this, 0, sizeof(sPNGHeader));
	}

	void Update(ulong NewWidth, ulong NewHeight, ulong NewColorType)
	{
		ulong CRC;

		this->Signature1 = 0x89504E47;		// 
		this->Signature2 = 0x0D0A1A0A;		// 
		this->IHDTSize = 13;				// always = 13
		this->IHDT = 0x49484452;			// "IHDT"

		this->Width = NewWidth;				// Image width (in pixels)
		this->Height = NewHeight;			// Image height (in pixels)

		this->BitDepth = 8;					// 8 is common value
		this->ColorType = NewColorType;		// Supported types: 2 - TrueColor (RGB), 3 - Indexed, 6 - TrueColor (RGBA)
		this->Compression = 0;				// No compression
		this->Filter = 0;					// Per-row filtering
		this->Interlacing = 0;				// No interlacing

		// Calculate CRC
		this->SwapEndian();
		CRC = crc32(0L, (const Bytef *) &this->IHDT, sizeof(ulong) * 3 + sizeof(uchar) * 5);
		this->SwapEndian();
		this->CRC32 = CRC;
	}

	uchar CheckType()
	{
		if (this->Signature1 != 0x89504E47 || this->Signature2 != 0x0D0A1A0A || this->IHDTSize != 13 || this->IHDT != 0x49484452
			|| this->BitDepth > 8 || this->Compression != 0 || this->Filter != 0 || this->Interlacing != 0)
			return PNG_UNKNOWN;

		return this->ColorType;
	}
};

#endif