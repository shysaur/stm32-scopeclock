#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)

#define PLUS_NONE  0
#define PLUS_SPACE 1
#define PLUS_PLUS  2
int formatSignedInt(int32_t n, int base, int plus, bool uppercase, int minimumWidth, char *out);
int formatUnsignedInt(uint32_t n, int base, bool uppercase, int minimumWidth, char *out);

#endif
