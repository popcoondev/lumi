#include "LEDManager.h"
#include "Constants.h"
#include <SPIFFS.h>

LEDManager::LEDManager() {
    leds = nullptr;
    numLeds = 0;
    ledOffset = 0;
    numFaces = 0;
    ledTaskHandle = nullptr;
    isTaskRunning = false;
    
    // JSONパターン関連の初期化
    m_isJsonPattern = false;
    m_currentJsonPatternIndex = 0;
    
    // パターンの初期化
    patternCount = 13; // 全パターン数
    patterns = new LedPattern*[patternCount];
    patterns[0] = new SequentialPattern();
    patterns[1] = new OnOffPattern();
    patterns[2] = new OddEvenPattern();
    patterns[3] = new RandomPattern();
    patterns[4] = new WavePattern();
    patterns[5] = new RainbowPattern();
    patterns[6] = new StrobePattern();
    patterns[7] = new ChasePattern();
    patterns[8] = new PulsePattern();
    patterns[9] = new TwinklePattern();
    patterns[10] = new FireFlickerPattern();
    patterns[11] = new CometPattern();
    patterns[12] = new IndividualRandomPattern();
    
    currentPatternIndex = 0;
}

LEDManager::~LEDManager() {
    if (leds != nullptr) {
        delete[] leds;
    }
    
    // パターンオブジェクトの解放
    for (int i = 0; i < patternCount; i++) {
        delete patterns[i];
    }
    delete[] patterns;
    
    // タスクの削除
    if (ledTaskHandle != nullptr) {
        vTaskDelete(ledTaskHandle);
    }
}

void LEDManager::begin(int pin, int numLeds, int ledOffset) {
    this->numLeds = numLeds;
    this->ledOffset = ledOffset;
    this->numFaces = MAX_FACES;
    
    // LEDストリップの初期化
    leds = new CRGB[numLeds];
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, numLeds);
    FastLED.setBrightness(255);
    
    // すべてのLEDを消灯
    resetAllLeds();
    
    // 初期状態ではタスクは作成しない（必要時に作成）
    ledTaskHandle = nullptr;
    isTaskRunning = false;
}

void LEDManager::ledTaskWrapper(void* parameter) {
    LEDManager* manager = static_cast<LEDManager*>(parameter);
    
    while (true) {
        // 現在選択されているパターンを実行
        manager->patterns[manager->currentPatternIndex]->run(
            manager->leds,
            manager->numLeds,
            manager->ledOffset,
            manager->numFaces,
            5000 // 各パターン5秒間実行
        );
        
        // // パターン実行後、自動的に一時停止
        // vTaskSuspend(NULL);
    }
    
    vTaskDelete(NULL);
}

void LEDManager::runPattern(int patternIndex) {
    if (patternIndex >= 0 && patternIndex < patternCount) {
        currentPatternIndex = patternIndex;
        
        // 既存のタスクがあれば削除
        if (ledTaskHandle != nullptr) {
            vTaskDelete(ledTaskHandle);
            ledTaskHandle = nullptr;
        }
        
        // 新しいタスクを作成
        xTaskCreatePinnedToCore(
            ledTaskWrapper,
            "LEDTask",
            4096,
            this,
            1,
            &ledTaskHandle,
            1
        );
        
        isTaskRunning = true;
    }
}

void LEDManager::stopPattern() {
    if (ledTaskHandle != nullptr) {
        // タスクを削除（一時停止ではなく完全停止）
        vTaskDelete(ledTaskHandle);
        ledTaskHandle = nullptr;
        isTaskRunning = false;
        
        // JSONパターンフラグをリセット
        m_isJsonPattern = false;
    }
    
    // パターン停止時にすべてのLEDを消灯
    resetAllLeds();
}

void LEDManager::lightFace(int faceId, CRGB color) {
    if (faceId >= 0 && faceId < numFaces) {
        int idx1 = ledOffset + (faceId * 2);
        int idx2 = ledOffset + (faceId * 2) + 1;
        
        leds[idx1] = color;
        leds[idx2] = color;
        FastLED.show();
    }
}

CRGB LEDManager::getFaceColor(int faceId) {
    if (faceId >= 0 && faceId < numFaces) {
        int idx = ledOffset + (faceId * 2);
        return leds[idx];
    }
    return CRGB::Black; // デフォルト値として黒（消灯状態）を返す
}

void LEDManager::resetAllLeds() {
    for (int i = 0; i < numLeds; i++) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
}

void LEDManager::nextPattern() {
    currentPatternIndex = (currentPatternIndex + 1) % patternCount;
}

void LEDManager::prevPattern() {
    currentPatternIndex = (currentPatternIndex - 1 + patternCount) % patternCount;
}

