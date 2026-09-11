#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) { }
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) { }
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) { }
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc) { }