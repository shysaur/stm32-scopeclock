#include "draw.h"
#include "font.h"


void plotCircle(t_plot *plot, t_fixp x0, t_fixp y0, t_fixp radius)
{
  t_fixp circ = 2 * FIX_MUL(FIX_PI, radius);
  unsigned numSa = (DEFL_SPEED * circ) / FIX_1;
  for (unsigned i=0; i<numSa && plot->i < plot->xyBufSz; i++, plot->i++) {
    t_fixp x = FIX_MUL(sins(TO_BINANG(i, numSa)), radius) + x0;
    t_fixp y = FIX_MUL(coss(TO_BINANG(i, numSa)), radius) + y0;
    plot->xyBuf[plot->i] = FIX_TO_DAC(x) | (FIX_TO_DAC(y) << 16);
  }
}


void plotLine(t_plot *plot, t_fixp x0, t_fixp y0, t_fixp x1, t_fixp y1, int lastDot)
{
  if (x0 >= x1) {
    /* flip west quadrants to east */
    t_fixp tmp = x0;
    x0 = x1;
    x1 = tmp;
    tmp = y0;
    y0 = y1;
    y1 = tmp;
  }

  int32_t px = FIX_TO_DAC(x0);
  int32_t py = FIX_TO_DAC(y0);
  int32_t px1 = FIX_TO_DAC(x1);
  int32_t py1 = FIX_TO_DAC(y1);

  int32_t dx = px1 - px;
  int32_t dy = py1 - py;
  int32_t yinc = 1;
  if (dy < 0) {
    /* slope > 0 */
    dy = -dy;
    yinc = -1;
  }

  if (dx > dy) {
    /* slope > 1 */
    int32_t incAccum = 0;
    int32_t emitAccum = AMP_X;
    for (int32_t i=0; i<dx && plot->i < plot->xyBufSz; i++) {
      if (emitAccum >= AMP_X || (i == (dx-1) && lastDot)) {
        plot->xyBuf[plot->i++] = (uint32_t)px | ((uint32_t)py << 16);
        emitAccum -= AMP_X;
      }
      px++;
      emitAccum += DEFL_SPEED;
      incAccum += dy;
      if (incAccum >= dx) {
        py += yinc;
        emitAccum += (DEFL_SPEED * 4142 / 10000);
        incAccum -= dx;
      }
    }
  } else {
    /* slope <= 1 */
    int32_t incAccum = 0;
    int32_t emitAccum = AMP_Y;
    for (int32_t i=0; i<dy && plot->i < plot->xyBufSz; i++) {
      if (emitAccum >= AMP_Y || (i == (dy-1) && lastDot)) {
        plot->xyBuf[plot->i++] = (uint32_t)px | ((uint32_t)py << 16);
        emitAccum -= AMP_Y;
      }
      py += yinc;
      emitAccum += DEFL_SPEED;
      incAccum += dx;
      if (incAccum >= dy) {
        px++;
        emitAccum += (DEFL_SPEED * 4142 / 10000);
        incAccum -= dy;
      }
    }
  }
}


void plotString(t_plot *plot, t_fixp x0, t_fixp y0, t_fixp scale, const char *str)
{
  scale /= GLYPH_MAX_HEIGHT;
  for (const char *pi = str; *pi != '\0'; pi++) {
    if (*pi < 0)
      continue;
    const uint8_t *cmds = glyph_vectors[*pi];
    if (!cmds)
      continue;
    t_fixp x = x0, y = y0, x1, y1;
    for (int stop = 0; stop == 0;) {
      uint8_t cmd = *cmds++;
      switch (cmd) {
        case 'M':
          x = scale * (t_fixp)((int8_t)(*cmds++)) + x0;
          y = scale * (t_fixp)((int8_t)(*cmds++)) + y0;
          break;
        case 'L':
          x1 = scale * (t_fixp)((int8_t)(*cmds++)) + x0;
          y1 = scale * (t_fixp)((int8_t)(*cmds++)) + y0;
          plotLine(plot, x, y, x1, y1, 0);
          x = x1;
          y = y1;
          break;
        case 'E':
          stop = 1;
          break;
      }
    }
    x0 += scale * (t_fixp)glyph_width[*pi];
  }
}

t_fixp sizeString(t_fixp scale, const char *str)
{
  scale /= GLYPH_MAX_HEIGHT;
  t_fixp res = 0;
  for (const char *pi = str; *pi != '\0'; pi++) {
    if (*pi < 0)
      continue;
    res += scale * (t_fixp)glyph_width[*pi];
  }
  return res;
}

