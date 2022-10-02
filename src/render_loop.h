#ifndef RENDER_LOOP_H
#define RENDER_LOOP_H

#include "draw.h"

/* Uses DAC, DMA1/3, TIM6 */

typedef void t_plot_func(t_plot *plot);

void rl_init(t_plot_func *plotUpdFunc);
void rl_setPlotUpdateFunc(t_plot_func *plotUpdFunc);
void rl_update(void);

#endif
