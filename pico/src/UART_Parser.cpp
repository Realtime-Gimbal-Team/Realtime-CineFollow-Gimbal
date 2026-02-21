#include "../include/UART_Parser.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

UART_Parser::UART_Parser(Motor* pitch, Motor* yaw) {
    pitch_motor = pitch;
    yaw_motor = yaw;
    current_state = WAIT_HEAD1;
}

void UART_Parser::spinOnce() {
    int c = getchar_timeout_us(0); // 绝对不阻塞的灵魂！
    while (c != PICO_ERROR_TIMEOUT) {
        parseByte((uint8_t)c);     
        c = getchar_timeout_us(0); 
    }
}

void UART_Parser::parseByte(uint8_t b) {
    switch (current_state) {
        case WAIT_HEAD1:
            if (b == 0x55) current_state = WAIT_HEAD2;
            break;
            
        case WAIT_HEAD2:
            if (b == 0xAA) current_state = WAIT_CMD;
            else current_state = WAIT_HEAD1; 
            break;
            
        case WAIT_CMD:
            cmd_id = b;
            calculated_checksum = b;        
            current_state = WAIT_LEN;
            break;
            
        case WAIT_LEN:
            data_len = b;
            calculated_checksum += b;
            payload_idx = 0;
            if (data_len > sizeof(payload_buffer)) current_state = WAIT_HEAD1; 
            else if (data_len > 0) current_state = WAIT_PAYLOAD;
            else current_state = WAIT_CHECKSUM;
            break;
            
        case WAIT_PAYLOAD:
            payload_buffer[payload_idx++] = b;
            calculated_checksum += b;
            if (payload_idx >= data_len) current_state = WAIT_CHECKSUM;
            break;
            
        case WAIT_CHECKSUM:
            if (b == calculated_checksum) current_state = WAIT_TAIL;
            else current_state = WAIT_HEAD1; // 校验失败，无情反杀！
            break;
            
        case WAIT_TAIL:
            if (b == 0x0D) executeCommand(); 
            current_state = WAIT_HEAD1;      
            break;
    }
}

void UART_Parser::executeCommand() {
    if (cmd_id == 0x01 && data_len == sizeof(PayloadSetAngle)) {
        PayloadSetAngle angles;
        memcpy(&angles, payload_buffer, sizeof(PayloadSetAngle)); 
        
        pitch_motor->setTargetAngle(angles.pitch);
        yaw_motor->setTargetAngle(angles.yaw);
        
        printf("【指令下达】目标 Pitch: %.1f, Yaw: %.1f\n", angles.pitch, angles.yaw);
    }
}

void UART_Parser::sendTelemetry(float battery_voltage, uint8_t status_flags) {
    uint8_t buffer[19]; 
    
    buffer[0] = 0x55; buffer[1] = 0xAA;
    buffer[2] = 0x10; buffer[3] = sizeof(PayloadTelemetry); 

    PayloadTelemetry payload;
    payload.current_pitch = pitch_motor->getCurrentAngle();
    payload.current_yaw = yaw_motor->getCurrentAngle();
    payload.battery_voltage = battery_voltage;
    payload.status_flags = status_flags;

    memcpy(&buffer[4], &payload, sizeof(PayloadTelemetry));

    uint8_t checksum = buffer[2] + buffer[3];
    for(int i = 0; i < sizeof(PayloadTelemetry); i++) {
        checksum += buffer[4 + i];
    }
    buffer[17] = checksum; 
    buffer[18] = 0x0D;     

    for(int i = 0; i < 19; i++) {
        putchar(buffer[i]);
    }
}