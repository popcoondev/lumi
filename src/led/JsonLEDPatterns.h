#ifndef JSON_LED_PATTERNS_H
#define JSON_LED_PATTERNS_H

#include <Arduino.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <functional>
// Forward declarations
class LedPattern;

// 値の範囲を表現するクラス
class MinMax {
public:
    MinMax() : min(0), max(0), fixed(true) {}
    
    // JSONからの初期化
    void fromJson(const JsonVariant& json) {
        if (json.is<JsonObject>()) {
            // 範囲指定の場合
            JsonObject obj = json.as<JsonObject>();
            if (obj.containsKey("min") && obj.containsKey("max")) {
                min = obj["min"].as<int>();
                max = obj["max"].as<int>();
                fixed = false;
            } else {
                min = max = 0;
                fixed = true;
            }
        } else {
            // 固定値の場合
            min = max = json.as<int>();
            fixed = true;
        }
    }
    
    // ランダム値または固定値を取得
    int getValue() const {
        if (fixed) {
            return min;
        } else {
            return random(min, max + 1);
        }
    }
    
    // 値を設定するメソッド
    void setValue(int value) {
        min = max = value;
        fixed = true;
    }
    
    // 範囲を設定するメソッド
    void setRange(int minValue, int maxValue) {
        min = minValue;
        max = maxValue;
        fixed = false;
    }
    
    bool isFixed() const { return fixed; }
    int getMin() const { return min; }
    int getMax() const { return max; }
    
private:
    int min;
    int max;
    bool fixed;
};

// HSV色空間での色指定を表現するクラス
class ColorHSV {
public:
    ColorHSV() {
        // デフォルト値の設定
        h.setValue(0);    // 色相 0 (赤)
        s.setValue(255);  // 彩度 最大
        v.setValue(255);  // 明度 最大
    }
    
    void fromJson(const JsonObject& json) {
        if (json.containsKey("h")) h.fromJson(json["h"]);
        if (json.containsKey("s")) s.fromJson(json["s"]);
        if (json.containsKey("v")) v.fromJson(json["v"]);
    }
    
    CHSV getColor() const {
        return CHSV(
            h.getValue(),
            s.getValue(),
            v.getValue()
        );
    }
    
    MinMax h;
    MinMax s;
    MinMax v;
};

// LEDの面選択方法を表現するクラス
class FaceSelection {
public:
    enum class Mode {
        FIXED,
        RANDOM,
        SEQUENTIAL,
        ALL
    };
    
    FaceSelection() : mode(Mode::ALL), count(0) {}
    
    void fromJson(const JsonObject& json) {
        if (json.containsKey("mode")) {
            String modeStr = json["mode"].as<String>();
            if (modeStr == "random") {
                mode = Mode::RANDOM;
                if (json.containsKey("range")) {
                    range.fromJson(json["range"]);
                }
                if (json.containsKey("count")) {
                    count = json["count"];
                }
            } else if (modeStr == "sequential") {
                mode = Mode::SEQUENTIAL;
            } else if (modeStr == "all") {
                mode = Mode::ALL;
            }
        }
    }
    
    std::vector<int> selectFaces(int numFaces) const {
        std::vector<int> result;
        
        switch (mode) {
            case Mode::RANDOM: {
                // 指定された範囲内からランダムに選択
                int min = range.getMin();
                int max = std::min(range.getMax(), numFaces - 1);
                
                // 選択候補を作成
                std::vector<int> candidates;
                for (int i = min; i <= max; i++) {
                    candidates.push_back(i);
                }
                
                // ランダムに選択
                int selectCount = std::min(count, static_cast<int>(candidates.size()));
                for (int i = 0; i < selectCount; i++) {
                    if (candidates.empty()) break;
                    int idx = random(0, candidates.size());
                    result.push_back(candidates[idx]);
                    candidates.erase(candidates.begin() + idx);
                }
                break;
            }
            
            case Mode::ALL:
                // すべての面を選択
                for (int i = 0; i < numFaces; i++) {
                    result.push_back(i);
                }
                break;
                
            case Mode::SEQUENTIAL:
                // 実行時に順番に選択するため、ここでは空のベクトルを返す
                break;
                
            case Mode::FIXED:
                // 固定面は別途指定されるため、ここでは空のベクトルを返す
                break;
        }
        
        return result;
    }
    
