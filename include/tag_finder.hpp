#ifndef TAG_FINDER_HPP
#define TAG_FINDER_HPP

#include <Arduino.h>
#include <CAN.h>

extern volatile bool CANMessageReceived;

// Setup CAN
#define CAN_TX_PIN 32
#define CAN_RX_PIN 33
#define CAN_BAUD 10E3

uint8_t setupCAN();
void sendCanMessage(uint32_t id, const char *message);
void readCanMessage(int messageSize);

enum CANcommand {
    START = 0x101,
    STOP  = 0x102,
    ACK   = 0x103,
    UID   = 0x104,
};
void sendCANcommand(CANcommand command);

#endif