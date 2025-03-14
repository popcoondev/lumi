#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class NetworkManager {
private:
    String _ssid;
    String _password;
    bool _isConnected;
    unsigned long _lastReconnectAttempt;
    const unsigned long _reconnectInterval = 30000; // 30 seconds

    // WiFi設定ファイルのパス
    const char* _configPath = "/wifi_config.json";
    
    // WiFi設定をSPIFFSから読み込む
    bool loadWiFiConfig();
    
    // WiFi設定をSPIFFSに保存する
    bool saveWiFiConfig();

public:
    NetworkManager();
    ~NetworkManager();
    
    // 初期化
    bool begin();
    
    // WiFiに接続
    bool connect();
    
    // WiFi接続状態を確認
    bool isConnected();
    
    // WiFi接続を維持するための定期的な処理
    void update();
    
    // WiFi設定を更新
    bool updateConfig(const String& ssid, const String& password);
    
    // IPアドレスを取得
    IPAddress getLocalIP();
    
    // MACアドレスを取得
    String getMacAddress();
    
    // WiFi信号強度を取得
    int getRSSI();
};

#endif // NETWORK_MANAGER_H
