# Team 16: 2轴实时智能跟随云台 - UART通信协议文档

> **文档版本:** V1.1 (完整版)
> **更新日期:** 2026年2月14日
> **拟定人:** 组员一 (视觉大脑 RPi 5) & 组员二 (底层脊髓 Pico)
> **项目目标:** 建立满足硬实时要求的高频串行通信，支持 FOC 控制与系统状态遥测。

---

## 1. 物理层通信标准 (Physical Layer)

* **通信接口:** UART (Universal Asynchronous Receiver-Transmitter)
* **波特率 (Baud Rate):** 115200 *(初期调试使用，后期如需降低延迟可提升至 921600)*
* **数据格式:** 8 数据位 (Data bits)，无校验 (No parity)，1 停止位 (1 Stop bit) -> 即 **8N1**
* **字节序 (Endianness):** 小端序 (Little-Endian) - *树莓派与 Pico 的 ARM 架构默认标准*

---

## 2. 标准数据帧结构 (Standard Frame Structure)

为了防止数据在连续传输中发生“粘包”或错位，所有指令均采用以下定长/变长混合的帧格式包装。

| 字节位置 | 字段名称 | 数据类型 | 长度(Byte) | 字段说明 |
| :--- | :--- | :--- | :--- | :--- |
| `0` | Header 1 | `uint8_t` | 1 | 帧头 1：固定为 `0x55` |
| `1` | Header 2 | `uint8_t` | 1 | 帧头 2：固定为 `0xAA` |
| `2` | CMD_ID | `uint8_t` | 1 | 指令功能码（详见第3节） |
| `3` | Data_Len | `uint8_t` | 1 | Payload 的有效数据长度 (`N`) |
| `4` ~ `3+N` | Payload | `Bytes` | N | 具体的数据内容（如浮点数） |
| `4+N` | Checksum | `uint8_t` | 1 | **校验和 (Checksum)**：`CMD_ID` + `Data_Len` + `Payload` 所有字节的累加和的低 8 位 |
| `5+N` | Tail | `uint8_t` | 1 | 帧尾：固定为 `0x0D` (`\r`) |

**总帧长度 :** `6 + N` 字节

---

## 3. 指令集详细定义 (Command Set)

### 3.1 下发指令 (RPi 5 发送至 Pico)

#### [CMD_ID: 0x01] 设定目标角度 (Set Target Angle)
* **功能描述:** RPi 5 下发视觉追踪算法计算出的绝对目标角度。
* **Data_Len:** 8
* **Payload 结构:**
  * `Byte 0~3`: `float` Pitch_Angle (俯仰角，单位：度)
  * `Byte 4~7`: `float` Yaw_Angle (偏航角，单位：度)

#### [CMD_ID: 0x02] 设定云台工作模式 (Set System Mode)
* **功能描述:** 切换 Pico 的底层控制逻辑，应对视觉目标丢失等突发情况。
* **Data_Len:** 1
* **Payload 结构:**
  * `Byte 0`: `uint8_t` Mode_ID
    * `0x00`: IDLE (放松模式，电机断电，方便手动调节)
    * `0x01`: TRACKING (正常跟随模式，响应速度快)
    * `0x02`: LOST_SEARCH (目标丢失模式，Pico 执行缓动或保持当前位置)
    * `0x03`: LOCKED (锁定模式，强力维持当前角度)

#### [CMD_ID: 0x03] 动态 PID 调参 (Tune Control Params)
* **功能描述:** 允许在系统运行时动态修改底层 PID 参数，无需重新烧录 Pico 固件。
* **Data_Len:** 12
* **Payload 结构:**
  * `Byte 0~3`: `float` Angle_P (位置环 P 增益)
  * `Byte 4~7`: `float` Angle_I (位置环 I 增益)
  * `Byte 8~11`: `float` Velocity_Limit (云台最大允许转速限制)

### 3.2 上报指令 (Pico 发送至 RPi 5)

#### [CMD_ID: 0x10] 高频状态遥测 (High-Freq Telemetry)
* **功能描述:** Pico 向上位机高频回传当前物理状态，供 OLED 屏幕显示及系统内存监控模块评估延迟使用。
* **Data_Len:** 13
* **Payload 结构:**
  * `Byte 0~3`: `float` Current_Pitch (IMU 或编码器反馈的当前俯仰角)
  * `Byte 4~7`: `float` Current_Yaw (当前偏航角)
  * `Byte 8~11`: `float` Battery_Voltage (当前电池电压，防止过放)
  * `Byte 12`: `uint8_t` Status_Flags (系统状态掩码)
    * `Bit 0`: 1 = 已到达目标角度，0 = 正在移动
    * `Bit 1`: 1 = 电机已使能 (Enabled)，0 = 电机放松
    * `Bit 2`: 1 = 驱动板过热警告 (Overheat)，0 = 温度正常

---

## 4. 故障保护与异常处理机制 (Failsafe Mechanisms)

为满足项目对系统可靠性的要求，通信层需包含以下安全机制：

1. **通信超时保护 (Watchdog):** 如果 Pico 连续 500ms 未收到任何来自 RPi 5 的有效指令帧，必须强制将系统模式切换至 IDLE (`0x00`) 或 LOCKED (`0x03`)，以防止云台失控疯转。
2. **校验和丢包机制:** 接收端每次读取到帧尾 `0x0D` 后，必须计算 Payload 的 Checksum。若计算值与接收到的 Checksum 不符，该帧数据必须被直接丢弃，不执行任何操作。

---

## 5. C++ 结构体代码参考 (供双方直接复制使用)

此部分代码强制采用单字节对齐，以防不同编译器自动填充导致数据错位。为了符合 C++ OOP 规范，建议将其保存为 `protocol_def.h` 头文件供两端项目共用。

```cpp
#pragma once
#include <stdint.h>

// 强制取消编译器结构体对齐优化
#pragma pack(push, 1)

// 基础帧头定义
struct FrameHeader {
    uint8_t head1 = 0x55;
    uint8_t head2 = 0xAA;
    uint8_t cmd_id;
    uint8_t data_len;
};

// ============ Payload 结构体定义 ============

// CMD 0x01: 目标角度数据包
struct PayloadSetAngle {
    float pitch;
    float yaw;
};

// CMD 0x03: 动态PID调参包
struct PayloadSetPID {
    float p_gain;
    float i_gain;
    float v_limit;
};

// CMD 0x10: 状态遥测包
struct PayloadTelemetry {
    float current_pitch;
    float current_yaw;
    float battery_voltage;
    uint8_t status_flags;
};

// ============ 完整数据帧示例 ============

// 下发目标角度的完整帧
struct FrameSetAngle {
    FrameHeader header;
    PayloadSetAngle payload;
    uint8_t checksum;
    uint8_t tail = 0x0D;
};

// 恢复编译器默认对齐方式
#pragma pack(pop)
