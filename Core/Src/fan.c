#include "main.h"
#include <stdlib.h>
#include <string.h>

/* =====================================================================
 * 软4 模拟风扇：电位器定目标转速 -> PID(10ms) -> AT8236 PWM -> 编码器反馈
 * FreeRTOS 单任务实现，控制周期 10ms，50ms 回传一次 VOFA+ 三通道。
 * USART1 接收 ASCII 命令，可由 VOFA+ 控件在线更新 PID 参数：
 *   KP=0.800\r\n   KI=3.000\r\n   KD=0.000\r\n   PID=0.800,3.000,0.000\r\n
 * ===================================================================== */

extern UART_HandleTypeDef huart1;

/* ================= VOFA+ PID 在线调参串口接收 ================= */
#define PID_RX_RING_SIZE 128U

static uint8_t pid_rx_byte;
static volatile uint8_t pid_rx_ring[PID_RX_RING_SIZE];
static volatile uint16_t pid_rx_head;
static volatile uint16_t pid_rx_tail;

static void PID_RxPush(uint8_t byte)
{
  uint16_t next = (uint16_t)((pid_rx_head + 1U) % PID_RX_RING_SIZE);
  if (next != pid_rx_tail) {
    pid_rx_ring[pid_rx_head] = byte;
    pid_rx_head = next;
  }
}

static int PID_RxPop(uint8_t *byte)
{
  if (pid_rx_tail == pid_rx_head) return 0;
  *byte = pid_rx_ring[pid_rx_tail];
  pid_rx_tail = (uint16_t)((pid_rx_tail + 1U) % PID_RX_RING_SIZE);
  return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    PID_RxPush(pid_rx_byte);
    (void)HAL_UART_Receive_IT(huart, &pid_rx_byte, 1);
  }
}

void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

/* ================= 编码器：TIM4 编码器模式，PB6/PB7 ================= */
static TIM_HandleTypeDef htim4;

void Encoder_Init(void)
{
  __HAL_RCC_TIM4_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_6 | GPIO_PIN_7;
#if defined(STM32F103xB)
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &g);
#else
  g.Mode = GPIO_MODE_AF_PP;
  g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  g.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &g);
#endif

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 0xFFFF;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TIM_Encoder_InitTypeDef enc = {0};
  enc.EncoderMode  = TIM_ENCODERMODE_TI12;      /* 双相双沿 4 倍频 */
  enc.IC1Polarity  = TIM_ICPOLARITY_RISING;
  enc.IC2Polarity  = TIM_ICPOLARITY_RISING;
  enc.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  enc.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  enc.IC1Prescaler = TIM_ICPSC_DIV1;
  enc.IC2Prescaler = TIM_ICPSC_DIV1;
  enc.IC1Filter = 6;                            /* 数字滤波，滤编码器毛刺 */
  enc.IC2Filter = 6;
  HAL_TIM_Encoder_Init(&htim4, &enc);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

int16_t Encoder_Delta10ms(void)
{
  static uint16_t last = 0;
  uint16_t now = __HAL_TIM_GET_COUNTER(&htim4);
  int16_t d = (int16_t)(now - last);            /* uint 回绕自动变有符号差值 */
  last = now;
  return d;
}

/* ================= AT8236 双输入 PWM：TIM3 10kHz ================= */
static TIM_HandleTypeDef htim3;

void MotorPWM_Init(void)
{
  __HAL_RCC_TIM3_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_6 | GPIO_PIN_7;
#if defined(STM32F103xB)
  g.Mode = GPIO_MODE_AF_PP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &g);
#else
  g.Mode = GPIO_MODE_AF_PP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  g.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOA, &g);
#endif

  /* F401: 84MHz；F103: APB1=36MHz 时定时器时钟×2=72MHz。均得到1MHz计数。 */
  htim3.Instance = TIM3;
#if defined(STM32F103xB)
  htim3.Init.Prescaler = 72 - 1;