String LEDManager::getPatternName(int index) {
    if (index >= 0 && index < patternCount) {
        return patterns[index]->getName();
    }
    return "Unknown";
}

String LEDManager::getCurrentPatternName() {
    return getPatternName(currentPatternIndex);
}

int LEDManager::getCurrentPatternIndex() {
    return currentPatternIndex;
}

void LEDManager::setBrightness(uint8_t brightness) {
    this->brightness = brightness;
    FastLED.setBrightness(brightness);
    FastLED.show();
}

bool LEDManager::isPatternRunning() {
    // isTaskRunningフラグを使用して実行状態を判断
    return isTaskRunning && ledTaskHandle != nullptr;
}

// JSONパターンタスクラッパー
void LEDManager::jsonPatternTaskWrapper(void* parameter) {
    LEDManager* manager = static_cast<LEDManager*>(parameter);
    
    // JSONパターンを実行
    JsonLedPattern* pattern = manager->m_jsonPatternManager.getPatternByIndex(manager->m_currentJsonPatternIndex);
    if (pattern) {
        pattern->run(
            manager->leds,
            manager->numLeds,
            manager->ledOffset,
            manager->numFaces,
            0  // 無限実行（停止要求があるまで）
        );
    }
    
    // タスク終了時にフラグをリセット
    manager->isTaskRunning = false;
    manager->m_isJsonPattern = false;
    vTaskDelete(NULL);
}

// JSONパターン関連のメソッド
bool LEDManager::loadJsonPatternsFromFile(const String& filename) {
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return false;
    }
    
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open pattern index file");
        return false;
    }
    
    String jsonString = file.readString();
    file.close();
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }
    
    if (!doc.containsKey("patterns") || !doc["patterns"].is<JsonArray>()) {
        Serial.println("Invalid pattern index format");
        return false;
    }
    
    JsonArray patterns = doc["patterns"].as<JsonArray>();
    int count = 0;
    
    for (JsonObject pattern : patterns) {
        if (pattern.containsKey("name") && pattern.containsKey("file")) {
            String name = pattern["name"].as<String>();
            String filePath = pattern["file"].as<String>();
            
            // Load the pattern file
            File patternFile = SPIFFS.open(filePath, "r");
            if (patternFile) {
                String patternJson = patternFile.readString();
                patternFile.close();
                
                if (loadJsonPatternsFromString(patternJson)) {
                    count++;
                    Serial.printf("Loaded pattern: %s from %s\n", name.c_str(), filePath.c_str());
                } else {
                    Serial.printf("Failed to parse pattern: %s\n", name.c_str());
                }
            } else {
                Serial.printf("Failed to open pattern file: %s\n", filePath.c_str());
            }
        }
    }
    
    Serial.printf("Loaded %d JSON patterns from index file\n", count);
    return count > 0;
}

bool LEDManager::loadJsonPatternsFromString(const String& jsonString) {
    return m_jsonPatternManager.loadPatternsFromJson(jsonString);
}

bool LEDManager::loadJsonPatternsFromDirectory(const String& dirPath) {
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return false;
    }
    
    File root = SPIFFS.open(dirPath);
    if (!root) {
        Serial.println("Failed to open directory");
        return false;
    }
    
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return false;
    }
    
    // Create a JSON array to hold all patterns
    String combinedJson = "{\"patterns\":[";
    bool firstPattern = true;
    int count = 0;
    
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String path = file.name();
            if (path.endsWith(".json")) {
                String jsonString = file.readString();
                
                // Add comma if not the first pattern
                if (!firstPattern) {
                    combinedJson += ",";
                } else {
                    firstPattern = false;
                }
                
                // Add the pattern JSON to the array
                combinedJson += jsonString;
                count++;
            }
        }
        file = root.openNextFile();
    }
    
    combinedJson += "]}";
    
    Serial.printf("Loaded %d JSON patterns from directory\n", count);
    
    if (count > 0) {
        // Load all patterns at once
        bool success = loadJsonPatternsFromString(combinedJson);
        if (success) {
            Serial.println("JSON patterns loaded successfully");
        } else {
            Serial.println("Failed to load JSON patterns");
        }
        return success;
    }
    
    return false;
}

int LEDManager::getJsonPatternCount() {
    return m_jsonPatternManager.getPatternCount();
}

String LEDManager::getJsonPatternName(int index) {
    JsonLedPattern* pattern = m_jsonPatternManager.getPatternByIndex(index);
    if (pattern) {
        return pattern->getName();
    }
    return "Unknown";
}

