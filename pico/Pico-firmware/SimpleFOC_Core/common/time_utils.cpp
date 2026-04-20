#include "time_utils.h"
#include "pico/stdlib.h" // 🌟 Must explicitly include the native Pico SDK libraries

// ---------------------------------------------------------
// Forcefully take over the delay function and completely abandon Arduino's delay()
// ---------------------------------------------------------
void _delay(unsigned long ms) {
    // Directly call the Pico SDK's native non-blocking millisecond delay
    sleep_ms(ms);
}

// ---------------------------------------------------------
//  Forcefully take over the timestamp function and completely eliminate time freezing!
// ---------------------------------------------------------
unsigned long _micros() {
    //  Directly read the RP2350's low-level 32-bit hardware timer
    // Its return type, uint32_t, matches unsigned long perfectly and ensures absolute precision
    return time_us_32();
}
