#include <stdio.h>
#include <stdlib.h>
#include "../src/math.c"
#include "../src/font_futural.c"
#include "../src/draw.c"

#define GET_X(n) ((n) & 0xFFF)
#define GET_Y(n) (((n) >> 16) & 0xFFF)


int main(int argc, char *argv[])
{
  #define RENDER_BUF_SIZE 800
  uint32_t buf[RENDER_BUF_SIZE];
  uint8_t cmdBuf1[1024];
  uint8_t cmdBuf2[512];
  t_plotRender plot;
  t_plot ctxDial, ctx;

  plot_renderInit(&plot, buf, RENDER_BUF_SIZE);
  plot_init(&ctx, cmdBuf2, 512);
  plot_init(&ctxDial, cmdBuf1, 1024);

  t_fixp text_height = FIX_1 * 2/16;
  t_plotFontID text_font = PLOT_FONT_ID_FUTURAL;
  plot_selectFont(&ctxDial, text_font, text_height);
  for (int i=0; i<60; i++) {
    t_binang ang = i * (BINANG_180 / 60) * 2;
    t_fixp x = sins(ang);
    t_fixp y = coss(ang);
    t_fixp x0, y0, x1, y1;

    if (i % 5 == 0) {
      x0 = x * 14 / 16;
      y0 = y * 14 / 16;
    } else {
      x0 = x * 15 / 16;
      y0 = y * 15 / 16;
    }
    x1 = x;
    y1 = y;
    plot_moveTo(&ctxDial, x0, y0),
    plot_lineTo(&ctxDial, x1, y1, 1);

    if (i % 5 == 0) {
      char buf[10];
      sprintf(buf, "%d", i / 5 ?: 12);
      x0 = x * 12 / 16;
      y0 = y * 12 / 16 - text_height/2;
      t_fixp width = plot_sizeString(text_font, text_height, buf);
      x0 -= width / 2;
      plot_moveTo(&ctxDial, x0, y0);
      plot_putString(&ctxDial, buf);
    }
  }

  plot_invoke(&ctx, &ctxDial);
  plot_moveTo(&ctx, 0, 0);
  plot_lineTo(&ctx, sins(0) * 10/16, coss(0) * 10/16, 1);
  plot_moveTo(&ctx, 0, 0);
  plot_lineTo(&ctx, sins(BINANG_90) * 10/16, coss(BINANG_90) * 10/16, 1);
  plot_moveTo(&ctx, 0, 0);
  plot_lineTo(&ctx, sins(BINANG_180 + BINANG_90/2) * 10/16, coss(BINANG_180 + BINANG_90/2) * 10/16, 1);

  plot_moveTo(&ctx, -FIX_1/2, -FIX_1/2);
  plot_selectFont(&ctx, text_font, FIX_1/8);
  plot_putString(&ctx, "Scope Clock!");

  int stop;
  do {
    stop = plot_render(&ctx, &plot);
    fprintf(stderr, "used %d/%d, stop=%d\n", plot.i, plot.xyBufSz, stop);
    if (plot.i == 0) {
      printf("error: plotted zero!\n");
      return 1;
    }
    for (int i=0; i<plot.i; i++) {
      printf("%d\t%d\n", GET_X(plot.xyBuf[i]), GET_Y(plot.xyBuf[i]));
    }
    plot.i = 0;
  } while (!stop);
}

