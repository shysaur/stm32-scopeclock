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


void rl_setPlotUpdateFunc(t_plot_func *plotUpdFunc)
{
  plot_update_func = plotUpdFunc;
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


static void updateRender(void)
{
  t_plotRender render;
  plot_renderInit(&render, dac_buffer[dac_buffer_i], DAC_BUFFER_SZ);
  int finished = plot_render(&plot, &render);
  if (finished) {
    plot_init(&plot, plot_buffer, PLOT_BUFFER_SZ);
    plot_update_func(&plot);
    finished = plot_render(&plot, &render);
  }
  dac_buffer_fill[dac_buffer_i] = render.i;
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
