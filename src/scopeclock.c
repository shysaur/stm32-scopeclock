#include "stm32f1xx.h"
#include "math.h"
#include "draw.h"
#include <stdint.h>

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)

#define DAC_BUFFER_SZ 1800
uint32_t dac_buffer[DAC_BUFFER_SZ];
unsigned dial_face_end;

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
  /* Turn on DAC, TIM6, DMA1 */
  SET_BITS(RCC->APB1ENR,
      RCC_APB1ENR_DACEN | RCC_APB1ENR_TIM6EN,
      RCC_APB1ENR_DACEN_Msk | RCC_APB1ENR_TIM6EN_Msk);
  SET_BITS(RCC->AHBENR, RCC_AHBENR_DMA1EN, RCC_AHBENR_DMA1EN_Msk);

  /* Configure DAC */
  DAC1->CR = DAC_CR_DMAEN1 | 
      (0 << DAC_CR_TSEL1_Pos) | DAC_CR_TEN1 | DAC_CR_EN1 |
      (0 << DAC_CR_TSEL2_Pos) | DAC_CR_TEN2 | DAC_CR_EN2;
  
  /* Configure DMA1 channel 3 */
  DMA1_Channel3->CNDTR = DAC_BUFFER_SZ;
  DMA1_Channel3->CPAR = (uint32_t)((void *)&(DAC1->DHR12RD));
  DMA1_Channel3->CMAR = (uint32_t)((void *)dac_buffer);
  DMA1_Channel3->CCR = (2 << DMA_CCR_MSIZE_Pos) | (2 << DMA_CCR_PSIZE_Pos) | 
      DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_DIR | DMA_CCR_EN;
  
  /* Configure TIM6 */
  TIM6->CR2 = 2 << TIM_CR2_MMS_Pos;
  TIM6->PSC = 0;
  TIM6->ARR = (24 * 8) - 1;
  TIM6->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
}


void updateDialFace(void)
{
  const char *labels[] = {
    "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"
  };
  t_plotRender plot;
  plot.i = 0;
  plot.xyBuf = dac_buffer;
  plot.xyBufSz = DAC_BUFFER_SZ;

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
    plot_renderLine(&plot, x0, y0, x1, y1, 1);

    if (i % 5 == 0) {
      t_fixp height = FIX_1 * 2/16;
      x0 = x * 12 / 16;
      y0 = y * 12 / 16 - height/2;
      t_fixp width = plot_sizeString(height, labels[i/5]);
      x0 -= width / 2;
      plot_renderString(&plot, x0, y0, height, labels[i/5]);
    }
  }
  dial_face_end = plot.i;
}

void updateClockDisp(void)
{
  t_binang angle;
  t_plotRender plot;
  plot.i = dial_face_end;
  plot.xyBuf = dac_buffer;
  plot.xyBufSz = DAC_BUFFER_SZ;

  uint32_t h = ms_counter % (12 * 60 * 60 * 1000) / (12 * 60 * 60);
  angle = BINANG_90 - TO_BINANG(h, 1000);
  plot_renderLine(&plot, 0, 0, coss(angle) * 6/16, sins(angle) * 6/16, 1);

  uint32_t m = ms_counter % (60 * 60 * 1000) / (60 * 60);
  angle = BINANG_90 - TO_BINANG(m, 1000);
  plot_renderLine(&plot, 0, 0, coss(angle) * 10/16, sins(angle) * 10/16, 1);

  uint32_t s = ms_counter % (60 * 1000) / 60;
  angle = BINANG_90 - TO_BINANG(s, 1000);
  plot_renderLine(&plot, 0, 0, coss(angle) * 11/16, sins(angle) * 11/16, 1);

  for (; plot.i < plot.xyBufSz; plot.i++) {
    dac_buffer[plot.i] = 0;
  }
}

void SysTick_Handler(void)
{
  ms_counter++;
  if (ms_counter % 500 == 0) {
    uint32_t n = (ms_counter / 500) & 1;
    SET_BITS(GPIOC->ODR, n << 8, 1 << 8);
    SET_BITS(GPIOC->ODR, (n ^ 1) << 9, 1 << 9);
  }
  if (ms_counter % 50 == 0)
    updateClockDisp();
}


void main(void)
{
  /* Turn on both leds on power on */
  SET_BITS(RCC->APB2ENR, RCC_APB2ENR_IOPCEN, RCC_APB2ENR_IOPCEN_Msk);
  SET_BITS(GPIOC->CRH, 
    (0 << GPIO_CRH_CNF8_Pos) | (3 << GPIO_CRH_MODE8_Pos) |
    (0 << GPIO_CRH_CNF9_Pos) | (3 << GPIO_CRH_MODE9_Pos), 
    GPIO_CRH_CNF8_Msk | GPIO_CRH_MODE8_Msk |
    GPIO_CRH_CNF9_Msk | GPIO_CRH_MODE9_Msk);
  SET_BITS(GPIOC->ODR, 1 << 8, 1 << 8);
  SET_BITS(GPIOC->ODR, 1 << 9, 1 << 9);

  configPLLClock();

  updateDialFace();
  updateClockDisp();

  configDAC();
  SysTick_Config(24 * 1000);
  
  for (;;) {
    //__WFI();
  }
}
