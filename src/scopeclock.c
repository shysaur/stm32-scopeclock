#include "stm32f1xx.h"
#include <stdint.h>

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)


void configClockDevice(void)
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
  SET_BITS(RCC->APB2ENR, RCC_APB2ENR_IOPCEN, RCC_APB2ENR_IOPCEN_Msk);
  SET_BITS(GPIOC->CRH, 
    (0 << GPIO_CRH_CNF8_Pos) | (3 << GPIO_CRH_MODE8_Pos) |
    (0 << GPIO_CRH_CNF9_Pos) | (3 << GPIO_CRH_MODE9_Pos), 
    GPIO_CRH_CNF8_Msk | GPIO_CRH_MODE8_Msk |
    GPIO_CRH_CNF9_Msk | GPIO_CRH_MODE9_Msk);
  SET_BITS(GPIOC->ODR, 1 << 8, 1 << 8);
  SET_BITS(GPIOC->ODR, 1 << 9, 1 << 9);

  configClockDevice();
  
  for (;;) {
    __WFI();
  }
}
