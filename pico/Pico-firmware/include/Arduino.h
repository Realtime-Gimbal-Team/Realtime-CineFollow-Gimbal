#pragma once
 
// C/C++ Standard library supplement
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>

// Pico SDK low-level layer
#include "pico/stdlib.h"


// 1. Math library and constant mapping

using std::min;
using std::max;
using std::abs;

inline float fabsf(float x) { return ::fabsf(x); }

#ifndef constrain
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#endif

// 🚨 Never define _PI, _2PI, or NOT_SET here — leave them to be managed by the SimpleFOC source code!

#define isDigit(c) isdigit((unsigned char)(c))


// 2. Time function mapping

inline void delay(uint32_t ms) { sleep_ms(ms); }
inline void delayMicroseconds(uint32_t us) { sleep_us(us); }
inline uint32_t micros() { return time_us_32(); }
inline uint32_t millis() { return to_ms_since_boot(get_absolute_time()); }


// 3. GPIO and interrupt black hole (suppress useless driver errors)

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


// 4. SPI & I2C hardware black hole (suppress internal magnetic encoder errors)

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


// 5.  Completely remove Arduino Print/Stream dependencies

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
