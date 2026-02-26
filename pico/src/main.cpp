#include "pico/stdlib.h"
#include <stdio.h>
#include "../include/Motor.h"
#include "../include/UART_Parser.h"

int main() {
    stdio_init_all();

    // 等待 USB 串口，方便监控
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("\n[SYSTEM] Project Migrated to UART1 (GP4/GP5)...\n");

    // 1. 初始化模拟电机对象
    Motor pitch_motor(2, 3, 4, 5);
    Motor yaw_motor(6, 7, 8, 9);
    UART_Parser uart_parser(&pitch_motor, &yaw_motor);

    // 2. 初始化 UART1 (使用你测试通过的 4 和 5 号引脚)
    // 强制指定 uart1，避开被占用的 uart0
    uart_parser.init(uart1, 4, 5, 115200);

    uint32_t last_heartbeat = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // --- 核心 A: 高频解析 (使用 uart1) ---
        uart_parser.spinOnce();

        // --- 核心 B: 心跳监视 ---
        if (now - last_heartbeat > 500) {
            printf("."); 
            fflush(stdout);
            last_heartbeat = now;
        }

        // --- 核心 C: 电机平滑控制 (目前仅做计算) ---
        pitch_motor.loopFOC();
        yaw_motor.loopFOC();
        
        // 维持控制频率
        sleep_ms(1);
    }
    return 0;
}