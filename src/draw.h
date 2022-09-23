#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>
#include "math.h"

typedef int32_t t_dac;

#define OFF_X ((t_dac)1536)
#define OFF_Y ((t_dac)1536)
#define AMP_X ((t_dac)1024)
#define AMP_Y ((t_dac)1024)
#define FIX_TO_DAC(x) ((t_dac)((x) / (FIX_1 / AMP_X) + OFF_X))

/* Number of samples that shall be used to span a deflection of AMP_X */
#define DEFL_SPEED 100

typedef struct {
  unsigned i;
  uint32_t *xyBuf;
  unsigned xyBufSz;
} t_plotRender;

typedef struct t_plot {
  unsigned cmdBufWriteI;
  unsigned cmdBufReadI;
  uint8_t *cmdBuf;
  unsigned cmdBufSz;
  t_dac curX, curY;
  struct t_plot *next;
} t_plot;


void plot_renderInit(t_plotRender *plot, uint32_t *xyBuf, unsigned xyBufSz);
void plot_renderCircle(t_plotRender *plot, t_fixp x0, t_fixp y0, t_fixp radius);
void plot_renderLine(t_plotRender *plot, t_fixp x0, t_fixp y0, t_fixp x1, t_fixp y1, int lastDot);
void plot_renderString(t_plotRender *plot, t_fixp x0, t_fixp y0, t_fixp scale, const char *str);

void plot_init(t_plot *ctx, uint8_t *cmdBuf, unsigned cmdBufSz);
int plot_moveTo(t_plot *ctx, t_fixp x, t_fixp y);
int plot_lineTo(t_plot *ctx, t_fixp x, t_fixp y, int lastDot);
int plot_circle(t_plot *ctx, t_fixp radius);
int plot_putString(t_plot *ctx, t_fixp scale, const char *str);
int plot_invoke(t_plot *ctx, t_plot *other);
int plot_render(t_plot *ctx, t_plotRender *dest);

t_fixp plot_sizeString(t_fixp scale, const char *str);

#endif
