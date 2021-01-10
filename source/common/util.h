// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

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
	#define UTIL_WAIT_KEY(MSG)
#else
	#define UTIL_WAIT_KEY(MSG)	{ puts(MSG); fflush(stderr); fflush(stdout); getc(stdin); }
#endif

#endif // UTIL_H

