# fan —— 编码器电机 PID 定速控制（STM32F103C8T6 + FreeRTOS）

模拟风扇工程：电位器给定目标转速，TIM4 编码器 4 倍频测速，10ms 位置式 PID 闭环，
AT8236 双输入 PWM 驱动 MG310 P30 电机，VOFA+ 实时绘制目标/实测/占空比三通道曲线，
并支持上位机滑条在线整定 PID 参数。
对应《G308 电控组 2026 夏季考核题》软件第四题。

## 功能特性

- FreeRTOS 双任务：10ms 控制任务（`vTaskDelayUntil` 精确节拍）+ 500ms 心跳指示
- 编码器测速：TIM4 `TIM_ENCODERMODE_TI12`（4 倍频），转速按 `ENC_CPR = PPR × 4 × 减速比` 换算
- 位置式 PID：积分限幅、输出限幅、目标死区、改参自动清积分
- AT8236 双输入 PWM：方向与占空比统一为一个有符号量，PID 输出直接可用
- VOFA+ JustFloat 三通道回传（目标 rpm / 实测 rpm / 占空比）+ ASCII 在线调参

## 硬件连接

| F103 引脚 | 连接 |
| --- | --- |
| PA6 / PA7（TIM3 CH1/CH2） | AT8236 A01 / A02 |
| PB6 / PB7（TIM4） | 编码器 A 相 / B 相 |
| PA0（ADC1_IN0） | 电位器中间脚（两端 3.3V / GND） |
| PA9 / PA10 | USART1 115200，USB 转串口 |
| 电机电源 | AT8236 Vin（独立 5~12V），与逻辑侧共地 |

## 构建与烧录

```bat
cmake --preset F103
cmake --build --preset F103
pyocd flash -t stm32f103c8 build\f103\fan.bin
```

F401 基线：`cmake --build build` + `pyocd flash -t stm32f401retx build\fan.bin`。

## 在线调参

串口发送以下 ASCII 命令（回车结束）：`KP=0.94`、`KI=1.6`、`KD=0`、`PID=0.94,1.6,0`、`RESET`。
建议配合 VOFA+ 滑条使用；改参时固件自动清积分。

## 文档与交付物

| 材料 | 文件 |
| --- | --- |
| 开发过程说明 | [docs/04-开发过程说明.md](docs/04-开发过程说明.md) |
| 实操视频 | [docs/video/soft4-demo.mp4](docs/video/soft4-demo.mp4) |
| 调参截图 | [docs/img/](docs/img/) |
| VOFA+ 配置与调参记录 | [docs/vofa-pid-slider-minimal.md](docs/vofa-pid-slider-minimal.md) |

## 已知限制

- 被控量为转速；如需转角定位，反馈需由计数增量改为累计计数（说明见 docs/04 §7）
- `ENC_PPR` / `MAX_RPM` 定义于 `fan.c` 顶部，更换电机型号时同步更新
- 测速为 10ms 差分、未加滤波，启用 Kd 前建议先加测速滤波
