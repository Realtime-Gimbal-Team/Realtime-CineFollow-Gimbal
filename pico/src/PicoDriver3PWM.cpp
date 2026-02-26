#include "PicoDriver3PWM.h"
#include "hardware/pwm.h"
#include "Arduino.h"

PicoDriver3PWM::PicoDriver3PWM(int pinA, int pinB, int pinC, int en) {
    pwmA = pinA; pwmB = pinB; pwmC = pinC; enable_pin = en;
}

int PicoDriver3PWM::init() {
    gpio_set_function(pwmA, GPIO_FUNC_PWM);
    gpio_set_function(pwmB, GPIO_FUNC_PWM);
    gpio_set_function(pwmC, GPIO_FUNC_PWM);
    sliceA = pwm_gpio_to_slice_num(pwmA);
    sliceB = pwm_gpio_to_slice_num(pwmB);
    sliceC = pwm_gpio_to_slice_num(pwmC);
    chanA = pwm_gpio_to_channel(pwmA);
    chanB = pwm_gpio_to_channel(pwmB);
    chanC = pwm_gpio_to_channel(pwmC);
    pwm_top = 5208;
    pwm_set_wrap(sliceA, pwm_top);
    pwm_set_wrap(sliceB, pwm_top);
    pwm_set_wrap(sliceC, pwm_top);
    pwm_set_enabled(sliceA, true);
    pwm_set_enabled(sliceB, true);
    pwm_set_enabled(sliceC, true);
    if (enable_pin != NOT_SET) {
        gpio_init(enable_pin);
        gpio_set_dir(enable_pin, GPIO_OUT);
    }
    return 1;
}

void PicoDriver3PWM::setPwm(float Ua, float Ub, float Uc) {
    uint32_t dutyA = (uint32_t)((constrain(Ua, 0, voltage_limit) / voltage_power_supply) * pwm_top);
    uint32_t dutyB = (uint32_t)((constrain(Ub, 0, voltage_limit) / voltage_power_supply) * pwm_top);
    uint32_t dutyC = (uint32_t)((constrain(Uc, 0, voltage_limit) / voltage_power_supply) * pwm_top);
    pwm_set_chan_level(sliceA, chanA, dutyA);
    pwm_set_chan_level(sliceB, chanB, dutyB);
    pwm_set_chan_level(sliceC, chanC, dutyC);
}

void PicoDriver3PWM::setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) {}
void PicoDriver3PWM::enable() { if (enable_pin != NOT_SET) gpio_put(enable_pin, 1); }
void PicoDriver3PWM::disable() { if (enable_pin != NOT_SET) gpio_put(enable_pin, 0); }