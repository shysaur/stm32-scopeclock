#include "stm32f1xx.h"
#include "math.h"
#include "draw.h"
#include "utils.h"
#include "dial_pic.h"
#include "render_loop.h"
#include "version.h"
#include <stdint.h>
#include <stdbool.h>

volatile uint32_t ms_counter = 0;


void configPLLClock(void)
{
  /* SYSCLK to 24MHz with 8MHz ext. crystal */
  SET_BITS(RCC->CR, RCC_CR_HSEON, RCC_CR_HSEON_Msk);
  RCC->CFGR2 = 0;
  SET_BITS(RCC->CFGR,
      RCC_CFGR_PLLSRC | (1 << RCC_CFGR_PLLMULL_Pos),
      RCC_CFGR_PLLSRC_Msk | RCC_CFGR_PLLMULL_Msk);
  SET_BITS(RCC->CR, RCC_CR_PLLON, RCC_CR_PLLON_Msk);
  SET_BITS(RCC->CFGR, 2 << RCC_CFGR_SW_Pos, RCC_CFGR_SW_Msk);
}


void plotClock(t_plot *plot)
{
  t_binang angle;
  uint32_t ms_timestamp = ms_counter;

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
  t_fixp width = plot_sizeString(PLOT_FONT_ID_FUTURAL, time_size, "00:00:00");
  plot_selectFont(plot, PLOT_FONT_ID_FUTURAL, time_size);
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

  if (ms_counter >= 5000)
    rl_setPlotUpdateFunc(plotClock);
}


void SysTick_Handler(void)
{
  ms_counter++;
  if (ms_counter % 500 == 0) {
    uint32_t n = (ms_counter / 500) & 1;
    SET_BITS(GPIOC->ODR, n << 9, 1 << 9);
  }
}


void main(void)
{
  configPLLClock();
  SysTick_Config(24 * 1000);

  /* Configure LED GPIOs */
  SET_BITS(RCC->APB2ENR, RCC_APB2ENR_IOPCEN, RCC_APB2ENR_IOPCEN_Msk);
  SET_BITS(GPIOC->CRH, 
    (0 << GPIO_CRH_CNF8_Pos) | (3 << GPIO_CRH_MODE8_Pos) |
    (0 << GPIO_CRH_CNF9_Pos) | (3 << GPIO_CRH_MODE9_Pos), 
    GPIO_CRH_CNF8_Msk | GPIO_CRH_MODE8_Msk |
    GPIO_CRH_CNF9_Msk | GPIO_CRH_MODE9_Msk);

  rl_init(plotCalibBox);
  
  for (;;) {
    //__WFI();
    rl_update();
  }
}
