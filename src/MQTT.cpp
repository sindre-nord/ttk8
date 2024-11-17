#include <WiFi.h>
#include <PubSubClient.h>
#include <MQTT.hpp>
#include <secrets.hpp>
#include <FSM.hpp>

const char* publish_topic = "home/esp32/publish";
const char* subscribe_topic = "home/esp32/subscribe";

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

// Callback function to handle incoming messages
void MQTT_callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);
  
  // Handle message (example: if you receive "ON" or "OFF" commands)
  if (messageTemp == "ON") {
    Serial.println("Turning on something...");
  } else if (messageTemp == "OFF") {
    Serial.println("Turning off something...");
  }
  push_event_to_queue({SHELF_REQUESTED, messageTemp});
}

// Function to connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to connect to MQTT broker with username and password
void reconnect() {
  while (!MQTTclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a MQTTclient ID based on the ESP32's MAC address
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect with username and password
    if (MQTTclient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to a topic
      MQTTclient.subscribe(subscribe_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void init_MQTT() {
  MQTTclient.setServer(mqtt_server, MQTT_PORT);
  MQTTclient.setCallback(MQTT_callback);
}

void publish_message(const char* message) {
  MQTTclient.publish(publish_topic, message);
}

