#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "buzzer.h"
#include "led_function.h"

// ==========================================
// ⚠️ ENTER YOUR WI-FI CREDENTIALS HERE ⚠️
// ==========================================
const char* ssid = "Iphone 19 pro max"; 
const char* pass = "00009999";

// 🟢 CRITICAL: Switch to EMQX Broker
const char* mqtt_server = "broker.emqx.io";
const char* topic_sub   = "k9ops/trainer/cmd"; 

WiFiClient espClient;
PubSubClient client(espClient);

void handleDirectMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("[Direct MQTT]: "); Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println("JSON Error");
    return;
  }

  const char* target = doc["target"];
  if (strcmp(target, "Dog") != 0) return;

  const char* cmd = doc["command"]; 

  // 1. BUZZER / VIBRATION
  if (strcmp(cmd, "vibration") == 0) {
      int pattern = doc["value"];
      Serial.print("Triggering Vibration: "); Serial.println(pattern);
      playBuzzerPattern(pattern);
  }
  // 2. LED
  else if (strcmp(cmd, "led") == 0) {
      JsonObject val = doc["value"];
      int m = val["mode"];
      int c = val["color"];
      int b = val["brightness"];
      setLedProperties(m, c, b);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    // Random Client ID to prevent conflicts
    String clientId = "DogDirect-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc="); Serial.print(client.state());
      delay(5000);
    }
  }
}

void setupDirectMQTT() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: "); Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  // 🟢 ESP32 always uses Port 1883 (TCP)
  client.setServer(mqtt_server, 1883);
  client.setCallback(handleDirectMessage);
}

void loopDirectMQTT() {
  if (!client.connected()) reconnect();
  client.loop();
}