#pragma once

class Motor {
private:
    float current_angle;
    float target_angle;
    int pin_in1, pin_in2, pin_in3, pin_en;

public:
    Motor(int in1, int in2, int in3, int en);
    void init();
    void setTargetAngle(float angle);
    float getCurrentAngle();
    void loopFOC();
};