#else
  htim3.Init.Prescaler = 84 - 1;
#endif
  htim3.Init.Period = 100 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  HAL_TIM_PWM_Init(&htim3);

  TIM_OC_InitTypeDef oc = {0};
  oc.OCMode = TIM_OCMODE_PWM1;
  oc.Pulse = 0;
  HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_1);
  HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

void Motor_SetDuty(int8_t duty)   /* -100~100，负值反转 */
{
  uint32_t magnitude = (duty < 0) ? (uint32_t)(-(int)duty) : (uint32_t)duty;

  /* AT8236：正转 AIN1=PWM/AIN2=0，反转 AIN1=0/AIN2=PWM。 */
  if (duty > 0) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, magnitude);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  } else if (duty < 0) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, magnitude);
  } else {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  }
}

/* ================= ADC：PA0 电位器 ================= */
static ADC_HandleTypeDef hadc1;

void PotADC_Init(void)
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_0;
  g.Mode = GPIO_MODE_ANALOG;
#if defined(STM32F103xB)
  g.Pull = GPIO_NOPULL;
#endif
  HAL_GPIO_Init(GPIOA, &g);

  hadc1.Instance = ADC1;
#if defined(STM32F103xB)
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
#else
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
#endif
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

  ADC_ChannelConfTypeDef channel = {0};
  channel.Channel = ADC_CHANNEL_0;
  channel.Rank = 1;
#if defined(STM32F103xB)
  channel.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
#else
  channel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
#endif
  if (HAL_ADC_ConfigChannel(&hadc1, &channel) != HAL_OK) Error_Handler();
}

uint16_t Pot_Read(void)
{
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 10);
  uint16_t value = (uint16_t)HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);
  return value;
}

/* ================= PID ================= */
typedef struct {
  float kp, ki, kd;
  float integ;
  float last_err;
  float out_min, out_max;
} pid_t;

static float ClampPIDValue(float value, float min_value, float max_value)
{
  if (value < min_value) return min_value;
  if (value > max_value) return max_value;
  return value;
}

static void PID_Reset(pid_t *pid)
{
  pid->kp = 0.8f;
  pid->ki = 3.0f;
  pid->kd = 0.0f;
  pid->integ = 0.0f;
  pid->last_err = 0.0f;
}

static void PID_ApplyCommand(pid_t *pid, char *line)
{
  char *end = NULL;
  float value;

  if (strcmp(line, "RESET") == 0) {
    PID_Reset(pid);
    return;
  }

  if (strncmp(line, "PID=", 4) == 0) {
    float kp = strtof(line + 4, &end);
    if (end == NULL || *end != ',') return;
    float ki = strtof(end + 1, &end);
    if (end == NULL || *end != ',') return;
    float kd = strtof(end + 1, &end);
    if (end == NULL || *end != '\0') return;
    pid->kp = ClampPIDValue(kp, 0.0f, 10.0f);
    pid->ki = ClampPIDValue(ki, 0.0f, 50.0f);
    pid->kd = ClampPIDValue(kd, 0.0f, 2.0f);
    pid->integ = 0.0f;
    pid->last_err = 0.0f;
    return;
  }

  if (strncmp(line, "KP=", 3) == 0) {
    value = strtof(line + 3, &end);
    if (end != NULL && *end == '\0') pid->kp = ClampPIDValue(value, 0.0f, 10.0f);
  } else if (strncmp(line, "KI=", 3) == 0) {
    value = strtof(line + 3, &end);
    if (end != NULL && *end == '\0') pid->ki = ClampPIDValue(value, 0.0f, 50.0f);
  } else if (strncmp(line, "KD=", 3) == 0) {
    value = strtof(line + 3, &end);
    if (end != NULL && *end == '\0') pid->kd = ClampPIDValue(value, 0.0f, 2.0f);
  } else {
    return;
  }

  /* 改参数时清积分，避免上一组参数的积分残留污染新曲线。 */
  pid->integ = 0.0f;
  pid->last_err = 0.0f;
}