void LEDManager::runJsonPattern(const String& patternName) {
    JsonLedPattern* pattern = m_jsonPatternManager.getPatternByName(patternName);
    if (pattern) {
        // 既存のタスクがあれば削除
        if (ledTaskHandle != nullptr) {
            vTaskDelete(ledTaskHandle);
            ledTaskHandle = nullptr;
        }
        
        // JSONパターンフラグを設定
        m_isJsonPattern = true;
        
        // 新しいタスクを作成
        xTaskCreatePinnedToCore(
            jsonPatternTaskWrapper,
            "JSONPatternTask",
            4096,
            this,
            1,
            &ledTaskHandle,
            1
        );
        
        isTaskRunning = true;
    }
}

void LEDManager::runJsonPatternByIndex(int index) {
    if (index >= 0 && index < m_jsonPatternManager.getPatternCount()) {
        m_currentJsonPatternIndex = index;
        
        // 既存のタスクがあれば削除
        if (ledTaskHandle != nullptr) {
            vTaskDelete(ledTaskHandle);
            ledTaskHandle = nullptr;
        }
        
        // JSONパターンフラグを設定
        m_isJsonPattern = true;
        
        // 新しいタスクを作成
        xTaskCreatePinnedToCore(
            jsonPatternTaskWrapper,
            "JSONPatternTask",
            4096,
            this,
            1,
            &ledTaskHandle,
            1
        );
        
        isTaskRunning = true;
    }
}

// 受信したJSONパターンをファイルから読み込んで実行する
bool LEDManager::runJsonPatternFromFile(const String& filename) {
    Serial.println("LEDManager: Loading JSON pattern from file: " + filename);
    
    if (!SPIFFS.begin(true)) {
        Serial.println("LEDManager: An error occurred while mounting SPIFFS");
        return false;
    }
    
    // ファイルが存在するか確認
    if (!SPIFFS.exists(filename)) {
        Serial.println("LEDManager: File not found: " + filename);
        return false;
    }
    
    // ファイルを開く
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("LEDManager: Failed to open file: " + filename);
        return false;
    }
    
    // ファイルの内容を読み込む
    String jsonString = file.readString();
    file.close();
    
    Serial.println("LEDManager: JSON pattern loaded from file, length: " + String(jsonString.length()));
    
    // JSONの検証（デバッグ用）
    DynamicJsonDocument docCheck(8192);
    DeserializationError errorCheck = deserializeJson(docCheck, jsonString);
    
    if (errorCheck) {
        Serial.println("LEDManager: JSON validation failed: " + String(errorCheck.c_str()));
        Serial.println("LEDManager: First 100 chars of JSON: " + jsonString.substring(0, 100) + "...");
        return false;
    }
    
    // JSONの構造を検証
    if (!docCheck.containsKey("type")) {
        Serial.println("LEDManager: Missing required field: type");
        return false;
    }
    
    if (!docCheck.containsKey("parameters")) {
        Serial.println("LEDManager: Missing required field: parameters");
        return false;
    }
    
    if (!docCheck.containsKey("steps") || !docCheck["steps"].is<JsonArray>()) {
        Serial.println("LEDManager: Missing or invalid field: steps (must be an array)");
        return false;
    }
    
    JsonArray steps = docCheck["steps"].as<JsonArray>();
    if (steps.size() == 0) {
        Serial.println("LEDManager: Steps array is empty");
        return false;
    }
    
    // パターン名を取得（あれば）
    String patternName = "Custom Pattern";
    if (docCheck.containsKey("name")) {
        patternName = docCheck["name"].as<String>();
    }
    
    // JSONパターンを読み込む
    Serial.println("LEDManager: Loading pattern into JsonPatternManager...");
    
    // 単一のパターンをラップして配列形式にする
    String wrappedJson = "{\"patterns\":[" + jsonString + "]}";
    
    bool success = m_jsonPatternManager.loadPatternsFromJson(wrappedJson);
    if (!success) {
        Serial.println("LEDManager: Failed to load JSON pattern");
        return false;
    }
    
    Serial.println("LEDManager: JSON pattern loaded successfully");
    
    // パターンを実行
    if (m_jsonPatternManager.getPatternCount() > 0) {
        m_currentJsonPatternIndex = 0; // 最初のパターンを使用
        
        // 既存のタスクがあれば削除
        if (ledTaskHandle != nullptr) {
            vTaskDelete(ledTaskHandle);
            ledTaskHandle = nullptr;
        }
        
        // JSONパターンフラグを設定
        m_isJsonPattern = true;
        
        // 新しいタスクを作成
        xTaskCreatePinnedToCore(
            jsonPatternTaskWrapper,
            "JSONPatternTask",
            4096,
            this,
            1,
            &ledTaskHandle,
            1
        );
        
        isTaskRunning = true;
        
        Serial.println("LEDManager: Running JSON pattern: " + patternName);
        return true;
    } else {
        Serial.println("LEDManager: No patterns were loaded");
        return false;
    }
}
