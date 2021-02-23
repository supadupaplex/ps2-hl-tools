// Author:	supadupaplex
// License:	BSD-3-Clause (check out license.txt)

#ifndef UTIL_H
#define UTIL_H

//#define DEBUG // uncomment to see debug messages
// Print debug message to stdout (fmt, args...)
#ifdef DEBUG
	#define UTIL_MSG_DEBUG(...) { \
		fprintf(stdout, "DEBUG: <%s> ", __func__); \
		fprintf(stdout, __VA_ARGS__); \
	}
#else
	#define UTIL_MSG_DEBUG(...) {;}
#endif

// Common messages
#define MSG_ERR_ALLOC	"Unable to allocate memory!"

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

// Safe operation
#define UTIL_SAFE_OP(OP, FAIL_COND, FAIL_ACTION) { \
	{OP;} \
	if (FAIL_COND) \
		{FAIL_ACTION;} \
}

// Print message to stdout (fmt, args...)
#define UTIL_MSG(...) { \
	fprintf(stdout, __VA_ARGS__); \
}

// Print error message to stderr (fmt, args...)
#define UTIL_MSG_ERR(...) { \
	fprintf(stderr, "\nERROR: <%s> ", __func__); \
	fprintf(stderr, __VA_ARGS__); \
	fputc('\n', stderr); \
}

// Print msg to stderr and do action (return/exit)
#define UTIL_ERR(MSG, ACTION) { \
	UTIL_MSG_ERR(MSG); \
	{ACTION;} \
}

// Safe malloc/calloc ops
#define UTIL_MALLOC(TYPE, PTR, SZ, FAIL_ACTION) { \
	UTIL_SAFE_OP(PTR = (TYPE)malloc(SZ), !PTR, UTIL_ERR(MSG_ERR_ALLOC, FAIL_ACTION)); \
}
#define UTIL_CALLOC(TYPE, PTR, N, SZ, FAIL_ACTION) { \
	UTIL_SAFE_OP(PTR = (TYPE)calloc(N, SZ), !PTR, UTIL_ERR(MSG_ERR_ALLOC, FAIL_ACTION)); \
}

#endif // UTIL_H

