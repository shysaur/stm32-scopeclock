/*
*************** (C) COPYRIGHT 2017 STMicroelectronics ************************
* @file      startup_stm32f100xb.s
* @author    MCD Application Team
* @brief     STM32F100xB Devices vector table for Atollic toolchain.
*            This module performs:
*                - Set the initial SP
*                - Set the initial PC == Reset_Handler,
*                - Set the vector table entries with the exceptions ISR address
*                - Configure the clock system   
*                - Branches to main in the C library (which eventually
*                  calls main()).
*            After Reset the Cortex-M3 processor is in Thread mode,
*            priority is Privileged, and the Stack is set to Main.
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
*/

.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.global g_pfnVectors
.global Default_Handler
.global Reset_Handler

.equ  BootRAM, 0xF108F85F


.section .text.Reset_Handler

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
 */
.type Reset_Handler, %function
Reset_Handler:
  // Zero fill the bss segment.
  ldr r2, =_sbss
  ldr r3, =_bss_sz
  movs r4, #0
  b .Lbss_init_loop_cond
.Lbss_init_loop:
  str r4, [r3, r2]
.Lbss_init_loop_cond:
  subs r3, r3, #4 
  bpl .Lbss_init_loop

  // Copy the data segment initializers from flash to SRAM
  ldr r0, =_sdata
  ldr r1, =_sidata
  ldr r2, =_data_sz
  b .Ldata_init_loop_cond
.Ldata_init:
  ldr r3, [r1, r2]
  str r3, [r0, r2]
.Ldata_init_loop_cond:
  subs r2, r2, #4
  bpl .Ldata_init

  // Call static constructors
  //bl __libc_init_array

  // Call the application's entry point.
  bl main
  // Main has exited, hang
.Lhang:
  b .Lhang

.size Reset_Handler, .-Reset_Handler


.section .text.Default_Handler,"ax",%progbits

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
 */
Default_Handler:
Infinite_Loop:
  b Infinite_Loop

.size Default_Handler, .-Default_Handler


.section .isr_vector,"a",%progbits

/******************************************************************************
*
* The minimal vector table for a Cortex M3.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
  
.type g_pfnVectors, %object
g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  .word WWDG_IRQHandler
  .word PVD_IRQHandler
  .word TAMPER_IRQHandler
  .word RTC_IRQHandler
  .word FLASH_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_IRQHandler
  .word EXTI1_IRQHandler
  .word EXTI2_IRQHandler
  .word EXTI3_IRQHandler
  .word EXTI4_IRQHandler
  .word DMA1_Channel1_IRQHandler
  .word DMA1_Channel2_IRQHandler
  .word DMA1_Channel3_IRQHandler
  .word DMA1_Channel4_IRQHandler
  .word DMA1_Channel5_IRQHandler
  .word DMA1_Channel6_IRQHandler
  .word DMA1_Channel7_IRQHandler
  .word ADC1_IRQHandler
  .word 0
  .word 0
  .word 0
  .word 0
  .word EXTI9_5_IRQHandler
  .word TIM1_BRK_TIM15_IRQHandler
  .word TIM1_UP_TIM16_IRQHandler
  .word TIM1_TRG_COM_TIM17_IRQHandler
  .word TIM1_CC_IRQHandler
  .word TIM2_IRQHandler
  .word TIM3_IRQHandler
  .word TIM4_IRQHandler
  .word I2C1_EV_IRQHandler
  .word I2C1_ER_IRQHandler
  .word I2C2_EV_IRQHandler
  .word I2C2_ER_IRQHandler
  .word SPI1_IRQHandler
  .word SPI2_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word USART3_IRQHandler
  .word EXTI15_10_IRQHandler
  .word RTC_Alarm_IRQHandler
  .word CEC_IRQHandler
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word TIM6_DAC_IRQHandler
  .word TIM7_IRQHandler  
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word BootRAM          /* @0x01CC. This is for boot in RAM mode for 
                            STM32F10xB Value Line devices. */
.size g_pfnVectors, .-g_pfnVectors


/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/
    
.weak  NMI_Handler
.thumb_set NMI_Handler,Default_Handler

.weak  HardFault_Handler
.thumb_set HardFault_Handler,Default_Handler

.weak  MemManage_Handler
.thumb_set MemManage_Handler,Default_Handler

.weak  BusFault_Handler
.thumb_set BusFault_Handler,Default_Handler

.weak  UsageFault_Handler
.thumb_set UsageFault_Handler,Default_Handler

.weak  SVC_Handler
.thumb_set SVC_Handler,Default_Handler

.weak  DebugMon_Handler
.thumb_set DebugMon_Handler,Default_Handler

.weak  PendSV_Handler
.thumb_set PendSV_Handler,Default_Handler

