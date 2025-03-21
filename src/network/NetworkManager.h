#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiType.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

class NetworkManager {
private:
    // STAモード用の変数
    String _ssid;
    String _password;
    bool _isConnected;
    unsigned long _lastReconnectAttempt;
    const unsigned long _reconnectInterval = 30000; // 30 seconds

    // APモード用の変数
    String _apSSID;
    String _apPassword;
    int _apChannel;
    bool _apHidden;
    int _apMaxConnections;
    WiFiMode_t _currentMode;  // WIFI_STA または WIFI_AP

    // WiFi設定ファイルのパス
    const char* _configPath = "/wifi_config.json";
    
    // mDNSが初期化されているかのフラグ
    bool _mdnsInitialized;
    
    // WiFi設定をSPIFFSから読み込む
    bool loadWiFiConfig();

public:
    NetworkManager();
    ~NetworkManager();
    
    // 初期化
    bool begin();
    
    // WiFiに接続 (STAモード)
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
    
    // APモード関連のメソッド
    bool startAP();
    bool stopAP();
    bool isAPActive();
    IPAddress getAPIP();
    WiFiMode_t getCurrentMode();
    bool setMode(WiFiMode_t mode);
    bool updateAPConfig(const String& ssid, const String& password, int channel = 1, bool hidden = false, int maxConnections = 4);
    
    // mDNS関連のメソッド
    bool startMDNS(const char* hostname);
    
    // WiFi設定をSPIFFSに保存する
    bool saveWiFiConfig();
};

#endif // NETWORK_MANAGER_H
