#ifndef __MAIN_H
#define __MAIN_H

#if defined(STM32F103xB)
#include "stm32f1xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif

/* ====== 软4 模拟风扇：引脚规划（F401RET6 / F103C8T6） ======
 * TIM3_CH1  PA6   -> AT8236 AIN1（正转 PWM，10kHz）
 * TIM3_CH2  PA7   -> AT8236 AIN2（反转 PWM，10kHz）
 * AT8236 没有独立 PWMA/STBY：AIN1/AIN2 直接采用双输入 PWM
 * TIM4_CH1/2 PB6/PB7 -> 电机霍尔编码器 A/B
 * ADC1_IN0  PA0   -> WH148 电位器中间脚（两端接 3.3V/GND）
 * USART1    PA9/PA10 -> 串口 115200（VOFA+ 观测）
 * LED       PA5   心跳灯；F103 版本使用同一组功能引脚
 */
#define UART_BAUD            115200

#define LED_PIN              GPIO_PIN_5
#define LED_PORT             GPIOA

void Fan_Start(void);
void Error_Handler(void);

#endif