.weak  SysTick_Handler
.thumb_set SysTick_Handler,Default_Handler

.weak  WWDG_IRQHandler
.thumb_set WWDG_IRQHandler,Default_Handler

.weak  PVD_IRQHandler
.thumb_set PVD_IRQHandler,Default_Handler

.weak  TAMPER_IRQHandler
.thumb_set TAMPER_IRQHandler,Default_Handler

.weak  RTC_IRQHandler
.thumb_set RTC_IRQHandler,Default_Handler

.weak  FLASH_IRQHandler
.thumb_set FLASH_IRQHandler,Default_Handler

.weak  RCC_IRQHandler
.thumb_set RCC_IRQHandler,Default_Handler

.weak  EXTI0_IRQHandler
.thumb_set EXTI0_IRQHandler,Default_Handler

.weak  EXTI1_IRQHandler
.thumb_set EXTI1_IRQHandler,Default_Handler

.weak  EXTI2_IRQHandler
.thumb_set EXTI2_IRQHandler,Default_Handler

.weak  EXTI3_IRQHandler
.thumb_set EXTI3_IRQHandler,Default_Handler

.weak  EXTI4_IRQHandler
.thumb_set EXTI4_IRQHandler,Default_Handler

.weak  DMA1_Channel1_IRQHandler
.thumb_set DMA1_Channel1_IRQHandler,Default_Handler

.weak  DMA1_Channel2_IRQHandler
.thumb_set DMA1_Channel2_IRQHandler,Default_Handler

.weak  DMA1_Channel3_IRQHandler
.thumb_set DMA1_Channel3_IRQHandler,Default_Handler

.weak  DMA1_Channel4_IRQHandler
.thumb_set DMA1_Channel4_IRQHandler,Default_Handler

.weak  DMA1_Channel5_IRQHandler
.thumb_set DMA1_Channel5_IRQHandler,Default_Handler

.weak  DMA1_Channel6_IRQHandler
.thumb_set DMA1_Channel6_IRQHandler,Default_Handler

.weak  DMA1_Channel7_IRQHandler
.thumb_set DMA1_Channel7_IRQHandler,Default_Handler

.weak  ADC1_IRQHandler
.thumb_set ADC1_IRQHandler,Default_Handler

.weak  EXTI9_5_IRQHandler
.thumb_set EXTI9_5_IRQHandler,Default_Handler

.weak  TIM1_BRK_TIM15_IRQHandler
.thumb_set TIM1_BRK_TIM15_IRQHandler,Default_Handler

.weak  TIM1_UP_TIM16_IRQHandler
.thumb_set TIM1_UP_TIM16_IRQHandler,Default_Handler

.weak  TIM1_TRG_COM_TIM17_IRQHandler
.thumb_set TIM1_TRG_COM_TIM17_IRQHandler,Default_Handler

.weak  TIM1_CC_IRQHandler
.thumb_set TIM1_CC_IRQHandler,Default_Handler

.weak  TIM2_IRQHandler
.thumb_set TIM2_IRQHandler,Default_Handler

.weak  TIM3_IRQHandler
.thumb_set TIM3_IRQHandler,Default_Handler

.weak  TIM4_IRQHandler
.thumb_set TIM4_IRQHandler,Default_Handler

.weak  I2C1_EV_IRQHandler
.thumb_set I2C1_EV_IRQHandler,Default_Handler

.weak  I2C1_ER_IRQHandler
.thumb_set I2C1_ER_IRQHandler,Default_Handler

.weak  I2C2_EV_IRQHandler
.thumb_set I2C1_EV_IRQHandler,Default_Handler

.weak  I2C2_ER_IRQHandler
.thumb_set I2C1_ER_IRQHandler,Default_Handler

.weak  SPI1_IRQHandler
.thumb_set SPI1_IRQHandler,Default_Handler

.weak  SPI1_IRQHandler
.thumb_set SPI2_IRQHandler,Default_Handler

.weak  USART1_IRQHandler
.thumb_set USART1_IRQHandler,Default_Handler

.weak  USART2_IRQHandler
.thumb_set USART2_IRQHandler,Default_Handler

.weak  USART3_IRQHandler
.thumb_set USART3_IRQHandler,Default_Handler

.weak  EXTI15_10_IRQHandler
.thumb_set EXTI15_10_IRQHandler,Default_Handler

.weak  RTC_Alarm_IRQHandler
.thumb_set RTC_Alarm_IRQHandler,Default_Handler

.weak  CEC_IRQHandler
.thumb_set CEC_IRQHandler,Default_Handler

.weak  TIM6_DAC_IRQHandler
.thumb_set TIM6_DAC_IRQHandler,Default_Handler

.weak  TIM7_IRQHandler
.thumb_set TIM7_IRQHandler,Default_Handler  
