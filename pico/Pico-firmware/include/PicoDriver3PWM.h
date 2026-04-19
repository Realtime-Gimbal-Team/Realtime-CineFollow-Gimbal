#ifndef PICO_DRIVER_3PWM_H
#define PICO_DRIVER_3PWM_H
#include "common/base_classes/BLDCDriver.h"
class PicoDriver3PWM : public BLDCDriver {
public:
    PicoDriver3PWM(int phA, int phB, int phC, int en = NOT_SET);
    int init() override;
    void enable() override;
    void disable() override;
    void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) override;
    void setPwm(float ua, float ub, float uc) override;
private:
    int pwmA, pwmB, pwmC, enable_pin;
    unsigned int sliceA, sliceB, sliceC;
    unsigned int chanA, chanB, chanC;
    uint32_t pwm_top;
};
#endif