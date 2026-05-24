# STM32 HAL 超声波测距教程（TIM1 输入捕获版）

## 1. 适用环境

这份教程基于当前工程整理，适合刚开始学习 STM32 HAL 定时器输入捕获的同学。

- MCU：`STM32L432KCUx`
- 开发环境：`STM32CubeIDE`
- 驱动库：`HAL`
- 超声波模块：`HC-SR04` 或同类模块
- 实现思路：使用 `TIM1` 输入捕获，`CH1` 捕获上升沿，`CH2` 捕获下降沿

这套写法的核心优点是：同一个 `Echo` 信号进入定时器后，可以分别记录回波高电平开始和结束的时间，再计算脉宽，从而换算距离。

---

## 2. 接线说明

按照当前工程的定义：

- `Echo -> PA8`，对应 `TIM1_CH1`
- `Trig -> PA11`，普通 GPIO 输出
- `VCC -> 5V` 或按模块要求供电
- `GND -> GND`

当前工程中引脚定义在 `Core/Inc/main.h`：

```c
#define Echo_Pin GPIO_PIN_8
#define Echo_GPIO_Port GPIOA
#define Trig_Pin GPIO_PIN_11
#define Trig_GPIO_Port GPIOA
```

`PA11` 用来发送触发脉冲，`PA8` 用来接收超声波模块返回的 `Echo` 脉冲。

---

## 3. 原理先讲明白

超声波模块的工作流程是这样的：

1. 单片机给 `Trig` 一个触发脉冲。
2. 超声波模块开始发射超声波。
3. 模块把 `Echo` 拉高。
4. 超声波遇到障碍物后返回。
5. 模块把 `Echo` 拉低。

所以：

- `Echo` 上升沿：表示开始计时
- `Echo` 下降沿：表示结束计时
- `Echo` 高电平持续时间：就是超声波往返所花的时间

距离公式：

```c
distance = (downedge - upedge) * 0.034 / 2;
```

解释：

- `downedge - upedge`：高电平持续时间
- `0.034`：声速约为 `0.034 cm/us`
- `/2`：因为超声波走了“去 + 回”两段路程

---

## 4. 为什么这里选 TIM1 双通道输入捕获

当前工程的做法不是“一个通道反复切换极性”，而是更直观的双通道捕获：

- `TIM1_CH1`：设置成**直接通道**，捕获上升沿
- `TIM1_CH2`：设置成**间接通道**，捕获下降沿

这样做的好处是：

- 不用在中断里来回切换极性
- 上升沿和下降沿分工清楚
- 逻辑更适合初学者理解

当前 `Echo` 实际接在 `PA8` 上，也就是 `TIM1_CH1` 的输入引脚。`CH2` 虽然没有单独接一根线，但它可以通过 **IndirectTI** 的方式去使用同一个输入源。

---

## 5. CubeMX 配置步骤

下面按照当前工程实际配置来说明。

### 5.1 GPIO 配置

#### Trig 引脚

把 `PA11` 配置为普通推挽输出：

- Mode：`GPIO_Output`
- Pull：`No pull-up and no pull-down`
- Output Level：默认低电平

对应代码会出现在 `gpio.c` 中，类似：

```c
HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);

GPIO_InitStruct.Pin = Trig_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(Trig_GPIO_Port, &GPIO_InitStruct);
```

#### Echo 引脚

`PA8` 作为 `TIM1_CH1`，由定时器复用功能接管，不需要手写普通输入模式。

---

### 5.2 TIM1 配置

进入 `TIM1`：

- `Channel1` 选择输入捕获
- 配置 `CH1` 捕获上升沿
- 配置 `CH2` 为间接输入捕获下降沿
- 使能 `TIM1_CC` 中断

当前工程在 `Core/Src/tim.c` 中对应的关键配置如下：

```c
htim1.Instance = TIM1;
htim1.Init.Prescaler = 72 - 1;
htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
htim1.Init.Period = 65535;
htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim1.Init.RepetitionCounter = 0;
htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
```

再看通道配置：

```c
sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
sConfigIC.ICFilter = 0;
HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1);

sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_2);
```

含义分别是：

- `CH1 + RISING + DIRECTTI`：直接从 `PA8` 这个输入脚抓上升沿
- `CH2 + FALLING + INDIRECTTI`：使用同一输入源抓下降沿

---

### 5.3 中断配置

当前工程中 `TIM1_CC` 中断已经使能，代码在 `Core/Src/tim.c`：

```c
HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
```

中断服务函数在 `Core/Src/stm32l4xx_it.c` 中会调用：

