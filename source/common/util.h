// Author:	supadupaplex
// License:	check out readme.txt and license.txt

#ifndef UTIL_H
#define UTIL_H

// Endian swap
#ifdef _WIN32
	#include <intrin.h>
	#define UTIL_BSWAP32(VAL)	_byteswap_ulong(VAL)
#else
	#include <byteswap.h>
	#define UTIL_BSWAP32(VAL)	__bswap_32(VAL)
#endif

// Wait for key
//#define NO_WAIT
#ifdef NO_WAIT
	#define UTIL_WAIT_KEY
#else
	#define UTIL_WAIT_KEY	{ fflush(stdout); getc(stdin); }
#endif

#endif // UTIL_H

