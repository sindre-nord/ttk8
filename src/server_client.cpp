#include <Arduino.h>
#include "MQTT.hpp"
#include "FSM.hpp"

void init_server_client(){
    setup_wifi();
    init_MQTT();
    reconnect();
    return;
}



