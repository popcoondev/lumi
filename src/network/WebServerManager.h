#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "../led/LEDManager.h"

class WebServerManager {
private:
    AsyncWebServer* _server;
    LEDManager* _ledManager;
    
    // APIエンドポイントのセットアップ
    void setupAPIEndpoints();
    
    // 静的ファイルのセットアップ
    void setupStaticFiles();
    
    // 404ハンドラのセットアップ
    void setupNotFoundHandler();

public:
    WebServerManager(LEDManager* ledManager);
    ~WebServerManager();
    
    // 初期化
    bool begin();
    
    // サーバーを開始
    void start();
    
    // サーバーを停止
    void stop();
};

#endif // WEB_SERVER_MANAGER_H
