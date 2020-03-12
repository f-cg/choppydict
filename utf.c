#include "utf.h"
#include <stdlib.h>
utf_t *utf[] = {
    /*             mask        lead        beg      end       bits */
    [0] = &(utf_t){0b00111111, 0b10000000, 0, 0, 6},
    [1] = &(utf_t){0b01111111, 0b00000000, 0000, 0177, 7},
    [2] = &(utf_t){0b00011111, 0b11000000, 0200, 03777, 5},
    [3] = &(utf_t){0b00001111, 0b11100000, 04000, 0177777, 4},
    [4] = &(utf_t){0b00000111, 0b11110000, 0200000, 04177777, 3},
    &(utf_t){0},
};

/* All lengths are in bytes */
static int codepoint_len(const uint32_t cp); /* len of associated utf-8 char */

int codepoint_len(const uint32_t cp) {
  int len = 0;
  for (utf_t **u = utf; *u; ++u) {
    if ((cp >= (*u)->beg) && (cp <= (*u)->end)) {
      break;
    }
    ++len;
  }
  if (len > 4) /* Out of bounds */
    exit(1);

  return len;
}

int to_utf8(const uint32_t cp, char ret[]) {
  const int bytes = codepoint_len(cp);

  int shift = utf[0]->bits_stored * (bytes - 1);
  ret[0] = (cp >> shift & utf[bytes]->mask) | utf[bytes]->lead;
  shift -= utf[0]->bits_stored;
  for (int i = 1; i < bytes; ++i) {
    ret[i] = (cp >> shift & utf[0]->mask) | utf[0]->lead;
    shift -= utf[0]->bits_stored;
  }
  ret[bytes] = '\0';
  return bytes;
}
/**  */
/** int main(void) { */
/**   const uint32_t *in, input[] = {0x0041, 0x00f6, 0x0416, 0x20ac, 0x1d11e,
 * 0x0}; */
/**  */
/**   printf("Character  Unicode  UTF-8 encoding (hex)\n"); */
/**   printf("----------------------------------------\n"); */
/**  */
/**   char *utf8; */
/**   for (in = input; *in; ++in) { */
/**     utf8 = to_utf8(*in); */
/**     printf("%s\n", utf8); */
/**   } */
/**   return 0; */
/** } */
