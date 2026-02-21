#include "../include/Motor.h"
#include <stdio.h>

Motor::Motor(int in1, int in2, int in3, int en) {
    pin_in1 = in1; pin_in2 = in2; pin_in3 = in3; pin_en = en;
    current_angle = 0.0f; target_angle = 0.0f;
}

void Motor::init() {
    printf("【系统提示】电机已初始化！引脚: %d,%d,%d, EN:%d\n", pin_in1, pin_in2, pin_in3, pin_en);
}

void Motor::setTargetAngle(float angle) {
    // 严密的安全保护机制：限制死角防排线扯断
    if (angle > 45.0f) {
        target_angle = 45.0f; 
        printf("【警告】角度 %.1f 过大！截断至 45.0 度\n", angle);
    } else if (angle < -45.0f) {
        target_angle = -45.0f;
        printf("【警告】角度 %.1f 过小！截断至 -45.0 度\n", angle);
    } else {
        target_angle = angle;
    }
}

float Motor::getCurrentAngle() { return current_angle; }

void Motor::loopFOC() {
    // 纯软件模拟硬件运转逻辑
    if (current_angle < target_angle) current_angle += 0.5f; 
    else if (current_angle > target_angle) current_angle -= 0.5f;
}