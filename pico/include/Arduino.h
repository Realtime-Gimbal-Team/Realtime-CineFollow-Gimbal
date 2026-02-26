#pragma once

// C/C++ 标准库填补
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>

// Pico SDK 底层
#include "pico/stdlib.h"

// ==========================================
// 1. 数学库与常量映射 
// ==========================================
using std::min;
using std::max;
using std::abs;

inline float fabsf(float x) { return ::fabsf(x); }

#ifndef constrain
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#endif

#ifndef _PI
#define _PI 3.14159265358979323846f
#endif
#ifndef _2PI
#define _2PI 6.28318530717958647692f
#endif

#define isDigit(c) isdigit((unsigned char)(c))

// ==========================================
// 2. 时间函数映射 
// ==========================================
inline void delay(uint32_t ms) { sleep_ms(ms); }
inline void delayMicroseconds(uint32_t us) { sleep_us(us); }
inline uint32_t micros() { return time_us_32(); }
inline uint32_t millis() { return to_ms_since_boot(get_absolute_time()); }

// ==========================================
// 3. GPIO 与 中断黑洞 (屏蔽无用驱动报错)
// ==========================================
#define NOT_SET -1
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#define HIGH 0x1
#define LOW  0x0

#define RISING 0x01
#define FALLING 0x02
#define CHANGE 0x03

#define word uint16_t
#define byte uint8_t

inline void pinMode(int pin, int mode) {}
inline void digitalWrite(int pin, int val) {}
inline int digitalRead(int pin) { return 0; }
inline int analogRead(int pin) { return 0; }
inline unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) { return 0; }

inline int digitalPinToInterrupt(int pin) { return pin; }
inline void attachInterrupt(int pin, void (*userFunc)(), int mode) {}
inline void interrupts() {}
inline void noInterrupts() {}

// ==========================================
// 4. SPI & I2C 硬件黑洞 (屏蔽磁编码器报错)
// ==========================================
#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define MSBFIRST 1
#define LSBFIRST 0

class SPISettings {
public:
    SPISettings() {}
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {}
};

class SPIClass {
public:
    void begin() {}
    void end() {}
    void beginTransaction(SPISettings settings) {}
    void endTransaction() {}
    uint16_t transfer16(uint16_t data) { return 0; }
    uint8_t transfer(uint8_t data) { return 0; }
};
extern SPIClass SPI;

class TwoWire {
public:
    void begin() {}
    void beginTransmission(uint8_t) {}
    int endTransmission(bool stopBit = true) { return 0; }
    void requestFrom(uint8_t, uint8_t) {}
    uint8_t read() { return 0; }
    void write(uint8_t) {}
};
extern TwoWire Wire;

// ==========================================
// 5. 彻底屏蔽 Arduino 的 Print/Stream 依赖 
// ==========================================
class __FlashStringHelper;
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))

class StringSumHelper {
public:
    const char* c_str() const { return ""; }
};

class Print {
public:
    virtual void print(const char* s) {}
    virtual void print(int val) {}
    virtual void print(float val, int dec=2) {}
    virtual void print(const __FlashStringHelper* s) {}
    virtual void println(const char* s) {}
    virtual void println(int val) {}
    virtual void println(float val, int dec=2) {}
    virtual void println(const __FlashStringHelper* s) {}
    virtual void println() {}
};

class Stream : public Print {
public:
    virtual int available() { return 0; }
    virtual int read() { return -1; }
};

extern Print Serial;