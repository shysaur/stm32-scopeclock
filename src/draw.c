#include <stdint.h>
#include <stddef.h>
#include "draw.h"
#include "font_futural.h"

#define DRAWCMD_MOVETO          0x00
#define DRAWCMD_LINETO          0x01
#define DRAWCMD_LINETO_LASTDOT  0x02
#define DRAWCMD_CIRCLE          0x03
#define DRAWCMD_SET_STR_SCALE   0x04
#define DRAWCMD_INVOKE          0x05
#define DRAWCMD_SET_STR_FONT    0x06
#define DRAWCMD_CHAR_BEGIN      0x20
#define DRAWCMD_CHAR_END        0xFF

const t_plotFont * const fonts[] = {
  &futural_font
};


void plot_renderInit(t_plotRender *plot, uint32_t *xyBuf, unsigned xyBufSz)
{
  plot->i = 0;
  plot->xyBuf = xyBuf;
  plot->xyBufSz = xyBufSz;
}


static void _plot_renderCircle(t_plotRender *plot, t_dac x0, t_dac y0, t_fixp radius)
{
  t_fixp circ = 2 * FIX_MUL(FIX_PI, radius);
  unsigned numSa = (DEFL_RATE * circ) / FIX_1;
  for (unsigned i=0; i<numSa && plot->i < plot->xyBufSz; i++, plot->i++) {
    t_fixp x = FIX_MUL(sins(TO_BINANG(i, numSa)), radius);
    t_fixp y = FIX_MUL(coss(TO_BINANG(i, numSa)), radius);
    plot->xyBuf[plot->i] = (FIX_TO_DAC(x)+x0) | ((FIX_TO_DAC(y)+y0) << 16);
  }
}

void plot_renderCircle(t_plotRender *plot, t_fixp x0, t_fixp y0, t_fixp radius)
{
  _plot_renderCircle(plot, FIX_TO_DAC(x0), FIX_TO_DAC(y0), radius);
}


static void _plot_renderLine(t_plotRender *plot, t_dac px, t_dac py, t_dac px1, t_dac py1, int lastDot)
{
  if (px >= px1) {
    /* flip west quadrants to east */
    t_dac tmp = px;
    px = px1;
    px1 = tmp;
    tmp = py;
    py = py1;
    py1 = tmp;
  }

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
      emitAccum += DEFL_RATE;
      incAccum += dy;
      if (incAccum >= dx) {
        py += yinc;
        emitAccum += (DEFL_RATE * 4142 / 10000);
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
      emitAccum += DEFL_RATE;
      incAccum += dx;
      if (incAccum >= dy) {
        px++;
        emitAccum += (DEFL_RATE * 4142 / 10000);
        incAccum -= dy;
      }
    }
  }
}

void plot_renderLine(t_plotRender *plot, t_fixp x0, t_fixp y0, t_fixp x1, t_fixp y1, int lastDot)
{
  _plot_renderLine(plot, FIX_TO_DAC(x0), FIX_TO_DAC(y0), FIX_TO_DAC(x1), FIX_TO_DAC(y1), lastDot);
}


static t_dac _plot_renderChar(t_plotRender *plot, const t_plotFont *font, t_dac x0, t_dac y0, t_fixp scale, char c)
{
  if (c < 0)
    return x0;
  
  const uint8_t *points = font->vectors[c];
  if (!points)
    return x0;

  unsigned data_len = *points++;
  data_len += (*points++) << 8;
  data_len *= 2;

  t_dac height = scale * AMP_Y / FIX_1;
  t_dac width = scale * AMP_X / FIX_1;
  unsigned font_defl_rate = font->ascender * FIX_1 / scale;
  unsigned step = font_defl_rate * FIX_1 / DEFL_RATE;

  for (t_fixp i=0; plot->i < plot->xyBufSz; i+=step) {
    unsigned ii = i / FIX_1 * 2;
    if (ii >= data_len)
      break;
    t_dac x = (int8_t)points[ii];
    t_dac y = (int8_t)points[ii+1];
    x = (x * width / font->ascender) + x0;
    y = (y * height / font->ascender) + y0;
    plot->xyBuf[plot->i++] = (uint32_t)x | ((uint32_t)y << 16);
  }
  return font->advancements[c] * width / font->ascender + x0;
}

