#include "../include/Motor.h"

// Constructor
Motor::Motor(int phA, int phB, int phC, int en) {
    driver = new BLDCDriver3PWM(phA, phB, phC, en);
    // The GM3506 gimbal motor typically has 11 pole pairs. In open-loop mode, the number of pole pairs must be specified.
    motor = new BLDCMotor(11); 
}

void Motor::init() {
    // 1. Driver board configuration (assuming you are using a 12V power battery supply)
    driver->voltage_power_supply = 12.0f; 
    driver->init();
    motor->linkDriver(driver);

    // 2.Core: open-loop safety limits (must never be removed!)
    motor->voltage_limit = 5.0f;   // Limit the maximum phase voltage to 3V to prevent overheating and burnout in open-loop operation
    motor->velocity_limit = 20.0f; // Limit the maximum rotational speed to 20 rad/s

    // 3. Configure for open-loop velocity control mode
    motor->controller = MotionControlType::velocity_openloop;

    // 4. Bypass the sensor and initialize directly
    motor->init();
    
    // Set the initial velocity to 0
    motor->target = 0.0f;
}

void Motor::setTargetVelocity(float target_vel) {
    motor->target = target_vel;
}

void Motor::loopFOC() {
    // In open-loop mode, loopFOC is effectively empty, but it is kept to maintain architectural consistency
    motor->loopFOC();
}

void Motor::move() {
    // Open-loop core: forcibly output the three-phase sine wave purely based on microsecond-level timestamps
    motor->move();
}
