/** [UTF-8 encode and decode - Rosetta
 * Code](https://rosettacode.org/wiki/UTF-8_encode_and_decode#C) */
#ifndef UTF_H
#define UTF_H
#include <inttypes.h>
#include <stdio.h>

typedef struct {
	char mask; /* char data will be bitwise AND with this */
	char lead; /* start bytes of current char in utf-8 encoded character */
	uint32_t beg; /* beginning of codepoint range */
	uint32_t end; /* end of codepoint range */
	int bits_stored; /* the number of bits from the codepoint that fits in char */
} utf_t;
int to_utf8(const uint32_t cp, char ret[]);
#endif
