#include "math.h"
#include "draw.h"
#include "utils.h"
#include "dial_pic.h"
#include "render_loop.h"
#include "version.h"
#include <stdint.h>
#include <stdbool.h>


void plotClock(t_plot *plot)
{
  t_binang angle;
  uint32_t ms_timestamp = rl_getMsTime();

  initDialPlot();
  plot_invoke(plot, &dial_plot);

  uint32_t h = ms_timestamp % (12 * 60 * 60 * 1000) / (12 * 60 * 60);
  angle = BINANG_90 - TO_BINANG(h, 1000);
  plot_moveTo(plot, 0, 0);
  plot_lineTo(plot, coss(angle) * 6/16, sins(angle) * 6/16, 1);

  uint32_t m = ms_timestamp % (60 * 60 * 1000) / (60 * 60);
  angle = BINANG_90 - TO_BINANG(m, 1000);
  plot_moveTo(plot, 0, 0);
  plot_lineTo(plot, coss(angle) * 10/16, sins(angle) * 10/16, 1);

  uint32_t s = ms_timestamp % (60 * 1000) / 60;
  angle = BINANG_90 - TO_BINANG(s, 1000);
  plot_moveTo(plot, 0, 0);
  plot_lineTo(plot, coss(angle) * 11/16, sins(angle) * 11/16, 1);

  static const t_fixp time_size = FIX_1/7;
  t_fixp width = plot_sizeString(PLOT_FONT_ID_FUTURAM, time_size, "00:00:00");
  plot_selectFont(plot, PLOT_FONT_ID_FUTURAM, time_size);
  plot_moveTo(plot, -width/2, -FIX_1*1/3);
  char str[4];
  str[2] = ':';
  str[3] = '\0';
  int tt;
  tt = (ms_timestamp / (1000 * 60 * 60)) % 24;
  str[1] = tt % 10 + '0';
  str[0] = tt / 10 + '0';
  plot_putString(plot, str);
  tt = (ms_timestamp / (1000 * 60)) % 60;
  str[1] = tt % 10 + '0';
  str[0] = tt / 10 + '0';
  plot_putString(plot, str);
  tt = (ms_timestamp / 1000) % 60;
  str[2] = '\0';
  str[1] = tt % 10 + '0';
  str[0] = tt / 10 + '0';
  plot_putString(plot, str);
}


void plotCalibBox(t_plot *plot)
{
  plot_moveTo(plot, -FIX_1, FIX_1);
  plot_lineTo(plot, FIX_1, FIX_1, true);
  plot_lineTo(plot, FIX_1, -FIX_1, true);
  plot_lineTo(plot, -FIX_1, -FIX_1, true);
  plot_lineTo(plot, -FIX_1, FIX_1, true);
  plot_lineTo(plot, FIX_1, -FIX_1, true);
  plot_moveTo(plot, FIX_1, FIX_1);
  plot_lineTo(plot, -FIX_1, -FIX_1, true);

  plot_selectFont(plot, PLOT_FONT_ID_FUTURAL, FIX_1/16);
  plot_moveTo(plot, -FIX_1*15/16, -FIX_1*13/16);
  plot_putString(plot, "CALIBRATION");
  plot_moveTo(plot, -FIX_1*15/16, -FIX_1*15/16);
  plot_putString(plot, "Version ");
  plot_putString(plot, version_tag);

  if (rl_getMsTime() >= 5000)
    rl_setPlotUpdateFunc(plotClock);
}


void main(void)
{
  rl_init(plotCalibBox);
  
  for (;;) {
    //__WFI();
    rl_update();
  }
}
