# Waveshare ESP32-S3-Touch-LCD-1.85 - PlatformIO Project

**非公式/個人開発版 by furukawa1020**

GitHub: https://github.com/furukawa1020/waveshare-esp32s3-lcd-platformiotest

---

## 🎯 プロジェクトの目標

このプロジェクトは、**Waveshare ESP32-S3-Touch-LCD-1.85** (1.85インチ円形タッチディスプレイ)を使って、以下を実現することを目指しています:

### 最終目標
1. **カービィ風の円形キャラクター表示** - 360x360円形ディスプレイ全体を使った、まん丸な顔のキャラクター
2. **ローカルLLMとの連携** - キャラクターがユーザーと会話できるインタラクティブなデバイス
3. **タッチ操作対応** - CST816タッチセンサーによる直感的な操作

### 現在のフェーズ
**Phase 1: ハードウェア基本動作確認** ✅ (部分的に完了)
- I2C通信 ✅
- TCA9554PWR IOエキスパンダー ✅  
- バックライトPWM制御 ✅
- ディスプレイ初期化 🔄 (初期化コード実装済み、表示確認中)

---

## 🔧 ハードウェア仕様

### ディスプレイ
- **型番**: Waveshare ESP32-S3-Touch-LCD-1.85
- **サイズ**: 1.85インチ 円形IPS
- **解像度**: 360×360ピクセル
- **ドライバIC**: ST77916
- **インターフェース**: QSPI (Quad SPI - 4本データライン)

### 主要部品
| 部品 | 型番/仕様 | I2Cアドレス | 機能 |
|------|-----------|-------------|------|
| MCU | ESP32-S3 (240MHz デュアルコア) | - | メインプロセッサ |
| IOエキスパンダー | TCA9554PWR | 0x20 | LCD電源/リセット制御 |
| タッチセンサー | CST816S | 0x15 | 静電容量式タッチ |
| ジャイロ | QMI8658 | 0x6B | 6軸モーションセンサー |
| RTC | PCF85063 | 0x51 | リアルタイムクロック |
| オーディオ | PCM5101A (I2S) | - | DACオーディオ出力 |

### ピン配置 (重要)
```
QSPI (ST77916):
  SCK   = GPIO 40
  DATA0 = GPIO 46  ┐
  DATA1 = GPIO 45  │ QSPI 4本データライン
  DATA2 = GPIO 42  │ (標準SPIと異なる!)
  DATA3 = GPIO 41  ┘
  CS    = GPIO 21
  TE    = GPIO 18 (Tearing Effect)
  
Backlight: GPIO 5 (PWM制御)
I2C: SDA=GPIO 6, SCL=GPIO 7
```

---

## 📂 プロジェクト構造

```
testwaveshareESPS3touchLCD/
├── platformio.ini          # PlatformIO設定
├── README.md              # このファイル
├── DEVELOPMENT.md         # 開発メモ・技術詳細
├── src/
│   ├── main.cpp           # メインプログラム ⭐
│   ├── Display_ST77916.cpp/h    # ST77916 QSPIディスプレイドライバ
│   ├── I2C_Driver.cpp/h         # I2C通信基盤
│   ├── TCA9554PWR.cpp/h         # IOエキスパンダー制御
│   ├── Touch_CST816.cpp/h       # タッチセンサー
│   ├── Gyro_QMI8658.cpp/h       # ジャイロセンサー
│   ├── RTC_PCF85063.cpp/h       # RTC
│   ├── Audio_PCM5101.cpp/h      # オーディオ
│   ├── Wireless.cpp/h           # WiFi/BLE
│   └── esp_lcd_st77916.c/h      # ESP-IDF LCDドライバ (2.0.16互換版)
├── lib/
│   ├── lvgl/              # LVGLグラフィックライブラリ
│   └── ESP32-audioI2S/    # オーディオライブラリ
└── ESP32-S3-Touch-LCD-1.85-Demo/  # Waveshare公式サンプル
```

---

## 🚀 現在の動作状況

### ✅ 動作確認済み (2025/11/10)
- **I2C通信**: TCA9554PWRとの通信成功
- **IOエキスパンダー**: LCD_RSTとLCD_CS_IO制御可能
- **バックライト**: PWM 80%で点灯確認 (GPIO 5)
- **シリアル出力**: USB CDC経由でデバッグメッセージ表示
- **ビルド**: 347KB (Flash 10.4%)、20KB (RAM 6.2%)
- **アップロード**: COM8経由で正常完了

### 🔄 開発中
- **QSPIディスプレイ通信**: 初期化コードは実装済み、画像表示は未確認
  - **原因**: ESP32 Arduino 2.0.16では`quad_mode`フラグが未サポート
  - **対策**: 手動でQSPIモード有効化、またはESP32 Arduino 3.0+への移行検討

### ⏳ 未実装
- カービィ風キャラクター描画
- タッチ入力処理
- LVGLグラフィックライブラリ統合
- ローカルLLM連携

---

## 🛠️ 開発環境

### 必須ツール
- **PlatformIO**: VS Code拡張機能
- **ESP32 Arduino**: 2.0.16 (framework-arduinoespressif32 3.20016.0)
- **USBドライバ**: CH340/CP2102等 (デバイスによる)

### ビルド設定 (platformio.ini)
```ini
[env:esp32s3_lcd]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

build_flags = 
    -std=gnu++17
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
    -DCORE_DEBUG_LEVEL=5

upload_speed = 921600
monitor_speed = 115200
```

---

## 📝 使い方

### 1. ビルド
```bash
cd testwaveshareESPS3touchLCD
platformio run
```

### 2. アップロード
```bash
platformio run --target upload --upload-port COM8
```
(COMポート番号は環境に応じて変更)

### 3. シリアルモニター
```bash
platformio device monitor --port COM8 --baud 115200
```

### 期待される動作
1. バックライトが点灯 (80%輝度)
2. シリアル出力に初期化メッセージ
3. 色サイクル: Red → Green → Blue → White (3秒間隔)

---

## 🐛 既知の問題

### 1. ディスプレイに画像が表示されない
**症状**: バックライトのみ点灯、画面は黒  
**原因**: ESP32 Arduino 2.0.16で`esp_lcd_panel_io_spi_config_t`の`quad_mode`フラグが未サポート  
**回避策**: 
- Option A: ESP32 Arduino 3.0+に更新 (PlatformIOでは困難)
- Option B: 純粋なESP-IDF環境に移行
- Option C: QSPIレジスタを直接操作 (高度)

### 2. アップロード失敗 (No serial data received)
**症状**: `Stub running...` の後にエラー  
**原因**: Baud rate変更時のシリアル通信不安定  
**対策**: 
- USB 2.0ポートを使用
- データ転送対応USBケーブルを使用
- `upload_speed`を115200に下げる

---

## 📚 参考資料

- [Waveshare公式Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
- [ST77916データシート](https://www.displayfuture.com/Display/datasheet/controller/ST77916.pdf)
- [ESP32-S3技術リファレンス](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [ESP-IDF LCD API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/lcd.html)

---

## 🤝 コントリビューション

このプロジェクトは個人的な実験プロジェクトですが、改善提案やバグレポートは歓迎します。

**連絡先**: f.kotaro.0530@gmail.com

---

## 📄 ライセンス

このプロジェクトのコードはMITライセンスの下で公開されています。  
Waveshare提供のサンプルコードは元のライセンスに従います。

---

**Last Updated**: 2025年11月10日  
**Version**: 0.1.0-alpha (Phase 1 開発中)
