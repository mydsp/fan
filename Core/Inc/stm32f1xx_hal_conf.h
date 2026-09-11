#ifndef __STM32F1xx_HAL_CONF_H
#define __STM32F1xx_HAL_CONF_H

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED

#ifdef USE_HAL_DRIVER
#include "stm32f1xx.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_hal_adc.h"
#endif

#define HSE_VALUE              8000000U
#define HSE_STARTUP_TIMEOUT   100U
#define LSE_STARTUP_TIMEOUT   5000U
#define HSI_VALUE              8000000U
#define LSI_VALUE              40000U
#define LSE_VALUE              32768U
#define VDD_VALUE              3300U
#define TICK_INT_PRIORITY      15U
#define PREFETCH_ENABLE        1U
#define USE_RTOS               0U
#define assert_param(expr)     ((void)0U)

#endif
