#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mqtt_handler.h"
#include "lora_module.h" 

const char* mqtt_server = "test.mosquitto.org";
const char* cmd_topic   = "k9ops/trainer/cmd"; 

WiFiClient espClient;
PubSubClient client(espClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("[MQTT RX]: ");
  Serial.println(message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println("JSON Parse Failed");
    return;
  }

  const char* target = doc["target"];
  if (strcmp(target, "Dog") != 0) return; 

  const char* cmd = doc["command"]; // Check if your dashboard sends "command" or "cmd"

  // 1. Original BUZZER (Manual On/Off)
  if (strcmp(cmd, "buzzer") == 0) {
      int val = doc["value"];
      loraSendBuzzerCommand(val);
  }
  // 2. NEW VIBRATION (Patterns 1, 2, 3)
  else if (strcmp(cmd, "vibration") == 0) {
      int pattern = doc["value"];
      loraSendVibrationCommand(pattern);
  }
  // 3. Original LED 
  else if (strcmp(cmd, "led") == 0) {
      JsonObject val = doc["value"];
      int m = val["mode"];
      int c = val["color"];
      int b = val["brightness"];
      loraSendLedCommand(m, c, b);
  }
}

void mqttInit() {
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback);
}

void mqttLoop() {
    if (!client.connected()) {
        Serial.print("[MQTT] Connecting...");
        if (client.connect("TrainerESP32_Client")) {
            Serial.println(" connected");
            client.subscribe(cmd_topic);
        } else {
            delay(5000);
        }
    }
    client.loop();
}