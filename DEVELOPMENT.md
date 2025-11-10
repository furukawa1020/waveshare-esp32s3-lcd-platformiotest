# 開発ログ - Waveshare ESP32-S3-Touch-LCD-1.85

このドキュメントは、開発過程で得られた技術的知見と問題解決の記録です。

---

## 📅 開発タイムライン

### 2025/11/10 - Phase 1: 基本ハードウェア動作確認

#### 成功したこと
1. **I2C通信の確立**
   - TCA9554PWR IOエキスパンダーと通信成功
   - アドレス 0x20 で応答確認

2. **バックライト制御**
   - GPIO 5 をPWM制御 (5kHz, 8bit)
   - 80%輝度で安定動作
   - ledcSetup + ledcAttachPin + ledcWrite の組み合わせ

3. **ビルド・アップロード**
   - PlatformIO環境構築成功
   - ESP32 Arduino 2.0.16 で動作
   - ファームウェアサイズ: 347KB (10.4%)

#### 課題
1. **QSPIディスプレイが表示しない**
   - バックライトのみ点灯、画面は黒
   - 初期化コードは実行されている
   - 根本原因: `quad_mode`フラグ未サポート

---

## 🔬 技術的詳細

### QSPI vs 標準SPI

このディスプレイは**QSPI (Quad SPI)**を使用しています。標準SPIとの違い:

| 項目 | 標準SPI | QSPI |
|------|---------|------|
| データライン | 1本 (MOSI) | 4本 (DATA0-3) |
| 転送速度 | 1ビット/クロック | 4ビット/クロック |
| Arduinoサポート | 標準 | 非標準 (ESP-IDF必要) |

**重要**: Arduino SPI ライブラリは QSPI に対応していません。ESP-IDF の `esp_lcd` API を使用する必要があります。

### ESP32 Arduino バージョン問題

#### 2.0.16 の制限
```cpp
// ❌ 2.0.16では使えない
esp_lcd_panel_io_spi_config_t io_config = {
    .flags.quad_mode = 1  // ← このフラグが存在しない
};
```

#### 3.0+ で利用可能
```cpp
// ✅ 3.0+で使える
esp_lcd_panel_io_spi_config_t io_config = {
    .flags.quad_mode = 1  // ← これでQSPI有効化
};
```

**問題**: PlatformIOでESP32 Arduino 3.0+への更新が困難

### ST77916 初期化シーケンス

ST77916は200以上の初期化コマンドが必要な複雑なドライバーです:

```cpp
static const st77916_lcd_init_cmd_t vendor_specific_init_new[] = {
    // コマンド1: レジスタアクセス有効化
    {0xF0, (uint8_t[]){0xC3}, 1, 0},
    {0xF0, (uint8_t[]){0x96}, 1, 0},
    
    // ... 200以上のコマンド ...
    
    // 最後: ディスプレイON
    {0x29, NULL, 0, 0},
};
```

各コマンドの構造:
- `cmd`: コマンドバイト (32bit幅でQSPI送信)
- `data`: パラメータデータ
- `len`: データ長
- `delay`: 送信後の待機時間 (ms)

---

## 🐛 問題解決の記録

### 問題1: ディスプレイが何も表示しない

**症状**:
- バックライトは点灯
- シリアル出力は正常
- 初期化コードは実行される
- 画面は真っ黒

**デバッグ手順**:
1. I2C通信確認 → OK
2. バックライトPWM確認 → OK
3. QSPI初期化ログ確認 → 実行されている
4. レジスタ読み取り試行 → タイムアウト

**根本原因**:
ESP32 Arduino 2.0.16 の `esp_lcd_panel_io_spi_config_t` 構造体に `quad_mode` フラグが存在しない。このため、QSPIモードが有効化されず、4本のデータラインが使用されていない。

**試した回避策**:
- ❌ PlatformIOでESP32 Arduino 3.0+に更新 → 失敗
- ❌ `platform_packages` で強制更新 → 失敗
- ❌ USB-CDC無効化 → 効果なし
- 🔄 直接レジスタ操作でQSPI有効化 → 未実装

**推奨解決策**:
1. **ESP-IDF環境に移行** (PlatformIO + ESP-IDF framework)
2. **Arduino IDE + ESP32 3.0+** を使用
3. **Waveshare公式サンプル** をベースに再構築

### 問題2: アップロード失敗 (No serial data received)

**症状**:
```
Stub running...
Changing baud rate to 921600
A fatal error occurred: No serial data received.
```

**原因**:
Baud rate変更時のシリアル通信が不安定

**解決策**:
1. `upload_speed = 115200` に下げる (platformio.ini)
2. USB 2.0ポートを使用
3. データ転送対応USBケーブルを使用
4. BOOTボタンを使った手動書き込み

---

## 📊 パフォーマンス

### メモリ使用量
```
RAM:   20,392 bytes (6.2% of 327,680 bytes)
Flash: 347,201 bytes (10.4% of 3,342,336 bytes)
```

### ビルド時間
- Clean build: 約60秒
- Incremental build: 約15秒

---

## 🎓 学んだこと

### 1. QSPIは特殊なプロトコル
標準のArduino SPI APIでは対応できない。ESP-IDF の低レベルAPIが必要。

### 2. ESP32 Arduinoのバージョン管理は複雑
PlatformIOでのバージョン指定は期待通りに動作しないことがある。

### 3. ハードウェア抽象化の重要性
IOエキスパンダー経由の制御など、ボード固有の初期化シーケンスが多い。

### 4. デバッグ出力は不可欠
シリアル出力がなければ、何も表示されないディスプレイのデバッグは不可能だった。

---

## 🔮 次のステップ

### Phase 2: ディスプレイ表示の実現

**Option A: ESP-IDF環境への移行** (推奨)
```bash
# 新規プロジェクト作成
pio init --board esp32-s3-devkitc-1 --project-option "framework=espidf"
```

**Option B: Arduino IDE + ESP32 3.0+**
Arduino IDEを使用し、ESP32ボードマネージャーで3.0+をインストール

**Option C: 直接レジスタ操作** (高難度)
ESP32-S3のSPI3レジスタを直接操作してQSPIモードを有効化

### Phase 3: グラフィック描画

1. 単色塗りつぶし確認
2. 基本図形描画 (円、矩形)
3. RGB565フォーマットの画像表示
4. カービィキャラクター実装

### Phase 4: インタラクション

1. タッチ入力処理
2. ジャイロセンサー連携
3. 簡単なアニメーション

### Phase 5: LLM連携

1. WiFi接続
2. HTTP/WebSocket通信
3. ローカルLLMサーバーとの連携
4. 音声入出力 (オプション)

---

## 📝 メモ

### デバッグに役立つコマンド

```bash
# シリアルモニター (カラー出力)
pio device monitor --port COM8 --baud 115200 --filter colorize

# 詳細ビルドログ
pio run -v

# クリーンビルド
pio run --target clean
pio run

# アップロード (ポート指定)
pio run --target upload --upload-port COM8
```

### 便利なGitコマンド

```bash
# 最後の動作確認バージョンに戻る
git reset --hard 8c8957f

# 変更を一時退避
git stash save "WIP: 実験中のコード"

# 履歴を確認
git log --oneline --graph --all
```

---

**最終更新**: 2025年11月10日
