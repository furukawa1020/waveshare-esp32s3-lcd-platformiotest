// Waveshare ESP32-S3-Touch-LCD-1.85 Test
// Step 1: I2C + Backlight

#include <Arduino.h>
#include <Wire.h>

// Pin definitions
#define I2C_SDA 6
#define I2C_SCL 7
#define LCD_BL 5
#define TCA9554_ADDR 0x20

// TCA9554PWR IO Expander registers
#define TCA9554_OUTPUT_PORT 0x01
#define TCA9554_CONFIG_PORT 0x03

void initIOExpander() {
  // Configure all pins as outputs
  Wire.beginTransmission(TCA9554_ADDR);
  Wire.write(TCA9554_CONFIG_PORT);
  Wire.write(0x00); // All outputs
  if (Wire.endTransmission() != 0) {
    Serial.println("ERROR: TCA9554 config failed!");
    return;
  }
  
  // Set all outputs HIGH (enable LCD power/reset)
  Wire.beginTransmission(TCA9554_ADDR);
  Wire.write(TCA9554_OUTPUT_PORT);
  Wire.write(0xFF); // All HIGH
  if (Wire.endTransmission() != 0) {
    Serial.println("ERROR: TCA9554 output failed!");
    return;
  }
  
  Serial.println("TCA9554 initialized OK");
}

void initBacklight() {
  // Use ESP32 2.0.16 compatible API
  ledcSetup(0, 5000, 8);      // Channel 0, 5kHz, 8-bit resolution
  ledcAttachPin(LCD_BL, 0);   // Attach GPIO5 to channel 0
  ledcWrite(0, 128);          // 50% brightness
  Serial.println("Backlight initialized (50%)");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== Waveshare ESP32-S3-Touch-LCD-1.85 Test ===");
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("I2C initialized");
  
  // Initialize IO Expander (controls LCD power/reset)
  delay(100);
  initIOExpander();
  
  // Initialize Backlight
  delay(100);
  initBacklight();
  
  Serial.println("Setup complete!");
}

void loop() {
  Serial.println("Running...");
  delay(2000);
}
