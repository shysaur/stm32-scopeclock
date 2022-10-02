#include "render_loop.h"
#include "utils.h"
#include "stm32f1xx.h"
#include <stdbool.h>

#define DAC_BUFFER_SZ 900
uint32_t dac_buffer[2][DAC_BUFFER_SZ];
unsigned dac_buffer_i;
uint32_t dac_buffer_fill[2];

#define PLOT_BUFFER_SZ 256
uint8_t plot_buffer[PLOT_BUFFER_SZ];
t_plot plot;

typedef void t_plot_func(t_plot *plot);
t_plot_func *plot_update_func;

volatile bool dac_finished_flag;

extern volatile uint32_t ms_counter;
uint32_t frame_time_ms = 0;
uint32_t update_time_ms = 0;
bool show_fps_disp = false;


void rl_setPlotUpdateFunc(t_plot_func *plotUpdFunc)
{
  plot_update_func = plotUpdFunc;
}

void rl_setEnableFPSDisplay(bool enable)
{
  show_fps_disp = enable;
}


static void configDAC(void)
{
  dac_finished_flag = false;

  /* Configure DAC */
  DAC1->SR = DAC_SR_DMAUDR1 | DAC_SR_DMAUDR2;
  DAC1->CR = DAC_CR_DMAEN1 | 
      (0 << DAC_CR_TSEL1_Pos) | DAC_CR_TEN1 | DAC_CR_EN1 |
      (0 << DAC_CR_TSEL2_Pos) | DAC_CR_TEN2 | DAC_CR_EN2;
  
  /* Configure DMA1 channel 3 */
  SET_BITS(DMA1_Channel3->CCR, 0, DMA_CCR_EN_Msk);
  uint32_t size = dac_buffer_fill[dac_buffer_i];
  DMA1_Channel3->CNDTR = size;
  DMA1_Channel3->CPAR = (uint32_t)((void *)&(DAC1->DHR12RD));
  DMA1_Channel3->CMAR = (uint32_t)((void *)dac_buffer[dac_buffer_i]);
  DMA1_Channel3->CCR = (2 << DMA_CCR_MSIZE_Pos) | (2 << DMA_CCR_PSIZE_Pos) | 
      DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TEIE | DMA_CCR_TCIE | DMA_CCR_EN;
  
  /* Configure TIM6 */
  TIM6->CR2 = 2 << TIM_CR2_MMS_Pos;
  TIM6->PSC = 0;
  TIM6->ARR = (24 * 12) - 1;
  TIM6->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
}


static void updateFPSDisp(t_plot *plot)
{
  char buffer[10];

  if (!show_fps_disp)
    return;

  plot_selectFont(plot, PLOT_FONT_ID_FUTURAL, FIX_1 / 15);

  t_fixp fps = 1000 * FIX_1 / (frame_time_ms);
  formatFixedPoint(fps, 10, PLUS_NONE, false, 2, 0, buffer);
  plot_moveTo(plot, -FIX_1, FIX_1*15/16);
  plot_putString(plot, "FPS: ");
  plot_putString(plot, buffer);

  unsigned load = update_time_ms * 100 / frame_time_ms;
  formatUnsignedInt(load, 10, false, 0, buffer);
  plot_moveTo(plot, -FIX_1, FIX_1*13/16);
  plot_putString(plot, "LOAD: ");
  plot_putString(plot, buffer);
}

static void updateRender(void)
{
  static uint32_t last_frame_start = 0;
  uint32_t this_update_start = ms_counter;

  t_plotRender render;
  plot_renderInit(&render, dac_buffer[dac_buffer_i], DAC_BUFFER_SZ);
  int finished = plot_render(&plot, &render);
  if (finished) {
    frame_time_ms = ms_counter - last_frame_start;
    last_frame_start = ms_counter;
    plot_init(&plot, plot_buffer, PLOT_BUFFER_SZ);
    plot_update_func(&plot);
    updateFPSDisp(&plot);
    update_time_ms = 0;
    finished = plot_render(&plot, &render);
  }
  dac_buffer_fill[dac_buffer_i] = render.i;

  update_time_ms += ms_counter - this_update_start;
}


static void continueTransfer(void)
{
  configDAC();
  dac_buffer_i = (dac_buffer_i + 1) % 2;
  updateRender();
}


void rl_init(t_plot_func *plotUpdFunc)
{
  /* Turn on DAC, TIM6, DMA1 */
  SET_BITS(RCC->APB1ENR,
      RCC_APB1ENR_DACEN | RCC_APB1ENR_TIM6EN,
      RCC_APB1ENR_DACEN_Msk | RCC_APB1ENR_TIM6EN_Msk);
  SET_BITS(RCC->AHBENR, RCC_AHBENR_DMA1EN, RCC_AHBENR_DMA1EN_Msk);

  rl_setPlotUpdateFunc(plotUpdFunc);
  #ifdef DEBUG
  rl_setEnableFPSDisplay(true);
  #endif
  plot_init(&plot, plot_buffer, PLOT_BUFFER_SZ);
  plotUpdFunc(&plot);
  updateRender();
  continueTransfer();

  __NVIC_EnableIRQ(13);
}


void DMA1_Channel3_IRQHandler(void)
{
  dac_finished_flag = true;
  SET_BITS(DMA1->IFCR, DMA_IFCR_CGIF3, DMA_IFCR_CGIF3_Msk);
}


void rl_update(void)
{
  if (dac_finished_flag) {
    continueTransfer();
  }
}
