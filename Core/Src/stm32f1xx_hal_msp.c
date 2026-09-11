#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) { (void)huart; }
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc) { (void)hadc; }
