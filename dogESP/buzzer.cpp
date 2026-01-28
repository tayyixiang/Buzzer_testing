#include "buzzer.h"
#include <Arduino.h>

// T-Beam Pin 25 is the Buzzer/Vibrator
#define BUZZER_PIN 25 

void initBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Start OFF
}

void buzzerOn() {
    digitalWrite(BUZZER_PIN, HIGH);
}

void buzzerOff() {
    digitalWrite(BUZZER_PIN, LOW);
}

// 1 = Single Tap
// 2 = Double Tap
// 3 = Continuous (3 seconds)
void playBuzzerPattern(int pattern) {
    Serial.print("Playing Pattern: "); Serial.println(pattern);
    
    if (pattern == 1) {
        // Single Tap
        digitalWrite(BUZZER_PIN, HIGH);
        delay(300);
        digitalWrite(BUZZER_PIN, LOW);
    }
    else if (pattern == 2) {
        // Double Tap
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(150);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
    }
    else if (pattern == 3) {
        // Continuous (Safe 3s limit)
        digitalWrite(BUZZER_PIN, HIGH);
        delay(3000); 
        digitalWrite(BUZZER_PIN, LOW);
    }
}