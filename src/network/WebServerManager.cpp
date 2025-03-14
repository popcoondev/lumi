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
        StaticJsonDocument<256> doc;
        doc["status"] = "ok";
        doc["device"] = "Lumi";
        doc["uptime"] = millis() / 1000;
        
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    
    // LED制御API - 特定の面のLEDを制御
    _server->on("^\\/api\\/led\\/face\\/([0-9]+)$", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String faceIdStr = request->pathArg(0);
        int faceId = faceIdStr.toInt();
        
        if (faceId < 0 || faceId >= 8) {
            request->send(400, "application/json", "{\"error\":\"Invalid face ID\"}");
            return;
        }
        
        // クエリパラメータからRGB値を取得
        int r = 255, g = 0, b = 0; // デフォルト値
        
        if (request->hasParam("r")) {
            r = request->getParam("r")->value().toInt();
        }
        
        if (request->hasParam("g")) {
            g = request->getParam("g")->value().toInt();
        }
        
        if (request->hasParam("b")) {
            b = request->getParam("b")->value().toInt();
        }
        
        // 値の範囲を制限
        r = constrain(r, 0, 255);
        g = constrain(g, 0, 255);
        b = constrain(b, 0, 255);
        
        // LEDを点灯
        _ledManager->lightFace(faceId, CRGB(r, g, b));
        
        StaticJsonDocument<256> doc;
        doc["status"] = "ok";
        doc["face"] = faceId;
        doc["color"]["r"] = r;
        doc["color"]["g"] = g;
        doc["color"]["b"] = b;
        
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    
    // LED制御API - パターンを実行
    _server->on("^\\/api\\/led\\/pattern\\/([0-9]+)$", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String patternIdStr = request->pathArg(0);
        int patternId = patternIdStr.toInt();
        
        if (patternId < 0 || patternId >= _ledManager->getPatternCount()) {
            request->send(400, "application/json", "{\"error\":\"Invalid pattern ID\"}");
            return;
        }
        
        // パターンを実行
        _ledManager->runPattern(patternId);
        
        StaticJsonDocument<256> doc;
        doc["status"] = "ok";
        doc["pattern"] = patternId;
        doc["name"] = _ledManager->getCurrentPatternName();
        
        String response;
        serializeJson(doc, response);
        
        request->send(200, "application/json", response);
    });
    
    // LED制御API - パターンを停止
    _server->on("/api/led/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
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
