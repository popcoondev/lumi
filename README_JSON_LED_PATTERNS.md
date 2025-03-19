# JSON LED パターンシステム

このシステムは、JSONファイルを使用してLEDパターンを定義し、動的に読み込んで実行することができます。これにより、コードを変更せずに新しいパターンを追加したり、既存のパターンをカスタマイズしたりすることが可能になります。

## 機能

- JSONファイルからLEDパターンを読み込み
- HSV色空間での色指定（固定値またはランダム範囲）
- 複数のステップからなるパターン定義
- 面選択の柔軟な指定（固定、ランダム、全面）
- エフェクト（フェード、ブラー）のサポート
- パターンのループ再生

## JSONフォーマット

```json
{
  "name": "パターン名",
  "type": "custom",
  "parameters": {
    "loop": true,
    "stepDelay": {
      "min": 500,
      "max": 1500
    },
    "colorHSV": {
      "h": { "min": 0, "max": 360 },
      "s": { "min": 50, "max": 100 },
      "v": { "min": 50, "max": 100 }
    },
    "effects": {
      "fade": {
        "enabled": true,
        "mode": "both",
        "duration": { "min": 500, "max": 1000 }
      },
      "blur": {
        "enabled": true,
        "intensity": { "min": 2, "max": 5 },
        "duration": { "min": 200, "max": 600 }
      }
    }
  },
  "steps": [
    {
      "faceSelection": {
        "mode": "random",
        "range": { "min": 0, "max": 7 },
        "count": 3
      },
      "colorHSV": {
        "h": { "min": 200, "max": 240 },
        "s": 80,
        "v": 100
      },
      "duration": { "min": 800, "max": 1200 }
    },
    {
      "faces": [4, 5, 6],
      "colorHSV": {
        "h": 50,
        "s": 90,
        "v": 100
      },
      "duration": 1200
    }
  ]
}
```

### パラメータの説明

#### グローバルパラメータ（parameters）

- `loop`: パターン全体をループさせるかどうか（true/false）
- `stepDelay`: 各ステップ間の遅延時間（ミリ秒）
  - 固定値または範囲指定（min/max）が可能
- `colorHSV`: グローバルな色のデフォルト値
  - `h`: 色相（0-360）
  - `s`: 彩度（0-255）
  - `v`: 明度（0-255）
- `effects`: エフェクト設定
  - `fade`: フェードエフェクト
    - `enabled`: 有効/無効
    - `mode`: "in"（フェードイン）, "out"（フェードアウト）, "both"（両方）
    - `duration`: エフェクトの持続時間
  - `blur`: ブラーエフェクト
    - `enabled`: 有効/無効
    - `intensity`: ブラーの強度
    - `duration`: エフェクトの持続時間

#### ステップパラメータ（steps）

- `faceSelection`: 面選択の設定
  - `mode`: 選択モード（"random", "all", "sequential"）
  - `range`: ランダム選択時の範囲
  - `count`: ランダム選択時の選択数
- `faces`: 直接面を指定する場合の配列
- `colorHSV`: ステップごとの色設定（グローバル設定を上書き）
- `duration`: ステップの持続時間（ミリ秒）

## 使用方法

### JSONパターンの読み込み

```cpp
// ファイルからの読み込み
ledManager.loadJsonPatternsFromFile("/led_patterns.json");

// 文字列からの読み込み
String jsonString = "..."; // JSONパターン文字列
ledManager.loadJsonPatternsFromString(jsonString);
```

### パターンの実行

```cpp
// 名前でパターンを実行
ledManager.runJsonPattern("パターン名");

// インデックスでパターンを実行
ledManager.runJsonPatternByIndex(0);
```

### パターンの停止

```cpp
ledManager.stopPattern();
```

## サンプルコード

`src/JsonLEDPatternTest.cpp` にサンプルコードがあります。このサンプルでは、SPIFFSからJSONパターンを読み込み、実行する方法を示しています。

## 注意事項

- JSONパターンを使用する前に、LEDManagerの初期化（begin）を呼び出す必要があります。
- パターンの実行中に別のパターンを実行すると、現在のパターンは停止し、新しいパターンが開始されます。
- エフェクトの使用はCPUに負荷をかける可能性があります。複雑なエフェクトを使用する場合は、パフォーマンスに注意してください。
