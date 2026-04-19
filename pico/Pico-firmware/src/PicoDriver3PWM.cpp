#include <SimpleFOC.h>
#include <stdio.h> // 🌟 Add the standard input/output header files to ensure printf syntax is fully safe
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

// Struct for caching pins and count values
struct Pico3PWMParams {
    int pinA;
    int pinB;
    int pinC;
    uint32_t wrap; 
};

void* _configure3PWM(long pwm_frequency, const int pinA, const int pinB, const int pinC) {
    if(pinA == NOT_SET || pinB == NOT_SET || pinC == NOT_SET) return nullptr;

    int pins[3] = {pinA, pinB, pinC};
    uint32_t sys_clk = clock_get_hz(clk_sys);
    
    // If no frequency is specified, set the default to 25 kHz
    if(pwm_frequency == 0 || pwm_frequency == NOT_SET) pwm_frequency = 25000;
    
    uint32_t wrap = sys_clk / pwm_frequency;
    if (wrap > 65535) wrap = 65535; 

    // Configure the Pico native PWM
    for (int i = 0; i < 3; i++) {
        gpio_set_function(pins[i], GPIO_FUNC_PWM);
        uint slice_num = pwm_gpio_to_slice_num(pins[i]);
        pwm_set_wrap(slice_num, wrap);
        pwm_set_clkdiv(slice_num, 1.0f);
        pwm_set_enabled(slice_num, true);
    }

    // Allocate memory to store the pins and count values, then return it to the driver layer
    Pico3PWMParams* params = new Pico3PWMParams();
    params->pinA = pinA;
    params->pinB = pinB;
    params->pinC = pinC;
    params->wrap = wrap; // Cache the count value to avoid hardware pointer pitfalls
    return params;
}

void _writeDutyCycle3PWM(float dc_a, float dc_b, float dc_c, void* params) {
    if(params == nullptr) return;
    Pico3PWMParams* p = (Pico3PWMParams*)params;

    // 🌟  Ultimate scout (tripwire): directly intercept the final duty cycle (0.0 ~ 1.0) computed by the SimpleFOC core
    // Reduce the print frequency to prevent the serial port from being saturated and causing motor stuttering
    static int count = 0;
    

    // Completely abandon dangerous hardware pointers and use the cached p->wrap directly for safe conversion!
    pwm_set_gpio_level(p->pinA, (uint16_t)(dc_a * p->wrap));
    pwm_set_gpio_level(p->pinB, (uint16_t)(dc_b * p->wrap));
    pwm_set_gpio_level(p->pinC, (uint16_t)(dc_c * p->wrap));
}
