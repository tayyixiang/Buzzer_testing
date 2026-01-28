#include <Arduino.h>
#include "lora_module.h"
#include "buzzer.h"
#include "led_function.h"
#include "mqtt_direct.h" // Enables Wi-Fi control

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Smart Dog ESP...");
  
  // 1. Init Hardware
  initBuzzer();
  LedInit();
  
  // 2. Init LoRa (We keep this so the code compiles)
  loraInit();

  // 3. Init Wi-Fi & MQTT (This is the new "Brain")
  setupDirectMQTT(); 
}

void loop() {
  // 1. Listen for Wi-Fi Commands (Priority)
  loopDirectMQTT(); 
  
  // 2. Listen for LoRa (Backup)
  loraHandleIncoming();
  
  // 3. Run Animations
  runLedAnimations();
}