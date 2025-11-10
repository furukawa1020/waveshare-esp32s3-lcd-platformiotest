// Waveshare ESP32-S3-Touch-LCD-1.85 Test
// Step 2: Display initialization

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// Pin definitions
#define I2C_SDA 6
#define I2C_SCL 7
#define LCD_BL 5
#define TCA9554_ADDR 0x20

// QSPI pins
#define QSPI_SCK 40
#define QSPI_CS 21
#define QSPI_DATA0 46
#define QSPI_DATA1 45
#define QSPI_DATA2 42
#define QSPI_DATA3 41

// LCD size
#define LCD_WIDTH 360
#define LCD_HEIGHT 360

// TCA9554PWR IO Expander registers
#define TCA9554_OUTPUT_PORT 0x01
#define TCA9554_CONFIG_PORT 0x03

SPIClass *spi = NULL;

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
  ledcWrite(0, 200);          // ~80% brightness
  Serial.println("Backlight initialized (80%)");
}

void lcd_send_cmd(uint8_t cmd) {
  // QSPI format: [OPCODE(8)][CMD(8)][PARAM(16)]
  // For command: 0x02 (WRITE_CMD opcode) + cmd + 0x0000
  uint32_t data = (0x02 << 24) | (cmd << 16);
  
  digitalWrite(QSPI_CS, LOW);
  spi->transfer32(data);
  digitalWrite(QSPI_CS, HIGH);
  delayMicroseconds(1);
}

void lcd_send_data(uint8_t data) {
  // QSPI format for data: 0x32 (WRITE_COLOR opcode) + 0x00 + data byte
  uint32_t packet = (0x32 << 24) | data;
  
  digitalWrite(QSPI_CS, LOW);
  spi->transfer32(packet);
  digitalWrite(QSPI_CS, HIGH);
  delayMicroseconds(1);
}

void initDisplay() {
  Serial.println("Initializing ST77916 display...");
  
  // Initialize SPI with higher frequency
  spi = new SPIClass(HSPI);
  spi->begin(QSPI_SCK, QSPI_DATA0, QSPI_DATA1, QSPI_CS);
  pinMode(QSPI_CS, OUTPUT);
  digitalWrite(QSPI_CS, HIGH);
  
  // Configure SPI settings
  spi->beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
  
  delay(200); // Wait for display power stabilization
  
  // Full ST77916 initialization sequence
  Serial.println("Sending init sequence...");
  
  // Page 1
  lcd_send_cmd(0xF0); lcd_send_data(0x01);
  lcd_send_cmd(0xF1); lcd_send_data(0x01);
  lcd_send_cmd(0xB0); lcd_send_data(0x56);
  lcd_send_cmd(0xB1); lcd_send_data(0x4D);
  lcd_send_cmd(0xB2); lcd_send_data(0x24);
  lcd_send_cmd(0xB4); lcd_send_data(0x87);
  lcd_send_cmd(0xB5); lcd_send_data(0x44);
  lcd_send_cmd(0xB6); lcd_send_data(0x8B);
  lcd_send_cmd(0xB7); lcd_send_data(0x40);
  lcd_send_cmd(0xB8); lcd_send_data(0x86);
  
  // Gamma settings
  lcd_send_cmd(0xF0); lcd_send_data(0x02);
  lcd_send_cmd(0xE0); 
  lcd_send_data(0xF0); lcd_send_data(0x0A); lcd_send_data(0x10); lcd_send_data(0x09);
  lcd_send_data(0x09); lcd_send_data(0x36); lcd_send_data(0x35); lcd_send_data(0x33);
  lcd_send_data(0x4A); lcd_send_data(0x29); lcd_send_data(0x15); lcd_send_data(0x15);
  lcd_send_data(0x2E); lcd_send_data(0x34);
  
  lcd_send_cmd(0xE1);
  lcd_send_data(0xF0); lcd_send_data(0x0A); lcd_send_data(0x0F); lcd_send_data(0x08);
  lcd_send_data(0x08); lcd_send_data(0x05); lcd_send_data(0x34); lcd_send_data(0x33);
  lcd_send_data(0x4A); lcd_send_data(0x39); lcd_send_data(0x15); lcd_send_data(0x15);
  lcd_send_data(0x2D); lcd_send_data(0x33);
  
  // Back to page 0
  lcd_send_cmd(0xF0); lcd_send_data(0x00);
  
  // Basic display settings
  lcd_send_cmd(0x21); // Display Inversion ON
  lcd_send_cmd(0x36); // Memory Access Control
  lcd_send_data(0x00); // Normal orientation
  
  lcd_send_cmd(0x3A); // Pixel Format Set
  lcd_send_data(0x55); // 16-bit/pixel (RGB565)
  
  lcd_send_cmd(0x11); // Sleep Out
  delay(120);
  
  lcd_send_cmd(0x29); // Display ON
  delay(50);
  
  Serial.println("Display initialized!");
}

void fillScreen(uint16_t color) {
  Serial.print("Filling screen with color: 0x");
  Serial.println(color, HEX);
  
  // Set column address (0 to 359)
  lcd_send_cmd(0x2A);
  lcd_send_data(0x00);
  lcd_send_data(0x00);
  lcd_send_data(0x01);
  lcd_send_data(0x67); // 359
  
  // Set row address (0 to 359)
  lcd_send_cmd(0x2B);
  lcd_send_data(0x00);
  lcd_send_data(0x00);
  lcd_send_data(0x01);
  lcd_send_data(0x67); // 359
  
  // Write to RAM
  lcd_send_cmd(0x2C);
  
  // Send pixel data in QSPI format
  // Each pixel is 16-bit (RGB565)
  uint8_t colorHigh = (color >> 8) & 0xFF;
  uint8_t colorLow = color & 0xFF;
  
  digitalWrite(QSPI_CS, LOW);
  
  // Fill entire screen (360x360 pixels)
  uint32_t totalPixels = LCD_WIDTH * LCD_HEIGHT;
  for (uint32_t i = 0; i < totalPixels; i++) {
    // Send high byte
    uint32_t packet1 = (0x32 << 24) | colorHigh;
    spi->transfer32(packet1);
    
    // Send low byte
    uint32_t packet2 = (0x32 << 24) | colorLow;
    spi->transfer32(packet2);
  }
  
  digitalWrite(QSPI_CS, HIGH);
  Serial.println("Screen filled!");
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
  
  // Initialize Display
  delay(100);
  initDisplay();
  
  // Test: Fill screen with red
  delay(500);
  fillScreen(0xF800); // Red (RGB565)
  
  Serial.println("Setup complete!");
}

void loop() {
  // Cycle through colors
  static uint8_t colorIndex = 0;
  static unsigned long lastChange = 0;
  
  if (millis() - lastChange > 3000) {
    lastChange = millis();
    
    switch(colorIndex) {
      case 0:
        fillScreen(0x07E0); // Green
        Serial.println("Green");
        break;
      case 1:
        fillScreen(0x001F); // Blue
        Serial.println("Blue");
        break;
      case 2:
        fillScreen(0xFFFF); // White
        Serial.println("White");
        break;
      case 3:
        fillScreen(0xF800); // Red
        Serial.println("Red");
        break;
    }
    
    colorIndex = (colorIndex + 1) % 4;
  }
  
  delay(100);
}