```c
HAL_TIM_IRQHandler(&htim1);
```

这一步非常重要。没有这一步，即使硬件捕获到了边沿，HAL 回调函数也不会被执行。

---

## 6. `main.c` 里需要加哪些代码

下面是当前工程这套超声波功能的核心代码结构。

### 6.1 头文件

在 `main.c` 中包含：

```c
#include "tim.h"
#include "gpio.h"
#include <stdio.h>
```

作用：

- `tim.h`：使用 `htim1`
- `gpio.h`：控制 `Trig` 引脚
- `stdio.h`：使用 `sprintf`

---

### 6.2 全局变量

在 `main.c` 的用户代码区定义：

```c
int upedge = 0;
int downedge = 0;
float distance = 0;
```

作用：

- `upedge`：保存上升沿捕获值
- `downedge`：保存下降沿捕获值
- `distance`：保存最终计算得到的距离

---

### 6.3 输入捕获回调函数

这一段最关键。

```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        upedge = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
        downedge = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_2);
        distance = (downedge - upedge) * 0.034 / 2;
    }
}
```

### 6.4 这段代码怎么理解

先看判断条件：

```c
htim == &htim1
```

表示：当前进入回调的，确实是 `TIM1`。

再看这一句：

```c
htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2
```

表示：当前触发回调的是 `CH2`，也就是**下降沿已经来了**。

为什么这里判断 `CH2`？

因为只有下降沿来了，说明一个完整的高电平脉冲已经结束，这时候：

- `CH1` 里已经记录了上升沿时间
- `CH2` 里已经记录了下降沿时间

于是就可以一起读出来计算距离。

如果你在 `CH1` 上升沿一到就算距离，那时下降沿还没来，数据是不完整的。

---

## 7. 定时器启动代码

光配置 `MX_TIM1_Init()` 还不够，还要真正启动定时器和输入捕获。

在 `main()` 初始化部分加入：

```c
HAL_TIM_Base_Start(&htim1);
HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
```

推荐放在 `OLED_Init();` 附近，当前工程就是这样写的：

```c
OLED_Init();
HAL_TIM_Base_Start(&htim1);
HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
```

三句分别是什么意思：

- `HAL_TIM_Base_Start(&htim1)`：启动基本计数器，让定时器开始计数
- `HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1)`：启动 `CH1` 输入捕获
- `HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2)`：启动 `CH2` 输入捕获，并开启中断回调

注意：

- 如果只写 `HAL_TIM_IC_Start()`，能捕获，但不会进中断回调
- 如果 `CH2` 不写 `_IT`，那么 `HAL_TIM_IC_CaptureCallback()` 大概率不会按你的预期执行

---

## 8. 主循环里如何触发超声波

当前工程主循环中的核心触发代码如下：

```c
HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
HAL_Delay(1);
HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);
__HAL_TIM_SET_COUNTER(&htim1, 0);
HAL_Delay(20);
```

### 8.1 每一句的作用

```c
HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
```

把 `Trig` 拉高，通知超声波模块开始一次测量。

```c
HAL_Delay(1);
```

延时一段时间，让触发脉冲保持高电平。

```c
HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);
```

把 `Trig` 拉低，结束触发脉冲。

```c
__HAL_TIM_SET_COUNTER(&htim1, 0);
```

把定时器计数器清零，方便这一次测量从 `0` 开始计数。

```c
HAL_Delay(20);
```

给模块留一点时间完成本次收发和回波处理。

---

## 9. 显示距离的代码

当前工程中后面把 `distance` 显示到 OLED：

```c
char message[20] = "";

sprintf(message, "距离: %.2fcm", distance);
OLED_PrintString(0, 0, message, &font16x16, OLED_COLOR_NORMAL);
```

如果你暂时不用 OLED，也可以先不显示，只要在调试器里观察 `distance` 的值是否变化就可以。

---

## 10. 程序整体流程串起来看

整套流程可以按下面顺序理解：

1. 初始化 GPIO、I2C、TIM1
2. 启动 OLED
3. 启动 `TIM1` 计数器和输入捕获
4. 主循环给 `Trig` 发送一个触发脉冲
5. 超声波模块把 `Echo` 拉高
6. `TIM1_CH1` 捕获上升沿，记录开始时间
7. 回波结束后 `Echo` 拉低
8. `TIM1_CH2` 捕获下降沿，进入回调函数
9. 在回调函数中读出 `CH1` 和 `CH2` 的捕获值
10. 计算高电平脉宽，换算成距离
11. 把距离显示到 OLED

---

## 11. 常见问题排查

