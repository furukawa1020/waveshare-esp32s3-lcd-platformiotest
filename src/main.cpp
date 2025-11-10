/**
 * Waveshare ESP32-S3-Touch-LCD-1.85 テストプログラム
 * 
 * 【プロジェクトの目標】
 * 1. カービィ風の円形キャラクターを360x360ディスプレイに表示
 * 2. ローカルLLMと連携した会話機能
 * 3. タッチ操作による直感的なインタラクション
 * 
 * 【現在のフェーズ】
 * Phase 1: ハードウェア基本動作確認
 *   ✅ I2C通信
 *   ✅ IOエキスパンダー (TCA9554PWR)
 *   ✅ バックライトPWM制御
 *   🔄 ディスプレイQSPI通信 (初期化済み、表示確認中)
 * 
 * 【動作確認済み】
 * - バックライト: 80%輝度で点灯
 * - シリアル出力: USB CDC経由でメッセージ表示
 * - カラーサイクル: Red→Green→Blue→White (3秒間隔)
 * 
 * 【既知の問題】
 * - ディスプレイに画像が表示されない (バックライトのみ)
 *   原因: ESP32 Arduino 2.0.16でQSPIの`quad_mode`フラグ未サポート
 *   対策: ESP-IDF環境への移行またはArduino 3.0+使用を検討
 * 
 * GitHub: https://github.com/furukawa1020/waveshare-esp32s3-lcd-platformiotest
 */

#include <Arduino.h>
#include "Display_ST77916.h"
#include "I2C_Driver.h"
#include "TCA9554PWR.h"

// ハードウェアピン定義
#define LCD_BL 5  // バックライトPWM制御ピン

// RGB565カラーパレット (16ビットカラー)
#define RED    0xF800  // 赤   (11111 000000 00000)
#define GREEN  0x07E0  // 緑   (00000 111111 00000)
#define BLUE   0x001F  // 青   (00000 000000 11111)
#define WHITE  0xFFFF  // 白   (全ビット1)
#define BLACK  0x0000  // 黒   (全ビット0)

/**
 * バックライトPWM初期化
 * 
 * PWM設定:
 *   - チャンネル: 0
 *   - 周波数: 5kHz
 *   - 分解能: 8bit (0-255)
 *   - デューティ: 200/255 (約80%輝度)
 */
void initBacklight() {
  ledcSetup(0, 5000, 8);       // チャンネル0, 5kHz, 8bit
  ledcAttachPin(LCD_BL, 0);    // GPIO5をチャンネル0に接続
  ledcWrite(0, 200);           // デューティ比 200/255 (約80%)
  Serial.println("[OK] Backlight initialized (80% brightness)");
}

/**
 * 画面全体を単色で塗りつぶす
 * 
 * @param color RGB565形式の色コード
 * 
 * 注意: 現在、ディスプレイに表示されない問題があります。
 *       バックライトは点灯しますが、QSPI通信が正常に動作していません。
 *       (ESP32 Arduino 2.0.16のquad_mode未サポートが原因)
 */
void fillScreen(uint16_t color) {
  uint16_t *buf = (uint16_t*)malloc(360 * sizeof(uint16_t));
  if (buf) {
    // 1行分のバッファに色を設定
    for(int i = 0; i < 360; i++) buf[i] = color;
    
    // 360行すべてに同じ色を書き込み
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