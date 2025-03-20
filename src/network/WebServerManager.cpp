#include "WebServerManager.h"

WebServerManager::WebServerManager(LEDManager* ledManager) {
    _server = new AsyncWebServer(80);
    _ledManager = ledManager;
}

WebServerManager::~WebServerManager() {
    delete _server;
}

bool WebServerManager::begin() {
    // SPIFFSが初期化されていることを確認
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return false;
    }
    
    // APIエンドポイントのセットアップ
    setupAPIEndpoints();
    
    // 静的ファイルのセットアップ
    setupStaticFiles();
    
    // 404ハンドラのセットアップ
    setupNotFoundHandler();
    
    return true;
}

void WebServerManager::start() {
    _server->begin();
    Serial.println("Web server started!");
}

void WebServerManager::stop() {
    _server->end();
    Serial.println("Web server stopped!");
}

void WebServerManager::setupAPIEndpoints() {
    // ステータスAPI
    _server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Serial.println("[API] Status API called");
        StaticJsonDocument<256> doc;
        doc["status"] = "ok";
        doc["device"] = "Lumi";
        doc["uptime"] = millis() / 1000;
        
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    
    // LED制御API - 特定の面のLEDを制御（正規表現を使わない方法）
    _server->on("/api/led/face/0", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 0 control requested");
        handleLedFaceControl(0, request);
    });
    _server->on("/api/led/face/1", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 1 control requested");
        handleLedFaceControl(1, request);
    });
    _server->on("/api/led/face/2", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 2 control requested");
        handleLedFaceControl(2, request);
    });
    _server->on("/api/led/face/3", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 3 control requested");
        handleLedFaceControl(3, request);
    });
    _server->on("/api/led/face/4", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 4 control requested");
        handleLedFaceControl(4, request);
    });
    _server->on("/api/led/face/5", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 5 control requested");
        handleLedFaceControl(5, request);
    });
    _server->on("/api/led/face/6", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 6 control requested");
        handleLedFaceControl(6, request);
    });
    _server->on("/api/led/face/7", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED face 7 control requested");
        handleLedFaceControl(7, request);
    });
    
    // LED制御API - パターンを実行（正規表現を使わない方法）
    for (int i = 0; i < _ledManager->getPatternCount(); i++) {
        String path = "/api/led/pattern/" + String(i);
        _server->on(path.c_str(), HTTP_POST, [this, i](AsyncWebServerRequest *request) {
            handlePatternControl(i, request);
        });
    }
    
    // LED制御API - パターンを停止
    _server->on("/api/led/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED stop API called");
        // パターンを停止
        _ledManager->stopPattern();
        
        StaticJsonDocument<256> doc;
        doc["status"] = "ok";
        
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    
    // LED制御API - 全てのLEDをリセット
    _server->on("/api/led/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED reset API called");
        // 全てのLEDをリセット
        _ledManager->resetAllLeds();
        
        StaticJsonDocument<256> doc;
        doc["status"] = "ok";
        
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    
    // LED制御API - 利用可能なパターンのリストを取得
    _server->on("/api/led/patterns", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("[API] LED patterns API called");
        StaticJsonDocument<1024> doc;
        JsonArray patterns = doc.createNestedArray("patterns");
        
        for (int i = 0; i < _ledManager->getPatternCount(); i++) {
            JsonObject pattern = patterns.createNestedObject();
            pattern["id"] = i;
            pattern["name"] = _ledManager->getPatternName(i);
        }
        
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    
    // LED制御API - JSONパターンを受け取って実行
    _server->on("/api/led/pattern/json", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            // 最初のフレーム
            Serial.println("[API] JSON LED pattern API called");
        }
        
        if (index + len == total) {
            // 最後のフレーム
            Serial.println("[API] JSON LED pattern API received all data");
            Serial.println("[API] JSON data: " + String((char*)data));
            Serial.println("[API] JSON data length: " + String(len));
            Serial.println("[API] JSON data total length: " + String(total));
            Serial.println("[API] JSON data index: " + String(index));

            handleJsonPatternControl(request, data, total);
        }
    });
}


