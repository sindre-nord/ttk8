/*
Serves as an interface to the ASS_reader_node
Provides conveinience.
*/
#include <Arduino.h>
#include "tag_finder.hpp"
#include "CAN.h"
#include "esp32-hal.h"

// CAN interrupt handler
volatile bool CANMessageReceived = false;
// Needs to take in an int due to the onReceive expecting
// you to handle the message as well (and read the bytes).
void IRAM_ATTR onCANReceive(int messageSize) {
  CANMessageReceived = true;  // Set flag when CAN message is received
}

/**
 * @brief Needs to be setup once to initiate CAN
 * 
 * @return uint8_t Return 0 on success, 1 on failure.
 */
uint8_t setupCAN(){
  CAN.onReceive(onCANReceive); //Binds the interrupt.
  CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
  if(!CAN.begin(CAN_BAUD)) {return 1;}
  else {return 0;}
}

void sendCanMessage(uint32_t id, const char *message) {
  CAN.beginPacket(id);
  while (*message) {
      CAN.write(*message++);
  }
  CAN.endPacket();
}
void sendCANcommand(CANcommand command){
  CAN.beginPacket(command); // Command is implied through ID
  CAN.write('A'); // Dummy byte to satisfy the CAN library
  CAN.endPacket();
}

// We either get an ACK
void readCanMessage(){
  
}


