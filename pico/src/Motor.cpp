#include "../include/Motor.h"

// 构造函数
Motor::Motor(int phA, int phB, int phC, int en) {
    driver = new BLDCDriver3PWM(phA, phB, phC, en);
    // GM3506 云台电机通常是 11 对极。开环模式下必须指定对极数。
    motor = new BLDCMotor(11); 
}

void Motor::init() {
    // 1. 驱动板配置 (假设你用的是 12V 动力电池供电)
    driver->voltage_power_supply = 12.0f; 
    driver->init();
    motor->linkDriver(driver);

    // 2. 核心：开环安全限制（绝不可删！）
    motor->voltage_limit = 5.0f;   // 限制最大相电压为 3V，防止开环过热烧毁
    motor->velocity_limit = 20.0f; // 限制最大转速为 20 rad/s

    // 3. 配置为开环速度控制模式
    motor->controller = MotionControlType::velocity_openloop;

    // 4. 绕过传感器，直接初始化
    motor->init();
    
    // 初始速度设为 0
    motor->target = 0.0f;
}

void Motor::setTargetVelocity(float target_vel) {
    motor->target = target_vel;
}

void Motor::loopFOC() {
    // 开环模式下 loopFOC 实际上为空，保留以维持架构统一
    motor->loopFOC();
}

void Motor::move() {
    // 开环核心：纯靠微秒级时间戳强行输出三相正弦波
    motor->move();
}