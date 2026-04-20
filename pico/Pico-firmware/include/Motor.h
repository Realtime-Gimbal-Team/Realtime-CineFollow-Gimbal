#pragma once
#include "SimpleFOC.h"

class Motor {
private:
    BLDCMotor* motor;
    BLDCDriver3PWM* driver;

public:
    // Constructor: only requires the three-phase PWM pins and the enable pin
    Motor(int phA, int phB, int phC, int en);
    
    void init();
    
    // 🌟 Core modification: unify everything under the velocity control interface
    void setTargetVelocity(float target_vel);
    
    // Real-time control core interface
    void loopFOC();
    void move();
};
