#include "pico/stdlib.h"
#include <stdio.h>
#include "Motor.h"
#include "UART_Parser.h"

int main() {
    stdio_init_all();

    // 1. 创建两个电机本体 (烧录引脚记忆)
    Motor pitch_motor(0, 1, 2, 3);
    Motor yaw_motor(4, 5, 6, 7);

    // 2. 厂长训话初始化
    pitch_motor.init();
    yaw_motor.init();

    // 3. 聘用前台秘书，并把电机的遥控器（指针）交给他
    UART_Parser uart_parser(&pitch_motor, &yaw_motor);

    uint32_t last_telemetry_time = 0; 

    // 4. 进入永不阻塞的终极循环
    while (true) {
        // 秘书光速查收串口快递
        uart_parser.spinOnce();

        // 厨师疯狂颠勺运算 FOC
        pitch_motor.loopFOC();
        yaw_motor.loopFOC();

        // 每隔 20 毫秒，向树莓派汇报一次当前阵地情况 (50Hz)
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_telemetry_time >= 20) { 
            uart_parser.sendTelemetry(12.0f, 0x02);
            last_telemetry_time = current_time; 
        }
    }
    return 0;
}