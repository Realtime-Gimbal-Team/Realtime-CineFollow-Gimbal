#pragma once
#include "protocol_def.h"
#include "Motor.h"
#include <stdint.h>

class UART_Parser {
private:
    enum State {
        WAIT_HEAD1, WAIT_HEAD2, WAIT_CMD, WAIT_LEN, 
        WAIT_PAYLOAD, WAIT_CHECKSUM, WAIT_TAIL
    };

    State current_state;           
    uint8_t cmd_id;                
    uint8_t data_len;              
    uint8_t payload_buffer[32];    
    uint8_t payload_idx;           
    uint8_t calculated_checksum;   

    Motor* pitch_motor;            
    Motor* yaw_motor;              

    void parseByte(uint8_t byte);  
    void executeCommand();         

public:
    UART_Parser(Motor* pitch, Motor* yaw);
    void spinOnce();               
    void sendTelemetry(float battery_voltage, uint8_t status_flags);
};