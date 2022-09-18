#ifndef MATH_H
#define MATH_H

#include <stdint.h>

#define BINANG_0      (uint32_t)(0)
#define BINANG_90     (uint32_t)(0x40000000)
#define BINANG_180    (uint32_t)(0x80000000)
#define TO_BINANG(x, xscale) ((uint32_t)(x) * (BINANG_180 / (((uint32_t)xscale) / 2)))

#define FIX_1         (int32_t)(0x10000)

int32_t sins(uint32_t binangle);
int32_t coss(uint32_t binangle);

#endif
