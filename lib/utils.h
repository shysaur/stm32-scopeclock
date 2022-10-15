#ifndef UTILS_H
#define UTILS_H

#include "math.h"
#include <stdbool.h>
#include <stdint.h>

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)

#define PLUS_NONE  0
#define PLUS_SPACE 1
#define PLUS_PLUS  2

/** Formats a signed integer.
 *  @param n    The integer to be formatted.
 *  @param base The numeric base for the output. Cannot be > 16.
 *  @param plus Determines if an additional prefix must be
 *     added for positive numbers. PLUS_NONE means no prefix,
 *     PLUS_SPACE means a space, PLUS_PLUS means the plus (+) character.
 *  @param uppercase Whether the output is uppercase. Useful only for
 *     bases greater than 10).
 *  @param minimumWidth The minimum amount of characters to use. If the
 *     number fits in less characters than specified, it is padded with
 *     leading zeroes.
 *  @param out  The string buffer where to output the formatted number.
 *  @returns The number of characters inserted in the buffer. */
int formatSignedInt(int32_t n, int base, int plus, bool uppercase, int minimumWidth, char *out);

/** Formats an unsigned integer.
 *  @param n    The integer to be formatted.
 *  @param base The numeric base for the output. Cannot be > 16.
 *  @param uppercase Whether the output is uppercase. Useful only for
 *     bases greater than 10).
 *  @param minimumWidth The minimum amount of characters to use. If the
 *     number fits in less characters than specified, it is padded with
 *     leading zeroes.
 *  @param out  The string buffer where to output the formatted number.
 *  @returns The number of characters inserted in the buffer. */
int formatUnsignedInt(uint32_t n, int base, bool uppercase, int minimumWidth, char *out);

/** Formats a 16/16 fixed point number.
 *  @param n    The fixed point number to be formatted.
 *  @param base The numeric base for the output. Cannot be > 16.
 *  @param plus Determines if an additional prefix must be
 *     added for positive numbers. PLUS_NONE means no prefix,
 *     PLUS_SPACE means a space, PLUS_PLUS means the plus (+) character.
 *  @param uppercase Whether the output is uppercase. Useful only for
 *     bases greater than 10).
 *  @param fracWidth The number of fractional digits to print. Cannot
 *     be greater than 4.
 *  @param minimumWidth The minimum amount of characters to use. If the
 *     number fits in less characters than specified, it is padded with
 *     leading zeroes.
 *  @param out  The string buffer where to output the formatted number.
 *  @returns The number of characters inserted in the buffer. */
int formatFixedPoint(t_fixp n, int base, int plus, bool uppercase, int fracWidth, int minimumWidth, char *out);

#endif
