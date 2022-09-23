#include <stdio.h>
#include <stdlib.h>
#include "../src/math.c"
#include "../src/font.c"
#include "../src/draw.c"


int main(int argc, char *argv[])
{
  uint8_t cmdBuf1[1024];
  t_plot ctxDial;

  plot_init(&ctxDial, cmdBuf1, 1024);

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
      t_fixp height = FIX_1 * 2/16;
      x0 = x * 12 / 16;
      y0 = y * 12 / 16 - height/2;
      t_fixp width = plot_sizeString(height, buf);
      x0 -= width / 2;
      plot_moveTo(&ctxDial, x0, y0);
      plot_putString(&ctxDial, height, buf);
    }
  }

  printf("#define DIAL_FACE_CMDBUF_SIZE %d\n", ctxDial.cmdBufWriteI);
  printf("const uint8_t dial_face_cmdbuf[DIAL_FACE_CMDBUF_SIZE] = {");
  for (int i=0; i<ctxDial.cmdBufWriteI; i++) {
    if (i % 8 == 0) {
      if (i != 0)
        putchar(',');
      printf("\n  ");
    } else {
      printf(", ");
    }
    printf("0x%02x", cmdBuf1[i]);
  }
  printf("\n};\n");
  return 0;
}