void WebServerManager::handleLedFaceControl(int faceId, AsyncWebServerRequest *request) {
    Serial.println("[API] LED control API called for face: " + String(faceId));
    
    // クエリパラメータからRGB値を取得
    int r = 255, g = 0, b = 0; // デフォルト値
    
    // URLクエリパラメータを取得するため、第2引数をfalseに変更
    if (request->hasParam("r", false)) {
        r = request->getParam("r", false)->value().toInt();
        Serial.println("[API] Received r parameter: " + String(r));
    } else {
        Serial.println("[API] No r parameter found, using default: " + String(r));
    }
    
    if (request->hasParam("g", false)) {
        g = request->getParam("g", false)->value().toInt();
        Serial.println("[API] Received g parameter: " + String(g));
    } else {
        Serial.println("[API] No g parameter found, using default: " + String(g));
    }
    
    if (request->hasParam("b", false)) {
        b = request->getParam("b", false)->value().toInt();
        Serial.println("[API] Received b parameter: " + String(b));
    } else {
        Serial.println("[API] No b parameter found, using default: " + String(b));
    }
    
    // 値の範囲を制限
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);
    
    Serial.println("[API] Setting face " + String(faceId) + " to RGB: " + String(r) + "," + String(g) + "," + String(b));
    
    // LEDを点灯
    _ledManager->lightFace(faceId, CRGB(r, g, b));
    
    Serial.println("[API] LED face " + String(faceId) + " color set completed");
    
    StaticJsonDocument<256> doc;
    doc["status"] = "ok";
    doc["face"] = faceId;
    doc["color"]["r"] = r;
    doc["color"]["g"] = g;
    doc["color"]["b"] = b;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServerManager::handlePatternControl(int patternId, AsyncWebServerRequest *request) {
    Serial.println("[API] LED pattern API called for pattern: " + String(patternId));
    
    // パターンを実行
    _ledManager->runPattern(patternId);
    
    Serial.println("[API] Pattern " + String(patternId) + " (" + _ledManager->getPatternName(patternId) + ") started");
    
    StaticJsonDocument<256> doc;
    doc["status"] = "ok";
    doc["pattern"] = patternId;
    doc["name"] = _ledManager->getCurrentPatternName();
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServerManager::setupStaticFiles() {
    // 静的ファイルのルートハンドラ
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    
    // CSSファイル
    _server->on("/css/*", HTTP_GET, [](AsyncWebServerRequest *request) {
        String path = request->url();
        request->send(SPIFFS, path, "text/css");
    });
    
    // JavaScriptファイル
    _server->on("/js/*", HTTP_GET, [](AsyncWebServerRequest *request) {
        String path = request->url();
        request->send(SPIFFS, path, "application/javascript");
    });
    
    // 画像ファイル
    _server->on("/images/*", HTTP_GET, [](AsyncWebServerRequest *request) {
        String path = request->url();
        request->send(SPIFFS, path, "image/jpeg");
    });
    
    // Markdownファイル
    _server->on("/README_JSON_LED_PATTERNS.md", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/README_JSON_LED_PATTERNS.md", "text/markdown");
    });
}

void WebServerManager::setupNotFoundHandler() {
    // 404ハンドラ
    _server->onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
}

