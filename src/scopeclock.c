#include "stm32f1xx.h"
#include "math.h"
#include <stdint.h>

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)

#define DAC_BUFFER_SZ 1800
uint32_t dac_buffer[DAC_BUFFER_SZ];

#define OFF_X 1536
#define OFF_Y 1536
#define AMP_X 1024
#define AMP_Y 1024
#define FIX_TO_DAC(x) ((x) / (FIX_1 / AMP_X) + OFF_X)

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


unsigned drawHand(uint32_t binangle, int32_t length, unsigned i, int32_t c)
{
  int32_t x0 = FIX_TO_DAC(sins(binangle) * 2 / 16);
  int32_t y0 = FIX_TO_DAC(coss(binangle) * 2 / 16);
  int32_t x1 = FIX_TO_DAC(sins(binangle) * length / 16);
  int32_t y1 = FIX_TO_DAC(coss(binangle) * length / 16);
  for (unsigned j=0; j<c; i++, j++) {
    int32_t sa1 = (x0 * (c - j) + x1 * j) / c;
    int32_t sa2 = (y0 * (c - j) + y1 * j) / c;
    dac_buffer[i] = sa2 | (sa1 << 16);
  }
  return i;
}

void updateClockDisp(void)
{
  uint32_t h = ms_counter % (12 * 60 * 60 * 1000) / (12 * 60 * 60);
  uint32_t m = ms_counter % (60 * 60 * 1000) / (60 * 60);
  uint32_t s = ms_counter % (60 * 1000) / 60;

  unsigned i = DAC_BUFFER_SZ/2;
  i = drawHand(BINANG_90 - TO_BINANG(h, 1000), 8, i, (DAC_BUFFER_SZ/2)/3);
  i = drawHand(BINANG_90 - TO_BINANG(m, 1000), 14, i, (DAC_BUFFER_SZ/2)/3);
  i = drawHand(BINANG_90 - TO_BINANG(s, 1000), 15, i, (DAC_BUFFER_SZ/2)/3);
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

  int i, j;
  int32_t x0,y0,x1,y1;
  for (i=0; i<DAC_BUFFER_SZ/2; i++) {
    uint32_t binang = TO_BINANG(i, DAC_BUFFER_SZ/2);
    int32_t sa1 = FIX_TO_DAC(sins(binang));
    int32_t sa2 = FIX_TO_DAC(coss(binang));
    dac_buffer[i] = sa1 | (sa2 << 16);
  }
  updateClockDisp();

  configDAC();
  SysTick_Config(24 * 1000);
  
  for (;;) {
    //__WFI();
  }
}
