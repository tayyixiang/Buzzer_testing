#ifndef LORA_MODULE_H
#define LORA_MODULE_H

#include <Arduino.h>

void loraInit();
void loraSendBuzzerCommand(int state);
void loraSendLedCommand(int mode, int color, int brightness);

// --- New Function (ADDED) ---
void loraSendVibrationCommand(int pattern); 

#endif