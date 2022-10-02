#ifndef UTILS_H
#define UTILS_H

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)

#endif
