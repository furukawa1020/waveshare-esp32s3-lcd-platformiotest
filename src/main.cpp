// Absolute minimum test - ESP32-S3

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);  // シリアル接続の安定化を待つ
  Serial.println("Setup complete!");
}

void loop() {
  Serial.println("Hello from ESP32-S3!");
  delay(1000);
}