void WebServerManager::handleJsonPatternControl(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    Serial.println("handleJsonPatternControl");
    
    // JSONデータを文字列に変換
    String jsonString = "";
    for (size_t i = 0; i < len; i++) {
        jsonString += (char)data[i];
    }
    
    // 受信したJSONデータをログに出力（デバッグ用）
    Serial.println("Received JSON data:");
    Serial.println(jsonString);
    
    // JSONの検証と修正
    DynamicJsonDocument doc(8192); // サイズを増やして大きなJSONにも対応
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        Serial.print(F("JSON parsing failed: "));
        Serial.println(error.c_str());
        
        // エラーの詳細情報を出力
        Serial.println("Error details: " + String(error.c_str()));
        
        StaticJsonDocument<256> response;
        response["status"] = "error";
        response["message"] = String("JSON parsing failed: ") + error.c_str();
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(400, "application/json", responseStr);
        return;
    }
    
    // JSONの構造を検証
    bool isValid = true;
    String errorMessage = "";
    
    // 必須フィールドの検証
    if (!doc.containsKey("type")) {
        isValid = false;
        errorMessage = "Missing required field: type";
    } else if (!doc.containsKey("parameters")) {
        isValid = false;
        errorMessage = "Missing required field: parameters";
    } else if (!doc.containsKey("steps") || !doc["steps"].is<JsonArray>()) {
        isValid = false;
        errorMessage = "Missing or invalid field: steps (must be an array)";
    } else {
        // stepsの検証
        JsonArray steps = doc["steps"].as<JsonArray>();
        if (steps.size() == 0) {
            isValid = false;
            errorMessage = "Steps array is empty";
        } else {
            // 各ステップの検証
            for (size_t i = 0; i < steps.size(); i++) {
                JsonObject step = steps[i];
                if (!step.containsKey("faceSelection") && !step.containsKey("faces")) {
                    isValid = false;
                    errorMessage = "Step " + String(i) + " is missing faceSelection or faces";
                    break;
                }
                // facesもfaceSelectionも指定されておらず、かつcolorHSVも指定されていない場合のみエラー
                if (!step.containsKey("colorHSV") && !step.containsKey("faces") && !step.containsKey("faceSelection")) {
                    isValid = false;
                    errorMessage = "Step " + String(i) + " is missing colorHSV and face selection method";
                    break;
                }
                if (!step.containsKey("duration")) {
                    isValid = false;
                    errorMessage = "Step " + String(i) + " is missing duration";
                    break;
                }
            }
        }
    }
    
    if (!isValid) {
        Serial.println("JSON validation failed: " + errorMessage);
        
        StaticJsonDocument<256> response;
        response["status"] = "error";
        response["message"] = "JSON validation failed: " + errorMessage;
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(400, "application/json", responseStr);
        return;
    }
    
    // 検証済みのJSONを再シリアライズして整形
    String formattedJson;
    serializeJson(doc, formattedJson);
    
    // 受信したJSONをファイルに保存
    const String jsonFilePath = "/received_json_pattern.json";
    
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        
        StaticJsonDocument<256> response;
        response["status"] = "error";
        response["message"] = "Failed to mount SPIFFS";
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(500, "application/json", responseStr);
        return;
    }
    
    // ファイルを書き込みモードで開く
    File file = SPIFFS.open(jsonFilePath, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        
        StaticJsonDocument<256> response;
        response["status"] = "error";
        response["message"] = "Failed to open file for writing";
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(500, "application/json", responseStr);
        return;
    }
    
    // 整形されたJSONデータをファイルに書き込む
    size_t bytesWritten = file.print(formattedJson);
    file.close();
    
    Serial.println("JSON pattern saved to file: " + jsonFilePath);
    Serial.println("Bytes written: " + String(bytesWritten));
    
    // LEDManagerにJSONパターンを読み込ませて実行
    bool success = _ledManager->runJsonPatternFromFile(jsonFilePath);
    
    if (success) {
        // パターン名を取得（あれば）
        String patternName = "Custom Pattern";
        if (doc.containsKey("name")) {
            patternName = doc["name"].as<String>();
        }
        
        StaticJsonDocument<256> response;
        response["status"] = "ok";
        response["pattern"] = patternName;
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(200, "application/json", responseStr);
    } else {
        StaticJsonDocument<256> response;
        response["status"] = "error";
        response["message"] = "Failed to load and run JSON pattern";
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(400, "application/json", responseStr);
    }
}
