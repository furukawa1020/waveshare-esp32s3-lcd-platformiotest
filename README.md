# Waveshare ESP32-S3-Touch-LCD-1.85 PlatformIO Project

Waveshare ESP32-S3-Touch-LCD-1.85 円形ディスプレイのための**動作する**PlatformIOプロジェクト

## ハードウェア仕様

- **ディスプレイ**: ST77916ドライバー、1.85インチ円形IPS、360×360解像度
- **接続**: QSPI (4データライン)
- **MCU**: ESP32-S3 デュアルコア 240MHz
- **重要**: TCA9554PWR IOエクスパンダー (I2C 0x20) でLCDリセット/電源制御

## 開発環境

- **PlatformIO** (Arduino Framework)
- **ESP32 Arduino**: 2.0.16 (自動インストール)
- **プラットフォーム**: espressif32 6.7.0

## セットアップ

```bash
# リポジトリをクローン
git clone https://github.com/furukawa1020/testwaveshareESPS3touchLCD2.git
cd testwaveshareESPS3touchLCD2

# PlatformIOでビルド
platformio run

# デバイスにアップロード (COMポートは環境に応じて変更)
platformio run --target upload --upload-port COM8

# シリアルモニター
platformio device monitor --port COM8 --baud 115200
```

## 現在の状態

✅ **動作確認済み**: 最小限のテストコード (シリアル出力)
🚧 **開発中**: ディスプレイドライバー実装

## ピン配置

- **QSPI**: SCK=40, DATA0=46, DATA1=45, DATA2=42, DATA3=41, CS=21
- **I2C**: SDA=6, SCL=7
- **バックライト**: PWM=5
- **LCD_TE**: 18

## 注意事項

- 公式デモはESP32 Arduino 3.0+が必要だがコンパイル不可
- このプロジェクトは2.0.16互換コードで実装
- 段階的に機能を追加中

## ライセンス

MIT License

## 参考

- [Waveshare公式Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
