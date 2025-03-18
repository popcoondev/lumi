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
        Serial.println("[API] Status API called");
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
}

void WebServerManager::setupNotFoundHandler() {
    // 404ハンドラ
    _server->onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
}