static unsigned _plot_renderString(t_plotRender *plot, const t_plotFont *font, t_dac x0, t_dac y0, t_fixp scale, const char *str)
{
  unsigned i;
  for (i = 0; str[i] != '\0'; i++) {
    char c = str[i];
    x0 = _plot_renderChar(plot, font, x0, y0, scale, c);
  }
  return i;
}

void plot_renderString(t_plotRender *plot, const t_plotFont *font, t_fixp x0, t_fixp y0, t_fixp scale, const char *str)
{
  _plot_renderString(plot, font, FIX_TO_DAC(x0), FIX_TO_DAC(y0), scale, str);
}

t_fixp plot_sizeString(t_plotFontID fontid, t_fixp scale, const char *str)
{
  const t_plotFont *font = fonts[fontid];
  scale /= font->ascender;
  t_fixp res = 0;
  for (const char *pi = str; *pi != '\0'; pi++) {
    if (*pi < 0)
      continue;
    res += scale * (t_fixp)font->advancements[*pi];
  }
  return res;
}


void plot_init(t_plot *ctx, uint8_t *cmdBuf, unsigned cmdBufSz)
{
  ctx->cmdBufWriteI = 0;
  ctx->cmdBufReadI = 0;
  ctx->cmdBuf = cmdBuf;
  ctx->cmdBufSz = cmdBufSz;
  ctx->curX = 0;
  ctx->curY = 0;
  ctx->stringScale = FIX_1 / 8;
  ctx->stringFont = fonts[PLOT_FONT_ID_FUTURAL];
  ctx->next = NULL;
}

int plot_moveTo(t_plot *ctx, t_fixp x, t_fixp y)
{
  if (ctx->cmdBufWriteI+4 > ctx->cmdBufSz)
    return 0;
  uint32_t param = FIX_TO_DAC(x) | (FIX_TO_DAC(y) << 12);
  ctx->cmdBuf[ctx->cmdBufWriteI++] = DRAWCMD_MOVETO;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = param & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (param >> 8) & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (param >> 16) & 0xFF;
  return 1;
}

int plot_lineTo(t_plot *ctx, t_fixp x, t_fixp y, int lastDot)
{
  if (ctx->cmdBufWriteI+4 > ctx->cmdBufSz)
    return 0;
  uint32_t param = FIX_TO_DAC(x) | (FIX_TO_DAC(y) << 12);
  if (lastDot)
    ctx->cmdBuf[ctx->cmdBufWriteI++] = DRAWCMD_LINETO_LASTDOT;
  else
    ctx->cmdBuf[ctx->cmdBufWriteI++] = DRAWCMD_LINETO;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = param & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (param >> 8) & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (param >> 16) & 0xFF;
  return 1;
}

int plot_circle(t_plot *ctx, t_fixp radius)
{
  if (ctx->cmdBufWriteI+4 > ctx->cmdBufSz)
    return 0;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = DRAWCMD_CIRCLE;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = radius & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (radius >> 8) & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (radius >> 16) & 0xFF;
  return 1;
}

int plot_selectFont(t_plot *ctx, t_plotFontID fontid, t_fixp scale)
{
  if (ctx->cmdBufWriteI+2+4 > ctx->cmdBufSz)
    return 0;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = DRAWCMD_SET_STR_FONT;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = fontid;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = DRAWCMD_SET_STR_SCALE;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = scale & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (scale >> 8) & 0xFF;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = (scale >> 16) & 0xFF;
  return 1;
}

int plot_putString(t_plot *ctx, const char *str)
{
  unsigned i = ctx->cmdBufWriteI;
  while (*str != '\0' && i < ctx->cmdBufSz) {
    ctx->cmdBuf[i++] = *str++;
  }
  if (i >= ctx->cmdBufSz)
    return 0;
  ctx->cmdBufWriteI = i;
  return 1;
}

int plot_invoke(t_plot *ctx, t_plot *other)
{
  if (ctx->cmdBufWriteI+1+sizeof(uintptr_t) > ctx->cmdBufSz)
    return 0;
  ctx->cmdBuf[ctx->cmdBufWriteI++] = DRAWCMD_INVOKE;
  uintptr_t p = (uintptr_t)other;
  for (int i=0; i<sizeof(uintptr_t); i++) {
    ctx->cmdBuf[ctx->cmdBufWriteI++] = p & 0xFF;
    p = p >> 8;
  }
  return 1;
}

