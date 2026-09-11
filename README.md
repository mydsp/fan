# fan —— 软4：模拟风扇 PID（F103 正式目标 / F401 基线 + FreeRTOS + AT8236 + MG310 P30）

> 一句话：电位器定目标转速 → PID（10ms）→ AT8236 双输入 PWM 驱动 → 霍尔编码器反馈，VOFA+ 实时看 目标/转速/占空比 三根曲线。
> 对应《G308 电控组 2026 夏季考核题》软件第四题。

## 交付物

| 题面提交要求 | 位置 |
| --- | --- |
| （1）项目文件 | 本仓库源码（F401 基线 + F103 正式两套预设） |
| （2）开发过程的简要说明 | [`docs/04-开发过程说明.md`](docs/04-开发过程说明.md) |
| （3）实操视频 | [`docs/video/soft4-demo.mp4`](docs/video/soft4-demo.mp4) |
| 补充证据：调参过程截图（三组参数对比） | [`docs/img/soft4-tuning-01.jpg`](docs/img/soft4-tuning-01.jpg) ~ [`03`](docs/img/soft4-tuning-03.jpg) |

关于"定速定位"中被控量的取舍（本题取**转速**，题面（2）原文即"转速或转动角度"二选一），论证与"转角定位"的扩展路径见开发过程说明 §8。

## 当前状态

- F401 基线已编译、烧录和运行验证通过：`build/Debug/fan.elf`；F401 通过 SWD 烧录后，USART1/COM4 可持续输出 JustFloat（2026-09-05）
- **F103 正式目标已接电机实测通过**（2026-09-11 取证）：电位器调速、PID 定速闭环、VOFA+ 在线调参（滑条）均已验证，演示视频见交付物表。
- 已完成的调参结果：Kp≈0.94 / Ki≈1.6 / Kd=0（Kd 保持 0 是因为编码器测速噪声会被微分项放大）。三组参数对比见 `docs/img/`。
- USART1 已支持 VOFA+ 控件在线调 PID：发送 `KP=...`、`KI=...`、`KD=...` 或 `PID=kp,ki,kd` 并以回车结束；JustFloat 输出格式不变。
- 转速换算参数 ENC_PPR、MAX_RPM 定义于 fan.c 顶部，更换电机型号时同步更新。


### FreeRTOS 启动注意

`Core/Inc/FreeRTOSConfig.h` 将 `vPortSVCHandler` 和 `xPortPendSVHandler` 直接映射到 `SVC_Handler`、`PendSV_Handler`，以保留 FreeRTOS GCC/Cortex-M4 端口要求的裸异常现场；优先级配置使用左移后的 NVIC 值（最大系统调用优先级 `0x50`，内核优先级 `0xF0`）。这两项不能删改成空中断函数，否则现象是上电无串口数据、无心跳，CPU 停在 `HardFault_Handler`。

## 控制链

```
PA0 ADC(电位器) → 目标转速 RPM → PID(kp0.8/ki3.0, 10ms, 积分限幅) → TIM3 CH1/CH2 双输入 PWM 10kHz → AT8236 → MG310 P30
                                                      ↑
                    TIM4 编码器模式(PB6/PB7, 4倍频) ← 霍尔编码器
```

## VOFA+ 在线调 PID

当前固件从 USART1 接收 ASCII 命令，同时继续向同一串口发送 JustFloat 曲线。每条命令必须以回车或换行结束：

```text
KP=0.200\r\n
KI=0.000\r\n
KD=0.000\r\n
PID=0.800,3.000,0.000\r\n
RESET\r\n
```

按 VOFA+ 官方控件文档，滑条要通过“右键 → 绑定命令”发送参数。命令选择 `Str/Ascii` 模式，在命令内容中使用 `%f` 占位符：

```text
Kp 滑条命令：KP=%f\n
Ki 滑条命令：KI=%f\n
Kd 滑条命令：KD=%f\n
```

滑条的当前浮点值会替换 `%f`；例如 Kp 滑条取 0.8，实际发出 `KP=0.8` 加换行。不要让滑条只发送裸数字，因为固件无法判断这个数字属于 Kp、Ki 还是 Kd。VOFA+ 的 Hex 模式会发送滑条数值的 IEEE754 小端字节，本固件当前不使用该模式。当前参数安全范围为：`0≤kp≤10`、`0≤ki≤50`、`0≤kd≤2`。每次收到参数命令后，固件会清除积分项，避免上一组参数污染新曲线。

官方参考：[slider 控件](https://www.vofa.plus/docs/learning/widgets/slider)；[数据、命令、参数](https://www.vofa.plus/docs/learning/start/data_cmd_parameter)。

调参记录时固定电位器目标和机械状态，清空 VOFA+ 缓冲区后分别记录 P-only、PI 和最终参数曲线；三个 JustFloat 通道仍依次为目标 RPM、实际 RPM、占空比。

## 接线（引脚级；F103 与当前功能引脚相同）

| F103/F401 | 去向 | 备注 |
| --- | --- | --- |
| PA6 (TIM3_CH1) | AT8236 AIN1 | 正转 PWM，10kHz |
| PA7 (TIM3_CH2) | AT8236 AIN2 | 反转 PWM，10kHz |
| PB6 / PB7 (TIM4) | 编码器 A / B | TI12 双沿 4 倍频，滤波 6 |
| PA0 (ADC1_IN0) | 电位器中间脚 | 两端 3.3V/GND |
| PA9 / PA10 | 串口 | 115200，JustFloat 50ms |
| 电机电源 | AT8236 Vin+/Vin- | 按驱动板标注供电，与逻辑共地 |

## 待核对参数（写死在 `Core/Src/fan.c` 顶部宏）

| 宏 | 当前值 | 依据 |
| --- | --- | --- |
| `ENC_PPR` | 11 | 霍尔编码器常见值，**以 P30 铭牌/实测为准** |
| `MAX_RPM` | 300 | P30 12V 空载约 300rpm，**以实测为准** |

电位器目标低于 `TARGET_DEADBAND_RPM=5` 时，固件清除 PID 积分并强制 PWM 为 0，避免上电时 PA0 悬空或零位 ADC 噪声误驱动电机。

## 构建与烧录

```powershell
cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM=E:\STM32CubeCLT_1.21.0\Ninja\bin\ninja.exe -DCMAKE_TOOLCHAIN_FILE=cmake\gcc-arm-none-eabi.cmake
cmake --build build
pyocd flash -t stm32f401retx build\fan.bin
```

F103 正式固件：

```powershell
cmake --preset F103
cmake --build --preset F103
pyocd list
pyocd flash -t stm32f103c8 build\f103\fan.bin
```

F103 版本使用 72MHz 时钟、F1 HAL、Cortex-M3 FreeRTOS port 和独立的 64KB/20KB 链接脚本；不要把 `build\Debug\fan.bin` 烧到 F103。

## 已知限制

- FreeRTOS 内核取自本机 CubeFW F4 的 Middlewares，换机编译改 `CMakeLists.txt` 的 `FW_DIR`
- PID 初值偏保守，调参过程本身就是提交材料（每组参数截一张波形图）
