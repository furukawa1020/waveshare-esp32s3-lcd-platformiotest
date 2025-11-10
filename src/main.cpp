#include <Arduino.h>
#include "Display_ST77916.h"
#include "I2C_Driver.h"
#include "TCA9554PWR.h"

#define LCD_BL 5

// RGB565 colors
#define RED    0xF800
#define GREEN  0x07E0
#define BLUE   0x001F
#define WHITE  0xFFFF
#define BLACK  0x0000

void initBacklight() {
  ledcSetup(0, 5000, 8);
  ledcAttachPin(LCD_BL, 0);
  ledcWrite(0, 200);
  Serial.println("Backlight OK");
}

void fillScreen(uint16_t color) {
  uint16_t *buf = (uint16_t*)malloc(360 * sizeof(uint16_t));
  if (buf) {
    for(int i = 0; i < 360; i++) buf[i] = color;
    for(int y = 0; y < 360; y++) {
      LCD_addWindow(0, y, 360, y+1, buf);
    }
    free(buf);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== ESP32-S3 LCD Test ===");
  
  I2C_Init();
  Serial.println("I2C OK");
  
  TCA9554PWR_Init(0x00);
  Serial.println("EXIO OK");
  
  initBacklight();
  
  for(int i = 0; i < 3; i++) {
    ledcWrite(0, 255);
    delay(200);
    ledcWrite(0, 50);
    delay(200);
  }
  ledcWrite(0, 200);
  Serial.println("Blink OK");
  
  Serial.println("Init LCD...");
  LCD_Init();
  Serial.println("LCD OK");
  
  Serial.println("Fill red...");
  fillScreen(RED);
  
  Serial.println("Done!");
}

void loop() {
  static uint8_t idx = 0;
  static unsigned long last = 0;
  
  if (millis() - last > 3000) {
    last = millis();
    
    switch(idx) {
      case 0: fillScreen(GREEN); Serial.println("Green"); break;
      case 1: fillScreen(BLUE); Serial.println("Blue"); break;
      case 2: fillScreen(WHITE); Serial.println("White"); break;
      case 3: fillScreen(RED); Serial.println("Red"); break;
    }
    idx = (idx + 1) % 4;
  }
  delay(100);
}