#ifndef MATH_H
#define MATH_H

#include <stdint.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))

typedef uint32_t t_binang;
#define BINANG_0      (t_binang)(0)
#define BINANG_90     (t_binang)(0x40000000)
#define BINANG_180    (t_binang)(0x80000000)
#define TO_BINANG(x, xscale) ((t_binang)(x) * (BINANG_180 / (((t_binang)xscale) / 2)))

typedef int32_t t_fixp;
#define FIX_1         (t_fixp)(0x10000)
#define FIX_PI        (t_fixp)(0x3243f)
#define FIX_MUL(a, b) (t_fixp)(((int64_t)(a) * (int64_t)(b)) / (int64_t)FIX_1)

t_fixp sins(t_binang binangle);
t_fixp coss(t_binang binangle);

#endif
