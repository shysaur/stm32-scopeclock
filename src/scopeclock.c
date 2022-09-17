#include "stm32f1xx.h"
#include <stdint.h>

#define SET_BITS(dest, bits, mask) do { \
    (dest) = (dest) & ~(mask) | (bits); \
  } while (0)

void main(void)
{
  SET_BITS(GPIOC->CRH, 
    (0 << GPIO_CRH_CNF8_Pos) | (3 << GPIO_CRH_MODE8_Pos) |
    (0 << GPIO_CRH_CNF9_Pos) | (3 << GPIO_CRH_MODE9_Pos), 
    GPIO_CRH_CNF8_Msk | GPIO_CRH_MODE8_Msk |
    GPIO_CRH_CNF9_Msk | GPIO_CRH_MODE9_Msk);
  SET_BITS(GPIOC->ODR, 1 << 8, 1 << 8);
  SET_BITS(GPIOC->ODR, 1 << 9, 1 << 9);
  for (;;) { }
}
