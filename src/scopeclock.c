#include "stm32f1xx.h"
#include <stdint.h>

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)

#define DAC_BUFFER_SZ 1000
uint32_t dac_buffer[1000];


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

  /* Systick device configuration */
  SysTick_Config(24 * 1000);
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
  TIM6->ARR = 24 - 1;
  TIM6->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
}


void SysTick_Handler(void)
{
  static uint32_t clock = 0;
  clock++;
  if (clock % 500 == 0) {
    uint32_t n = (clock / 500) & 1;
    SET_BITS(GPIOC->ODR, n << 8, 1 << 8);
    SET_BITS(GPIOC->ODR, (n ^ 1) << 9, 1 << 9);
  }
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

  for (int i=0; i<DAC_BUFFER_SZ; i++) {
    uint16_t sa = i * 2048 / DAC_BUFFER_SZ + 1024;
    dac_buffer[i] = sa | ((4096-sa) << 16);
  }

  configDAC();
  
  for (;;) {
    //__WFI();
  }
}