    Mode mode;
    MinMax range;
    int count;
};

// エフェクトクラス
class FadeEffect {
public:
    enum class Mode {
        IN,
        OUT,
        BOTH
    };
    
    FadeEffect() : enabled(false), mode(Mode::BOTH) {}
    
    void fromJson(const JsonObject& json) {
        if (json.containsKey("enabled")) {
            enabled = json["enabled"];
        }
        
        if (json.containsKey("mode")) {
            String modeStr = json["mode"].as<String>();
            if (modeStr == "in") {
                mode = Mode::IN;
            } else if (modeStr == "out") {
                mode = Mode::OUT;
            } else {
                mode = Mode::BOTH;
            }
        }
        
        if (json.containsKey("duration")) {
            duration.fromJson(json["duration"]);
        }
    }
    
    bool enabled;
    Mode mode;
    MinMax duration;
};

class BlurEffect {
public:
    BlurEffect() : enabled(false) {}
    
    void fromJson(const JsonObject& json) {
        if (json.containsKey("enabled")) {
            enabled = json["enabled"];
        }
        
        if (json.containsKey("intensity")) {
            intensity.fromJson(json["intensity"]);
        }
        
        if (json.containsKey("duration")) {
            duration.fromJson(json["duration"]);
        }
    }
    
    bool enabled;
    MinMax intensity;
    MinMax duration;
};

class Effects {
public:
    Effects() {}
    
    void fromJson(const JsonObject& json) {
        if (json.containsKey("fade")) {
            fade.fromJson(json["fade"]);
        }
        
        if (json.containsKey("blur")) {
            blur.fromJson(json["blur"]);
        }
    }
    
    FadeEffect fade;
    BlurEffect blur;
};

// パターンステップとグローバルパラメータ
class PatternStep {
public:
    PatternStep() : hasFaces(false) {}
    
    void fromJson(const JsonObject& json) {
        // 面の選択
        if (json.containsKey("faces")) {
            JsonArray facesArray = json["faces"];
            for (JsonVariant face : facesArray) {
                faces.push_back(face.as<int>());
            }
            hasFaces = true;
        } else {
            hasFaces = false;
        }
        
        if (json.containsKey("faceSelection")) {
            faceSelection.fromJson(json["faceSelection"]);
        }
        
        // 色の設定
        if (json.containsKey("colorHSV")) {
            colorHSV.fromJson(json["colorHSV"]);
        }
        
        // 持続時間
        if (json.containsKey("duration")) {
            duration.fromJson(json["duration"]);
        }
    }
    
    std::vector<int> faces;
    bool hasFaces;
    FaceSelection faceSelection;
    ColorHSV colorHSV;
    MinMax duration;
};

class GlobalParameters {
public:
    GlobalParameters() : loop(false) {}
    
    void fromJson(const JsonObject& json) {
        if (json.containsKey("loop")) {
            loop = json["loop"];
        }
        
        if (json.containsKey("stepDelay")) {
            stepDelay.fromJson(json["stepDelay"]);
        }
        
        if (json.containsKey("colorHSV")) {
            defaultColor.fromJson(json["colorHSV"]);
        }
        
        if (json.containsKey("effects")) {
            effects.fromJson(json["effects"]);
        }
    }
    
    bool loop;
    MinMax stepDelay;
    ColorHSV defaultColor;
    Effects effects;
};

// JSONパターンの基底クラス
class JsonLedPattern {
public:
    JsonLedPattern() : m_name("JSON Pattern") {}
    virtual ~JsonLedPattern() {}
    
    // JSONからパターンを解析するメソッド
    virtual void parseJson(const JsonObject& json) {
        if (json.containsKey("name")) {
            m_name = json["name"].as<String>();
        }
    }
    
