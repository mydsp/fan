#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED

#ifdef USE_HAL_DRIVER
 #include "stm32f4xx.h"
 #ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
 #endif
 #ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
 #endif
 #ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
 #endif
 #ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f4xx_hal_pwr.h"
 #endif
 #ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f4xx_hal_flash.h"
 #endif
 #ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32f4xx_hal_exti.h"
 #endif
 #ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
 #endif
 #ifdef HAL_UART_MODULE_ENABLED
  #include "stm32f4xx_hal_uart.h"
 #endif
 #ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32f4xx_hal_tim.h"
 #endif
 #ifdef HAL_I2C_MODULE_ENABLED
  #include "stm32f4xx_hal_i2c.h"
 #endif
 #ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32f4xx_hal_adc.h"
 #endif
#endif

#if !defined (HSE_VALUE)
  #define HSE_VALUE              8000000U
#endif
#if !defined (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U
#endif
#if !defined (HSI_VALUE)
  #define HSI_VALUE              16000000U
#endif
#if !defined (LSI_VALUE)
  #define LSI_VALUE              32000U
#endif
#if !defined (LSE_VALUE)
  #define LSE_VALUE              32768U
#endif
#if !defined (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U
#endif
#if !defined (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE   12288000U
#endif
#if !defined (VDD_VALUE)
  #define VDD_VALUE              3300U
#endif
#if !defined (TICK_INT_PRIORITY)
  #define TICK_INT_PRIORITY      15U
#endif
#if !defined (USE_RTOS)
  #define USE_RTOS               0U
#endif
#if !defined (PREFETCH_ENABLE)
  #define PREFETCH_ENABLE        1U
#endif
#if !defined (INSTRUCTION_CACHE_ENABLE)
  #define INSTRUCTION_CACHE_ENABLE 1U
#endif
#if !defined (DATA_CACHE_ENABLE)
  #define DATA_CACHE_ENABLE      1U
#endif

#define assert_param(expr) ((void)0U)

#ifdef __cplusplus
}
#endif
#endif