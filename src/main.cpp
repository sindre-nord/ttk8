#include <Arduino.h>
#include "FSM.hpp"

void setup() {
    // Initialization code
    Serial.begin(115200);
    Serial.println("System initializing...");
}

void loop() {
    run_FSM();
}