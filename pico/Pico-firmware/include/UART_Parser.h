#pragma once
#include "pico/stdlib.h"
#include "hardware/uart.h"

// Completely remove the forward declaration of class Motor

enum ParserState { WAIT_HEAD1, WAIT_HEAD2, WAIT_CMD, WAIT_LEN, STATE_DATA, WAIT_CKSM, WAIT_TAIL };

class UART_Parser {
private:
    // Completely remove the pointer members for pitch_motor and yaw_motor
    uart_inst_t* uart_port;

    ParserState currentState = WAIT_HEAD1;
    uint8_t current_cmd = 0;
    uint8_t target_len = 0;
    uint8_t data_idx = 0;
    uint8_t data_buffer[16];

    // Timestamp for the communication safety watchdog
    volatile uint32_t last_valid_rx_time; 

    void parseByte(uint8_t b);

public:
    // The constructor no longer requires any parameters
    UART_Parser(); 
    void init(uart_inst_t* uart, uint tx_pin, uint rx_pin, uint baud);
    void handle_rx_irq();

    uint32_t getLastRxTime();
};