    // パターン名を取得
    virtual String getName() { return m_name; }
    
    // パターンを実行するメソッド
    virtual void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) = 0;
    
protected:
    String m_name;
};

// カスタムJSONパターンの実装
class CustomJsonPattern : public JsonLedPattern {
public:
    CustomJsonPattern() : JsonLedPattern() {
        m_name = "Custom Pattern";
    }
    
    void parseJson(const JsonObject& json) {
        JsonLedPattern::parseJson(json);
        
        // グローバルパラメータの解析
        if (json.containsKey("parameters")) {
            m_params.fromJson(json["parameters"]);
        }
        
        // ステップの解析
        if (json.containsKey("steps")) {
            JsonArray stepsArray = json["steps"];
            for (JsonObject stepObj : stepsArray) {
                PatternStep step;
                step.fromJson(stepObj);
                m_steps.push_back(step);
            }
        }
    }
    
    void run(CRGB* leds, int numLeds, int ledOffset, int numFaces, int duration) {
        unsigned long startTime = millis();
        int stepIndex = 0;
        
        do {
            for (stepIndex = 0; stepIndex < m_steps.size(); stepIndex++) {
                // 実行時間チェック
                if (duration > 0 && millis() - startTime >= (unsigned long)duration) {
                    return;
                }
                
                // ステップを実行
                executeStep(leds, numLeds, ledOffset, numFaces, m_steps[stepIndex]);
                
                // ステップ間の遅延
                int stepDelay = m_params.stepDelay.getValue();
                if (stepDelay > 0) {
                    vTaskDelay(stepDelay / portTICK_PERIOD_MS);
                }
            }
        } while (m_params.loop && (duration == 0 || millis() - startTime < (unsigned long)duration));
    }
    
private:
    void executeStep(CRGB* leds, int numLeds, int ledOffset, int numFaces, const PatternStep& step) {
        // 面の選択
        std::vector<int> selectedFaces;
        
        if (step.hasFaces) {
            // 明示的に指定された面を使用
            selectedFaces = step.faces;
        } else {
            // faceSelectionに基づいて面を選択
            selectedFaces = step.faceSelection.selectFaces(numFaces);
        }
        
        // 色の取得
        CHSV color = step.colorHSV.getColor();
        
        // 選択された面にLEDを設定
        for (int i = 0; i < numFaces; i++) {
            int idx1 = ledOffset + (i * 2);
            int idx2 = ledOffset + (i * 2) + 1;
            
            // 選択された面かどうかをチェック
            bool isSelected = false;
            for (int face : selectedFaces) {
                if (face == i) {
                    isSelected = true;
                    break;
                }
            }
            
            if (isSelected) {
                // 選択された面は指定された色に
                leds[idx1] = color;
                leds[idx2] = color;
            } else {
                // 選択されていない面は消灯
                leds[idx1] = CRGB::Black;
                leds[idx2] = CRGB::Black;
            }
        }
        
        // エフェクトの適用
        applyEffects(leds, numLeds, ledOffset, numFaces);
        
        // LEDの表示
        FastLED.show();
        
        // ステップの持続時間
        int stepDuration = step.duration.getValue();
        if (stepDuration > 0) {
            vTaskDelay(stepDuration / portTICK_PERIOD_MS);
        }
    }
    
    void applyEffects(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
        // フェードエフェクト
        if (m_params.effects.fade.enabled) {
            applyFadeEffect(leds, numLeds, ledOffset, numFaces);
        }
        
        // ブラーエフェクト
        if (m_params.effects.blur.enabled) {
            applyBlurEffect(leds, numLeds, ledOffset, numFaces);
        }
    }
    
