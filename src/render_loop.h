#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H

#include "draw.h"
#include <stdbool.h>
#include <stdint.h>

typedef void t_plot_func(t_plot *plot);

void rl_init(t_plot_func *plotUpdFunc);
void rl_setPlotUpdateFunc(t_plot_func *plotUpdFunc);
void rl_setEnableFPSDisplay(bool enable);
void rl_update(void);
uint32_t rl_getMsTime(void);

#endif
