#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>
#include "math.h"

#define OFF_X 1536
#define OFF_Y 1536
#define AMP_X 1024
#define AMP_Y 1024
#define FIX_TO_DAC(x) ((x) / (FIX_1 / AMP_X) + OFF_X)

/* Number of samples that shall be used to span a deflection of AMP_X */
#define DEFL_SPEED 150

typedef struct {
  unsigned i;
  uint32_t *xyBuf;
  unsigned xyBufSz;
} t_plot;

void plotCircle(t_plot *plot, t_fixp x0, t_fixp y0, t_fixp radius);
void plotLine(t_plot *plot, t_fixp x0, t_fixp y0, t_fixp x1, t_fixp y1, int lastDot);
void plotString(t_plot *plot, t_fixp x0, t_fixp y0, t_fixp scale, const char *str);
t_fixp sizeString(t_fixp scale, const char *str);

#endif