    void applyFadeEffect(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
        int duration = m_params.effects.fade.duration.getValue();
        FadeEffect::Mode mode = m_params.effects.fade.mode;
        
        // フェードインの実装
        if (mode == FadeEffect::Mode::IN || mode == FadeEffect::Mode::BOTH) {
            for (uint8_t b = 0; b < 255; b += 5) {
                for (int i = 0; i < numFaces; i++) {
                    int idx1 = ledOffset + (i * 2);
                    int idx2 = ledOffset + (i * 2) + 1;
                    
                    // 現在の色を保存
                    CRGB color1 = leds[idx1];
                    CRGB color2 = leds[idx2];
                    
                    // 明るさを適用
                    leds[idx1].nscale8_video(b);
                    leds[idx2].nscale8_video(b);
                    
                    // 元の色を復元（次のループのため）
                    leds[idx1] = color1;
                    leds[idx2] = color2;
                }
                FastLED.show();
                vTaskDelay((duration / 50) / portTICK_PERIOD_MS);
            }
        }
        
        // フェードアウトの実装
        if (mode == FadeEffect::Mode::OUT || mode == FadeEffect::Mode::BOTH) {
            for (int b = 255; b > 0; b -= 5) {
                for (int i = 0; i < numFaces; i++) {
                    int idx1 = ledOffset + (i * 2);
                    int idx2 = ledOffset + (i * 2) + 1;
                    
                    // 現在の色を保存
                    CRGB color1 = leds[idx1];
                    CRGB color2 = leds[idx2];
                    
                    // 明るさを適用
                    leds[idx1].nscale8_video(b);
                    leds[idx2].nscale8_video(b);
                    
                    // 元の色を復元（次のループのため）
                    leds[idx1] = color1;
                    leds[idx2] = color2;
                }
                FastLED.show();
                vTaskDelay((duration / 50) / portTICK_PERIOD_MS);
            }
        }
    }
    
