#include <stdio.h>
#include <stdlib.h>
#include "../src/math.c"
#include "../src/font.c"
#include "../src/draw.c"

#define GET_X(n) ((n) & 0xFFF)
#define GET_Y(n) (((n) >> 16) & 0xFFF)


int main(int argc, char *argv[])
{
  uint32_t buf[256];
  uint8_t cmdBuf1[1024];
  uint8_t cmdBuf2[512];
  t_plotRender plot;
  t_plot ctxDial, ctx;

  plot_renderInit(&plot, buf, 256);
  plot_init(&ctx, cmdBuf2, 512);
  plot_init(&ctxDial, cmdBuf1, 1024);

/*
  for (int i=0; i<36; i++) {
    t_binang ang = i * (BINANG_180 / 36) * 2;
    t_fixp x0 = 0;
    t_fixp y0 = 0;
    t_fixp x1 = sins(ang);
    t_fixp y1 = coss(ang);
    plot_renderLine(&plot, x0, y0, x1, y1, 1);
  }
  plot_renderString(&plot, -5000, 5000, 300, "123 Hello yg!");
  plot_renderCircle(&plot, -1000, -10000, 10000);
*/
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
    //plot_renderLine(&plot, x0, y0, x1, y1, 1);

    if (i % 5 == 0) {
      char buf[10];
      sprintf(buf, "%d", i / 5 ?: 12);
      t_fixp height = FIX_1 * 2/16;
      x0 = x * 12 / 16;
      y0 = y * 12 / 16 - height/2;
      t_fixp width = plot_sizeString(height, buf);
      x0 -= width / 2;
      plot_moveTo(&ctxDial, x0, y0);
      plot_putString(&ctxDial, height, buf);
      //plot_renderString(&plot, x0, y0, height, buf);
    }
  }

  plot_invoke(&ctx, &ctxDial);
  plot_moveTo(&ctx, 0, 0);
  plot_lineTo(&ctx, sins(0) * 10/16, coss(0) * 10/16, 1);
  plot_moveTo(&ctx, 0, 0);
  plot_lineTo(&ctx, sins(BINANG_90) * 10/16, coss(BINANG_90) * 10/16, 1);
  plot_moveTo(&ctx, 0, 0);
  plot_lineTo(&ctx, sins(BINANG_180 + BINANG_90/2) * 10/16, coss(BINANG_180 + BINANG_90/2) * 10/16, 1);
  //plot_renderLine(&plot, 0, 0, sins(0) * 10/16, coss(0) * 10/16, 1);
  //plot_renderLine(&plot, 0, 0, sins(BINANG_90) * 6/16, coss(BINANG_90) * 6/16, 1);
  //plot_renderLine(&plot, 0, 0, sins(BINANG_180 + BINANG_90/2) * 11/16, coss(BINANG_180 + BINANG_90/2) * 11/16, 1);

  plot_moveTo(&ctx, -FIX_1/2, -FIX_1/2);
  plot_putString(&ctx, FIX_1/8, "Scope Clock!");
/*
  plot_renderString(&plot, -5000, 5000, 300, "123 Hello yg!");
  plot_renderCircle(&plot, -1000, -10000, 10000);
*/

  int stop;
  do {
    stop = plot_render(&ctx, &plot);
    fprintf(stderr, "used %d/%d, stop=%d\n", plot.i, plot.xyBufSz, stop);
    for (int i=0; i<plot.i; i++) {
      printf("%d\t%d\n", GET_X(plot.xyBuf[i]), GET_Y(plot.xyBuf[i]));
    }
    plot.i = 0;
  } while (!stop);
}