static void PID_CommandPoll(pid_t *pid)
{
  static char line[48];
  static uint8_t length;
  uint8_t byte;

  while (PID_RxPop(&byte)) {
    if (byte == '\r' || byte == '\n') {
      if (length != 0U) {
        line[length] = '\0';
        PID_ApplyCommand(pid, line);
        length = 0U;
      }
    } else if (byte >= 0x20U && byte <= 0x7EU) {
      if (length < (sizeof(line) - 1U)) {
        line[length++] = (char)byte;
      } else {
        length = 0U;
      }
    }
  }
}

static float PID_Step(pid_t *p, float target, float meas, float dt)
{
  float err = target - meas;
  p->integ += p->ki * err * dt;
  if (p->integ > p->out_max) p->integ = p->out_max;   /* 积分限幅抗饱和 */
  if (p->integ < p->out_min) p->integ = p->out_min;
  float deriv = (err - p->last_err) / dt;
  p->last_err = err;
  float out = p->kp * err + p->integ + p->kd * deriv;
  if (out > p->out_max) out = p->out_max;
  if (out < p->out_min) out = p->out_min;
  return out;
}

/* ================= 参数 =================
 * 编码器每转计数 = PPR × 4倍频 × 减速比30
 * ENC_PPR 与 MAX_RPM 以 P30 铭牌/实测为准（考核题跟做\99-来源与待核对） */
#define ENC_PPR     11U
#define ENC_CPR     (ENC_PPR * 4U * 30U)
#define MAX_RPM     300.0f
#define CTRL_DT     0.01f      /* 10ms 控制周期 */
#define TARGET_DEADBAND_RPM 5.0f /* 电位器零位 ADC 噪声不应驱动电机 */

/* ================= FreeRTOS 任务 ================= */
#include "FreeRTOS.h"
#include "task.h"

static void vTaskFan(void *pv)
{
  (void)pv;
  pid_t pid = { .kp = 0.8f, .ki = 3.0f, .kd = 0.0f,
                .integ = 0, .last_err = 0, .out_min = -100, .out_max = 100 };
  TickType_t last = xTaskGetTickCount();
  uint8_t div = 0;

  for (;;) {
    PID_CommandPoll(&pid);
    float target = Pot_Read() * MAX_RPM / 4095.0f;
    float rpm = (float)Encoder_Delta10ms() * 100.0f * 60.0f / (float)ENC_CPR;
    float duty;
    if (target < TARGET_DEADBAND_RPM) {
      /* 零位安全：清除旧积分，防止无编码器/悬空输入时满占空比 */
      pid.integ = 0.0f;
      pid.last_err = 0.0f;
      duty = 0.0f;
    } else {
      duty = PID_Step(&pid, target, rpm, CTRL_DT);
    }
    Motor_SetDuty((int8_t)duty);

    /* 每 5 个周期（50ms）回传一次 VOFA+：目标转速 / 实测转速 / 占空比 */
    if (++div >= 5) {
      div = 0;
      uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7F};
      float fb[3] = {target, rpm, duty};
      HAL_UART_Transmit(&huart1, (uint8_t *)fb, 12, 20);
      HAL_UART_Transmit(&huart1, tail, 4, 20);
    }
    vTaskDelayUntil(&last, pdMS_TO_TICKS(10));
  }
}

void Fan_Start(void)
{
  Encoder_Init();
  MotorPWM_Init();
  PotADC_Init();
  HAL_UART_Receive_IT(&huart1, &pid_rx_byte, 1);
  /* strtof() 用于解析 VOFA+ 命令，给控制任务留出足够栈空间。 */
  xTaskCreate(vTaskFan, "fan", 384, NULL, tskIDLE_PRIORITY + 2, NULL);
}
