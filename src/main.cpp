// Waveshare ESP32-S3-Touch-LCD-1.85 Test
// Step 3: ESP-IDF QSPI Display Driver

#include <Arduino.h>
#include <Wire.h>

extern "C" {
  #include "driver/spi_master.h"
  #include "esp_lcd_panel_io.h"
  #include "esp_lcd_panel_ops.h"
  #include "esp_lcd_st77916.h"
}

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

// Display opcodes
#define LCD_OPCODE_WRITE_CMD    0x02ULL
#define LCD_OPCODE_WRITE_COLOR  0x32ULL

esp_lcd_panel_handle_t panel_handle = NULL;

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

void initDisplay() {
  Serial.println("Initializing ST77916 display with ESP-IDF...");
  
  // Configure SPI bus
  spi_bus_config_t bus_config = {
    .mosi_io_num = QSPI_DATA0,
    .miso_io_num = -1,
    .sclk_io_num = QSPI_SCK,
    .quadwp_io_num = QSPI_DATA2,
    .quadhd_io_num = QSPI_DATA3,
    .data4_io_num = -1,
    .data5_io_num = -1,
    .data6_io_num = -1,
    .data7_io_num = -1,
    .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
    .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS,
    .intr_flags = 0,
  };
  
  // Initialize SPI bus
  esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    Serial.printf("SPI bus init failed: %d\n", ret);
    return;
  }
  Serial.println("SPI bus initialized");
  
  // Configure panel IO (QSPI)
  esp_lcd_panel_io_spi_config_t io_config = {};
  io_config.cs_gpio_num = QSPI_CS;
  io_config.dc_gpio_num = -1;
  io_config.spi_mode = 0;
  io_config.pclk_hz = 40 * 1000 * 1000; // 40MHz
  io_config.trans_queue_depth = 10;
  io_config.on_color_trans_done = NULL;
  io_config.user_ctx = NULL;
  io_config.lcd_cmd_bits = 32;
  io_config.lcd_param_bits = 8;
  // Note: QSPI flags not available in 2.0.16, configured in vendor config
  
  esp_lcd_panel_io_handle_t io_handle = NULL;
  ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);
  if (ret != ESP_OK) {
    Serial.printf("Panel IO init failed: %d\n", ret);
    return;
  }
  Serial.println("Panel IO initialized");
  
  // Configure panel device (ESP32 Arduino 2.0.16 compatible)
  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = -1; // Controlled by TCA9554PWR
  panel_config.bits_per_pixel = 16;
  // Note: rgb_ele_order not available in 2.0.16
  
  st77916_vendor_config_t vendor_config = {
    .init_cmds = NULL,
    .init_cmds_size = 0,
    .flags = {
      .use_qspi_interface = 1,
    },
  };
  panel_config.vendor_config = &vendor_config;
  
  // Create ST77916 panel
  ret = esp_lcd_new_panel_st77916(io_handle, &panel_config, &panel_handle);
  if (ret != ESP_OK) {
    Serial.printf("Panel init failed: %d\n", ret);
    return;
  }
  Serial.println("Panel created");
  
  // Reset and initialize panel
  esp_lcd_panel_reset(panel_handle);
  delay(100);
  
  esp_lcd_panel_init(panel_handle);
  delay(100);
  
  // Turn on display
  esp_lcd_panel_disp_on_off(panel_handle, true);
  delay(10);
  
  Serial.println("Display initialized!");
}

void fillScreen(uint16_t color) {
  if (panel_handle == NULL) {
    Serial.println("Panel not initialized!");
    return;
  }
  
  Serial.print("Filling screen with color: 0x");
  Serial.println(color, HEX);
  
  // Allocate buffer for one row
  uint16_t *line_buf = (uint16_t *)heap_caps_malloc(LCD_WIDTH * sizeof(uint16_t), MALLOC_CAP_DMA);
  if (line_buf == NULL) {
    Serial.println("Failed to allocate line buffer!");
    return;
  }
  
  // Fill buffer with color
  for (int i = 0; i < LCD_WIDTH; i++) {
    line_buf[i] = (color >> 8) | (color << 8); // Swap bytes for RGB565
  }
  
  // Draw each row
  for (int y = 0; y < LCD_HEIGHT; y++) {
    esp_lcd_panel_draw_bitmap(panel_handle, 0, y, LCD_WIDTH, y + 1, line_buf);
  }
  
  free(line_buf);
  Serial.println("Screen filled!");
}

void setup() {
  // USB Serial
  Serial.begin(115200);
  delay(2000); // Wait for USB CDC
  Serial.println("\n=== Waveshare ESP32-S3-Touch-LCD-1.85 Test ===");
  Serial.println("Starting initialization...");
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("I2C initialized");
  
  // Initialize IO Expander (controls LCD power/reset)
  delay(100);
  initIOExpander();
  Serial.println("IO Expander done");
  
  // Initialize Backlight
  delay(100);
  initBacklight();
  Serial.println("Backlight done");
  
  // Blink backlight to show we're alive
  for(int i = 0; i < 3; i++) {
    ledcWrite(0, 255); // Full brightness
    delay(200);
    ledcWrite(0, 50);  // Dim
    delay(200);
  }
  ledcWrite(0, 200); // Back to 80%
  Serial.println("Blink test done");
  
  // Initialize Display
  delay(100);
  Serial.println("About to init display...");
  initDisplay();
  Serial.println("Display init returned");
  
  // Test: Fill screen with red
  delay(500);
  Serial.println("About to fill screen...");
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
