#include <SimpleFOC.h>
#include <stdio.h> // 🌟 补充标准输入输出头文件，确保 printf 语法绝对安全
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

// 缓存引脚与计数值的结构体
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
    
    // 若未指定频率，默认设为 25kHz
    if(pwm_frequency == 0 || pwm_frequency == NOT_SET) pwm_frequency = 25000;
    
    uint32_t wrap = sys_clk / pwm_frequency;
    if (wrap > 65535) wrap = 65535; // Pico PWM 计数器是 16 位的

    // 配置 Pico 原生 PWM
    for (int i = 0; i < 3; i++) {
        gpio_set_function(pins[i], GPIO_FUNC_PWM);
        uint slice_num = pwm_gpio_to_slice_num(pins[i]);
        pwm_set_wrap(slice_num, wrap);
        pwm_set_clkdiv(slice_num, 1.0f);
        pwm_set_enabled(slice_num, true);
    }

    // 分配内存保存引脚和计数值，返回给驱动层
    Pico3PWMParams* params = new Pico3PWMParams();
    params->pinA = pinA;
    params->pinB = pinB;
    params->pinC = pinC;
    params->wrap = wrap; // 缓存计数值，避开硬件指针陷阱

    return params;
}

void _writeDutyCycle3PWM(float dc_a, float dc_b, float dc_c, void* params) {
    if(params == nullptr) return;
    Pico3PWMParams* p = (Pico3PWMParams*)params;

    // 🌟 终极侦察兵（绊线）：直接拦截 SimpleFOC 核心算出来的最终占空比 (0.0 ~ 1.0)
    // 降频打印，防止占满串口导致电机运行卡顿
    static int count = 0;
    

    // 彻底抛弃危险的硬件指针，直接用缓存好的 p->wrap 进行安全换算！
    pwm_set_gpio_level(p->pinA, (uint16_t)(dc_a * p->wrap));
    pwm_set_gpio_level(p->pinB, (uint16_t)(dc_b * p->wrap));
    pwm_set_gpio_level(p->pinC, (uint16_t)(dc_c * p->wrap));
}