### 11.1 距离始终是 0

优先按这个顺序检查：

#### 第一项：回调函数名字是不是写对了

要写：

```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
```

不要写成：

```c
HAL_TIM_IC_CaptureHalfCpltCallback
```

后者通常不是普通输入捕获中断要用的回调。

---

#### 第二项：有没有真正启动输入捕获

一定要检查这三句有没有执行：

```c
HAL_TIM_Base_Start(&htim1);
HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
```

如果没有启动，回调不会进，`distance` 就一直保持初值 `0`。

---

#### 第三项：`TIM1_CC_IRQn` 有没有打开

检查 `tim.c` 中是否有：

```c
HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
```

以及 `stm32l4xx_it.c` 中是否有：

```c
void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}
```

---

#### 第四项：`Echo` 引脚接线对不对

当前工程必须接到：

```c
PA8 -> TIM1_CH1
```

如果 `Echo` 接错脚，即使你的代码没问题，也不会有捕获值。

---

### 11.2 为什么 `HAL_TIM_ACTIVE_CHANNEL_2` 不是我手动赋值的

不是你在用户代码里赋值的。

当 `TIM1` 发生捕获中断时，HAL 库内部会在 `HAL_TIM_IRQHandler()` 里根据是哪个通道触发，自动设置：

- `HAL_TIM_ACTIVE_CHANNEL_1`
- `HAL_TIM_ACTIVE_CHANNEL_2`
- `HAL_TIM_ACTIVE_CHANNEL_3`
- `HAL_TIM_ACTIVE_CHANNEL_4`

所以你在回调函数里判断：

```c
htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2
```

本质上是在判断：这次是不是 `CH2` 触发的回调。

---

### 11.3 为什么上升沿用 `CH1`，下降沿用 `CH2`

因为这套配置是：

- `CH1`：直接抓上升沿
- `CH2`：间接抓下降沿

这样可以在下降沿到来时，一次性读到：

- `CH1` 记录的开始时间
- `CH2` 记录的结束时间

逻辑比较顺。

---

### 11.4 `sprintf("%.2f")` 会不会有问题

有些 STM32 工程默认不支持浮点格式化，这时：

```c
sprintf(message, "距离: %.2fcm", distance);
```

可能显示异常。

如果你遇到这个问题，需要在链接选项里开启 `printf float` 支持，比如加入：

```text
-u _printf_float
```

如果你的工程已经能正常显示带小数的浮点数，那这一项就不用再改。

---

### 11.5 `HAL_Delay(1)` 和标准 10us 触发脉冲不完全一样

很多超声波模块资料会写：

- 给 `Trig` 一个 **至少 10us** 的高电平脉冲

而 `HAL_Delay(1)` 的单位是 **ms**，也就是大约 `1000us`。

所以：

- 从“能不能触发模块”来说，`1ms` 一般是可以触发的
- 但从“是否严格标准”来说，它比 10us 长很多

如果后面你想写得更规范，可以把这段改成微秒级延时函数。但对于入门阶段，先让整套逻辑跑通更重要。

---

## 12. 可以直接参考的 `main.c` 核心代码

下面给出一份适合直接参考的核心片段：

```c
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include <stdio.h>
#include "oled.h"
#include "font.h"

int upedge = 0;
int downedge = 0;
float distance = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        upedge = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
        downedge = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_2);
        distance = (downedge - upedge) * 0.034 / 2;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM1_Init();

    OLED_Init();
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);

    char message[20] = "";

    while (1)
    {
        HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);

        __HAL_TIM_SET_COUNTER(&htim1, 0);
        HAL_Delay(20);

        sprintf(message, "距离: %.2fcm", distance);
        OLED_NewFrame();
        OLED_PrintString(0, 0, message, &font16x16, OLED_COLOR_NORMAL);
        OLED_ShowFrame();
    }
}
```

---

## 13. 学到这里你应该记住的 4 句话

- `Trig` 是输出，负责发起测量
- `Echo` 是输入，负责返回一个高电平脉冲
- 上升沿和下降沿的时间差，就是测距的时间基础
- 想让输入捕获回调生效，不仅要配好 `TIM1`，还要真正 `Start` 和 `Start_IT`

---

## 14. 结尾建议

如果你后面想继续优化这份程序，优先级建议如下：

1. 先确认 `distance` 能稳定变化
2. 再优化 `Trig` 的微秒级触发脉冲
3. 再处理计数器溢出、异常值过滤、平均值滤波
4. 最后再做更漂亮的 OLED 界面

对于初学阶段，先把“能测到距离”跑通，就是最重要的一步。