    void applyBlurEffect(CRGB* leds, int numLeds, int ledOffset, int numFaces) {
        int intensity = m_params.effects.blur.intensity.getValue();
        int duration = m_params.effects.blur.duration.getValue();
        
        // 簡易的なブラー効果の実装
        // 実際の実装ではより洗練されたアルゴリズムが必要かもしれません
        for (int t = 0; t < duration; t += 50) {
            // 現在のLED状態をコピー
            CRGB tempLeds[numLeds];
            for (int i = 0; i < numLeds; i++) {
                tempLeds[i] = leds[i];
            }
            
            // 各LEDに対して周囲のLEDの平均値を計算
            for (int i = 0; i < numFaces; i++) {
                int idx1 = ledOffset + (i * 2);
                int idx2 = ledOffset + (i * 2) + 1;
                
                // 隣接する面のインデックス（簡易的な実装）
                int prev = (i > 0) ? i - 1 : numFaces - 1;
                int next = (i < numFaces - 1) ? i + 1 : 0;
                
                int prevIdx1 = ledOffset + (prev * 2);
                int prevIdx2 = ledOffset + (prev * 2) + 1;
                int nextIdx1 = ledOffset + (next * 2);
                int nextIdx2 = ledOffset + (next * 2) + 1;
                
                // 平均値の計算（簡易的）
                leds[idx1].r = (tempLeds[idx1].r * (10 - intensity) + (tempLeds[prevIdx1].r + tempLeds[nextIdx1].r) * intensity / 2) / 10;
                leds[idx1].g = (tempLeds[idx1].g * (10 - intensity) + (tempLeds[prevIdx1].g + tempLeds[nextIdx1].g) * intensity / 2) / 10;
                leds[idx1].b = (tempLeds[idx1].b * (10 - intensity) + (tempLeds[prevIdx1].b + tempLeds[nextIdx1].b) * intensity / 2) / 10;
                
                leds[idx2].r = (tempLeds[idx2].r * (10 - intensity) + (tempLeds[prevIdx2].r + tempLeds[nextIdx2].r) * intensity / 2) / 10;
                leds[idx2].g = (tempLeds[idx2].g * (10 - intensity) + (tempLeds[prevIdx2].g + tempLeds[nextIdx2].g) * intensity / 2) / 10;
                leds[idx2].b = (tempLeds[idx2].b * (10 - intensity) + (tempLeds[prevIdx2].b + tempLeds[nextIdx2].b) * intensity / 2) / 10;
            }
            
            FastLED.show();
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
    
    GlobalParameters m_params;
    std::vector<PatternStep> m_steps;
};

// パターンファクトリークラス
class PatternFactory {
public:
    static PatternFactory& getInstance() {
        static PatternFactory instance;
        return instance;
    }
    
    JsonLedPattern* createPattern(const JsonObject& json) {
        String type = "custom";
        if (json.containsKey("type")) {
            type = json["type"].as<String>();
        }
        
        if (m_creators.find(type) != m_creators.end()) {
            return m_creators[type](json);
        }
        
        // デフォルトはカスタムパターン
        return createCustomPattern(json);
    }
    
    void registerPatternType(const String& type, std::function<JsonLedPattern*(const JsonObject&)> creator) {
        m_creators[type] = creator;
    }
    
private:
    PatternFactory() {
        // デフォルトのパターン作成関数を登録
        registerPatternType("custom", createCustomPattern);
    }
    
    static JsonLedPattern* createCustomPattern(const JsonObject& json) {
        CustomJsonPattern* pattern = new CustomJsonPattern();
        pattern->parseJson(json);
        return pattern;
    }
    
    std::map<String, std::function<JsonLedPattern*(const JsonObject&)>> m_creators;
};

// JSONパターンマネージャークラス
class JsonPatternManager {
public:
    JsonPatternManager() {}
    ~JsonPatternManager() {
        // パターンの解放
        for (auto pattern : m_patterns) {
            delete pattern;
        }
        m_patterns.clear();
        m_patternNameMap.clear();
    }
    
    bool loadPatternsFromFile(const String& filename) {
        // SPIFFSやSDカードからJSONファイルを読み込む実装
        // 現在は未実装
        return false;
    }
    
    bool loadPatternsFromJson(const String& jsonString) {
        // 既存のパターンをクリア
        for (auto pattern : m_patterns) {
            delete pattern;
        }
        m_patterns.clear();
        m_patternNameMap.clear();
        
        // JSONの解析
        DynamicJsonDocument doc(16384);  // サイズは適宜調整
        DeserializationError error = deserializeJson(doc, jsonString);
        
        if (error) {
            Serial.print(F("JSON parsing failed: "));
            Serial.println(error.c_str());
            return false;
        }
        
        // 単一のパターンの場合
        if (doc.is<JsonObject>()) {
            JsonObject patternObj = doc.as<JsonObject>();
            JsonLedPattern* pattern = PatternFactory::getInstance().createPattern(patternObj);
            if (pattern) {
                m_patterns.push_back(pattern);
                m_patternNameMap[pattern->getName()] = 0;
            }
        }
        // パターンの配列の場合
        else if (doc.containsKey("patterns") && doc["patterns"].is<JsonArray>()) {
            JsonArray patternsArray = doc["patterns"];
            int index = 0;
            for (JsonObject patternObj : patternsArray) {
                JsonLedPattern* pattern = PatternFactory::getInstance().createPattern(patternObj);
                if (pattern) {
                    m_patterns.push_back(pattern);
                    m_patternNameMap[pattern->getName()] = index++;
                }
            }
        }
        
        return !m_patterns.empty();
    }
    
    int getPatternCount() const {
        return m_patterns.size();
    }
    
    JsonLedPattern* getPatternByIndex(int index) {
        if (index >= 0 && index < m_patterns.size()) {
            return m_patterns[index];
        }
        return nullptr;
    }
    
    JsonLedPattern* getPatternByName(const String& name) {
        auto it = m_patternNameMap.find(name);
        if (it != m_patternNameMap.end()) {
            return m_patterns[it->second];
        }
        return nullptr;
    }
    
private:
    std::vector<JsonLedPattern*> m_patterns;
    std::map<String, int> m_patternNameMap;
};

#endif // JSON_LED_PATTERNS_H
