#include "stm32f1xx.h"
#include "math.h"
#include "draw.h"
#include "utils.h"
#include "dial_pic.h"
#include <stdint.h>
#include <stdbool.h>

#define DAC_BUFFER_SZ 900
uint32_t dac_buffer[2][DAC_BUFFER_SZ];
unsigned dac_buffer_i;
uint32_t dac_buffer_fill[2];

#define PLOT_BUFFER_SZ 256
uint8_t plot_buffer[PLOT_BUFFER_SZ];
t_plot plot;

volatile bool dac_finished_flag;
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


void configDAC(void)
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


void updateDisp(uint32_t ms_timestamp)
{
  t_binang angle;

  plot_init(&plot, plot_buffer, PLOT_BUFFER_SZ);

  initDialPlot();
  plot_invoke(&plot, &dial_plot);

  uint32_t h = ms_timestamp % (12 * 60 * 60 * 1000) / (12 * 60 * 60);
  angle = BINANG_90 - TO_BINANG(h, 1000);
  plot_moveTo(&plot, 0, 0);
  plot_lineTo(&plot, coss(angle) * 6/16, sins(angle) * 6/16, 1);

  uint32_t m = ms_timestamp % (60 * 60 * 1000) / (60 * 60);
  angle = BINANG_90 - TO_BINANG(m, 1000);
  plot_moveTo(&plot, 0, 0);
  plot_lineTo(&plot, coss(angle) * 10/16, sins(angle) * 10/16, 1);

  uint32_t s = ms_timestamp % (60 * 1000) / 60;
  angle = BINANG_90 - TO_BINANG(s, 1000);
  plot_moveTo(&plot, 0, 0);
  plot_lineTo(&plot, coss(angle) * 11/16, sins(angle) * 11/16, 1);

  static const t_fixp time_size = FIX_1/7;
  t_fixp width = plot_sizeString(time_size, "00:00:00");
  plot_moveTo(&plot, -width/2, -FIX_1*1/3);
  char str[4];
  str[2] = ':';
  str[3] = '\0';
  int tt;
  tt = (ms_timestamp / (1000 * 60 * 60)) % 24;
  str[1] = tt % 10 + '0';
  str[0] = tt / 10 + '0';
  plot_putString(&plot, time_size, str);
  tt = (ms_timestamp / (1000 * 60)) % 60;
  str[1] = tt % 10 + '0';
  str[0] = tt / 10 + '0';
  plot_putString(&plot, time_size, str);
  tt = (ms_timestamp / 1000) % 60;
  str[2] = '\0';
  str[1] = tt % 10 + '0';
  str[0] = tt / 10 + '0';
  plot_putString(&plot, time_size, str);
}

void updateRender(void)
{
  t_plotRender render;
  plot_renderInit(&render, dac_buffer[dac_buffer_i], DAC_BUFFER_SZ);
  int finished = plot_render(&plot, &render);
  if (finished) {
    updateDisp(ms_counter);
    finished = plot_render(&plot, &render);
  }
  dac_buffer_fill[dac_buffer_i] = render.i;
}

void continueTransfer(void)
{
  configDAC();
  dac_buffer_i = (dac_buffer_i + 1) % 2;
  updateRender();
}

void DMA1_Channel3_IRQHandler(void)
{
  dac_finished_flag = true;
  SET_BITS(DMA1->IFCR, DMA_IFCR_CGIF3, DMA_IFCR_CGIF3_Msk);
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
  /* Configure LED GPIOs */
  SET_BITS(RCC->APB2ENR, RCC_APB2ENR_IOPCEN, RCC_APB2ENR_IOPCEN_Msk);
  SET_BITS(GPIOC->CRH, 
    (0 << GPIO_CRH_CNF8_Pos) | (3 << GPIO_CRH_MODE8_Pos) |
    (0 << GPIO_CRH_CNF9_Pos) | (3 << GPIO_CRH_MODE9_Pos), 
    GPIO_CRH_CNF8_Msk | GPIO_CRH_MODE8_Msk |
    GPIO_CRH_CNF9_Msk | GPIO_CRH_MODE9_Msk);
  /* Turn on DAC, TIM6, DMA1 */
  SET_BITS(RCC->APB1ENR,
      RCC_APB1ENR_DACEN | RCC_APB1ENR_TIM6EN,
      RCC_APB1ENR_DACEN_Msk | RCC_APB1ENR_TIM6EN_Msk);
  SET_BITS(RCC->AHBENR, RCC_AHBENR_DMA1EN, RCC_AHBENR_DMA1EN_Msk);

  configPLLClock();
  SysTick_Config(24 * 1000);

  updateDisp(0);
  updateRender();
  continueTransfer();

  __NVIC_EnableIRQ(13);
  
  for (;;) {
    //__WFI();
    if (dac_finished_flag) {
      continueTransfer();
    }
  }
}
