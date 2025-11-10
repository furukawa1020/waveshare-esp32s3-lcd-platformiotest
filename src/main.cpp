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

/**
 * setup() - 初期化処理
 * 
 * 実行順序:
 * 1. シリアル通信初期化 (115200bps)
 * 2. I2C通信初期化
 * 3. IOエキスパンダー初期化 (TCA9554PWR)
 * 4. バックライトPWM初期化
 * 5. バックライト点滅テスト (3回)
 * 6. LCDディスプレイ初期化 (QSPI)
 * 7. 画面を赤で塗りつぶし
 * 
 * 動作確認:
 *   ✅ シリアル出力が表示される
 *   ✅ バックライトが点滅する
 *   ❌ 画面に色は表示されない (QSPI問題)
 */
void setup() {
  // シリアル通信開始
  Serial.begin(115200);
  delay(2000);  // シリアルモニター接続待ち
  Serial.println("\n=== Waveshare ESP32-S3-Touch-LCD-1.85 Test ===");
  Serial.println("Project: Kirby-style character + Local LLM");
  Serial.println("Phase: Hardware initialization\n");
  
  // I2C初期化 (SDA=GPIO6, SCL=GPIO7)
  I2C_Init();
  Serial.println("[OK] I2C initialized");
  
  // IOエキスパンダー初期化 (TCA9554PWR @ 0x20)
  // LCD_RSTとLCD_CS_IOを制御
  TCA9554PWR_Init(0x00);
  Serial.println("[OK] IO Expander (TCA9554PWR) initialized");
  
  // バックライト初期化
  initBacklight();
  
  // バックライト点滅テスト (動作確認用)
  Serial.println("[TEST] Backlight blinking test...");
  for(int i = 0; i < 3; i++) {
    ledcWrite(0, 255);  // 100%輝度
    delay(200);
    ledcWrite(0, 50);   // 約20%輝度
    delay(200);
  }
  ledcWrite(0, 200);    // 80%輝度に戻す
  Serial.println("[OK] Backlight test passed");
  
  // LCDディスプレイ初期化 (ST77916 QSPI)
  Serial.println("[INFO] Initializing LCD (ST77916 QSPI)...");
  LCD_Init();
  Serial.println("[OK] LCD initialization complete");
  Serial.println("[WARN] Display output not visible due to quad_mode issue");
  
  // 画面を赤で塗りつぶし (テスト)
  Serial.println("[INFO] Filling screen with RED...");
  fillScreen(RED);
  
  Serial.println("\n[DONE] Setup complete! Entering color cycle loop...\n");
}

/**
 * loop() - メインループ
 * 
 * 3秒ごとに画面の色を変更:
 *   Red → Green → Blue → White → (繰り返し)
 * 
 * シリアル出力で色の変更を確認できます。
 * (実際のディスプレイには表示されません)
 */
void loop() {
  static uint8_t idx = 0;              // 現在の色インデックス
  static unsigned long last = 0;       // 最後に色を変えた時刻
  
  // 3秒経過したら次の色に切り替え
  if (millis() - last > 3000) {
    last = millis();
    
    switch(idx) {
      case 0: 
        fillScreen(GREEN); 
        Serial.println("[COLOR] Green"); 
        break;
      case 1: 
        fillScreen(BLUE); 
        Serial.println("[COLOR] Blue"); 
        break;
      case 2: 
        fillScreen(WHITE); 
        Serial.println("[COLOR] White"); 
        break;
      case 3: 
        fillScreen(RED); 
        Serial.println("[COLOR] Red"); 
        break;
    }
    idx = (idx + 1) % 4;  // 0→1→2→3→0...
  }
  
  delay(100);  // CPU負荷軽減
}