int plot_render(t_plot *ctx, t_plotRender *dest)
{
  if (ctx->cmdBufReadI == 0) {
    /* New fresh render */
    ctx->curX = 0;
    ctx->curY = 0;
  } else if (ctx->next) {
    /* Old render suspended in a subplot */
    int res = plot_render(ctx->next, dest);
    if (!res)
      return 0;
    ctx->next = NULL;
  }

  unsigned oldPlotI = dest->i;
  unsigned oldCmdBufReadI = ctx->cmdBufReadI;
  t_dac newX = ctx->curX;
  t_dac newY = ctx->curY;
  while (ctx->cmdBufReadI < ctx->cmdBufWriteI && dest->i < dest->xyBufSz) {
    oldPlotI = dest->i;
    oldCmdBufReadI = ctx->cmdBufReadI;
    ctx->curX = newX;
    ctx->curY = newY;
    uint8_t cmd = ctx->cmdBuf[ctx->cmdBufReadI++];
    uint32_t tmp;
    uintptr_t ptr;
    if (DRAWCMD_CHAR_BEGIN <= cmd && cmd <= DRAWCMD_CHAR_END) {
      newX = _plot_renderChar(dest, ctx->stringFont, ctx->curX, ctx->curY, ctx->stringScale, cmd);
    } else {
      switch (cmd) {
        case DRAWCMD_MOVETO:
        case DRAWCMD_LINETO:
        case DRAWCMD_LINETO_LASTDOT:
          tmp  = ctx->cmdBuf[ctx->cmdBufReadI++];
          tmp |= ctx->cmdBuf[ctx->cmdBufReadI++] << 8;
          tmp |= ctx->cmdBuf[ctx->cmdBufReadI++] << 16;
          newX = tmp & 0xFFF;
          newY = (tmp >> 12) & 0xFFF;
          if (cmd == DRAWCMD_LINETO)
            _plot_renderLine(dest, ctx->curX, ctx->curY, newX, newY, 0);
          else if (cmd == DRAWCMD_LINETO_LASTDOT)
            _plot_renderLine(dest, ctx->curX, ctx->curY, newX, newY, 1);
          break;
        case DRAWCMD_CIRCLE:
          tmp  = ctx->cmdBuf[ctx->cmdBufReadI++];
          tmp |= ctx->cmdBuf[ctx->cmdBufReadI++] << 8;
          tmp |= ctx->cmdBuf[ctx->cmdBufReadI++] << 16;
          _plot_renderCircle(dest, ctx->curX, ctx->curY, tmp);
          break;
        case DRAWCMD_SET_STR_FONT:
          ctx->stringFont = fonts[ctx->cmdBuf[ctx->cmdBufReadI++]];
          break;
        case DRAWCMD_SET_STR_SCALE:
          tmp  = ctx->cmdBuf[ctx->cmdBufReadI++];
          tmp |= ctx->cmdBuf[ctx->cmdBufReadI++] << 8;
          tmp |= ctx->cmdBuf[ctx->cmdBufReadI++] << 16;
          ctx->stringScale = tmp;
          break;
        case DRAWCMD_INVOKE:
          ptr = 0;
          for (int i=0; i<sizeof(uintptr_t); i++)
            ptr |= (uintptr_t)(ctx->cmdBuf[ctx->cmdBufReadI++]) << ((uintptr_t)(i*8));
          ctx->next = (t_plot *)((void *)ptr);
          tmp = plot_render(ctx->next, dest);
          if (!tmp)
            return 0;
          ctx->next = NULL;
      }
    }
  }

  int finished = 0;
  if (dest->i >= dest->xyBufSz) {
    if (oldPlotI > 0) {
      /* More than one command was completely plotted.
       * Rollback to the last command successfully plotted
       * in its entirety. */
      dest->i = oldPlotI;
      ctx->cmdBufReadI = oldCmdBufReadI;
    } else {
      /* Only one command was plotted and it was incomplete.
       * This command will never fit into the buffer,
       * do not rollback to avoid locking up here. */
    }
  }
  if (ctx->cmdBufReadI >= ctx->cmdBufWriteI) {
    ctx->cmdBufReadI = ctx->cmdBufWriteI = 0;
    finished = 1;
  }
  return finished;
}

