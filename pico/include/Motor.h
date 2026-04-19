#pragma once
#include "SimpleFOC.h"

class Motor {
private:
    BLDCMotor* motor;
    BLDCDriver3PWM* driver;

public:
    // 构造函数：只需传入三相PWM引脚和使能引脚
    Motor(int phA, int phB, int phC, int en);
    
    void init();
    
    // 🌟 核心修改：统一改为速度控制接口
    void setTargetVelocity(float target_vel);
    
    // 实时控制核心接口
    void loopFOC();
    void move();
};