#ifndef MQTT_h
#define MQTT_h

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <secrets.hpp>

#define MQTT_PORT 1883

// Topics for publishing and subscribing
extern const char* publish_topic;
extern const char* subscribe_topic;
extern PubSubClient MQTTclient;

void init_MQTT();
void MQTT_callback(char* topic, byte* message, unsigned int length);
void setup_wifi();
void reconnect();
void publish_message(const char* message);



#endif