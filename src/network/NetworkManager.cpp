#include "NetworkManager.h"

NetworkManager::NetworkManager() {
    _isConnected = false;
    _lastReconnectAttempt = 0;
    
    // APモード用の変数を初期化
    _apSSID = "Lumi_" + WiFi.macAddress().substring(9);
    _apPassword = "lumipassword";
    _apChannel = 1;
    _apHidden = false;
    _apMaxConnections = 4;
    _currentMode = (WiFiMode_t)1; // WIFI_STA = 1
}

NetworkManager::~NetworkManager() {
    // 現在のモードに応じて適切に切断
    if (_currentMode == (WiFiMode_t)1) { // WIFI_STA = 1
        WiFi.disconnect(true);
    } else if (_currentMode == (WiFiMode_t)2) { // WIFI_AP = 2
        WiFi.softAPdisconnect(true);
    }
}

bool NetworkManager::begin() {
    // SPIFFSを初期化
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return false;
    }
    
    // WiFi設定を読み込む
    if (!loadWiFiConfig()) {
        Serial.println("Failed to load WiFi configuration!");
        return false;
    }
    
    // コンパイル時の設定に基づいてモードを強制的に設定
#ifdef LUMI_WIFI_MODE_AP
    Serial.println("Compiled for AP mode");
    _currentMode = (WiFiMode_t)2; // WIFI_AP = 2
    return startAP();
#elif defined(LUMI_WIFI_MODE_STA)
    Serial.println("Compiled for STA mode");
    _currentMode = (WiFiMode_t)1; // WIFI_STA = 1
    return connect();
#else
    // 設定ファイルに基づいてモードを選択
    Serial.println("Using mode from config file");
    if (_currentMode == (WiFiMode_t)2) { // WIFI_AP = 2
        return startAP();
    } else {
        return connect();
    }
#endif
}

bool NetworkManager::connect() {
    if (_ssid.length() == 0) {
        Serial.println("SSID is not set!");
        return false;
    }
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(_ssid);
    
    // 既存の接続を切断
    WiFi.disconnect(true);
    
    // WiFiに接続
    WiFi.begin(_ssid.c_str(), _password.c_str());
    
    // 接続を待機（最大20秒）
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        _isConnected = true;
        Serial.println("");
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        _isConnected = false;
        Serial.println("");
        Serial.println("WiFi connection failed!");
        return false;
    }
}

bool NetworkManager::isConnected() {
    _isConnected = (WiFi.status() == WL_CONNECTED);
    return _isConnected;
}

void NetworkManager::update() {
    // STAモードの場合のみ接続状態を確認
    if (_currentMode == (WiFiMode_t)1) { // WIFI_STA = 1
        // 接続が切れた場合、一定間隔で再接続を試みる
        if (!isConnected() && millis() - _lastReconnectAttempt > _reconnectInterval) {
            _lastReconnectAttempt = millis();
            Serial.println("Attempting to reconnect to WiFi...");
            connect();
        }
        
        // 接続状態の更新
        _isConnected = (WiFi.status() == WL_CONNECTED);
    }
}

bool NetworkManager::updateConfig(const String& ssid, const String& password) {
    _ssid = ssid;
    _password = password;
    
    // 設定を保存
    if (!saveWiFiConfig()) {
        Serial.println("Failed to save WiFi configuration!");
        return false;
    }
    
    // 新しい設定で接続
    return connect();
}

IPAddress NetworkManager::getLocalIP() {
    return WiFi.localIP();
}

String NetworkManager::getMacAddress() {
    return WiFi.macAddress();
}

int NetworkManager::getRSSI() {
    return WiFi.RSSI();
}

bool NetworkManager::loadWiFiConfig() {
    // 設定ファイルが存在するか確認
    if (!SPIFFS.exists(_configPath)) {
        Serial.println("WiFi config file does not exist!");
        return false;
    }
    
    // ファイルを開く
    File configFile = SPIFFS.open(_configPath, "r");
    if (!configFile) {
        Serial.println("Failed to open WiFi config file!");
        return false;
    }
    
    // ファイルサイズを確認
    size_t size = configFile.size();
    if (size > 1024) {
        Serial.println("WiFi config file is too large!");
        configFile.close();
        return false;
    }
    
    // JSONをパース
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        Serial.print("Failed to parse WiFi config file: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // 拡張された設定ファイルの読み込み
    if (doc.containsKey("sta")) {
        _ssid = doc["sta"]["ssid"].as<String>();
        _password = doc["sta"]["password"].as<String>();
    } else {
        // 後方互換性のため
        _ssid = doc["ssid"].as<String>();
        _password = doc["password"].as<String>();
    }
    
    // APモード設定の読み込み
    if (doc.containsKey("ap")) {
        _apSSID = doc["ap"]["ssid"].as<String>();
        _apPassword = doc["ap"]["password"].as<String>();
        _apChannel = doc["ap"]["channel"] | 1;
        _apHidden = doc["ap"]["hidden"] | false;
        _apMaxConnections = doc["ap"]["max_connections"] | 4;
    }
    
    // モード設定の読み込み
    String modeStr = doc["mode"] | "sta";
    if (modeStr == "ap") {
        _currentMode = (WiFiMode_t)2; // WIFI_AP = 2
    } else {
        _currentMode = (WiFiMode_t)1; // WIFI_STA = 1
    }
    
    Serial.println("WiFi configuration loaded successfully!");
    return true;
}

bool NetworkManager::saveWiFiConfig() {
    // JSONを作成
    StaticJsonDocument<1024> doc;
    
    // STA設定
    JsonObject sta = doc.createNestedObject("sta");
    sta["ssid"] = _ssid;
    sta["password"] = _password;
    
    // AP設定
    JsonObject ap = doc.createNestedObject("ap");
    ap["ssid"] = _apSSID;
    ap["password"] = _apPassword;
    ap["channel"] = _apChannel;
    ap["hidden"] = _apHidden;
    ap["max_connections"] = _apMaxConnections;
    
    // モード設定
    if (_currentMode == (WiFiMode_t)2) { // WIFI_AP = 2
        doc["mode"] = "ap";
    } else {
        doc["mode"] = "sta";
    }
    
    // ファイルを開く
    File configFile = SPIFFS.open(_configPath, "w");
    if (!configFile) {
        Serial.println("Failed to open WiFi config file for writing!");
        return false;
    }
    
    // JSONをシリアライズ
    if (serializeJson(doc, configFile) == 0) {
        Serial.println("Failed to write WiFi config to file!");
        configFile.close();
        return false;
    }
    
    configFile.close();
    Serial.println("WiFi configuration saved successfully!");
    return true;
}

// APモード関連のメソッド実装
bool NetworkManager::startAP() {
    // APモードを設定
    WiFi.mode(WIFI_AP);
    _currentMode = (WiFiMode_t)2; // WIFI_AP = 2
    
    // APを開始
    bool result = WiFi.softAP(_apSSID.c_str(), _apPassword.c_str(), _apChannel, _apHidden, _apMaxConnections);
    
    if (result) {
        Serial.println("AP started!");
        Serial.print("AP SSID: ");
        Serial.println(_apSSID);
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
        return true;
    } else {
        Serial.println("Failed to start AP!");
        return false;
    }
}

bool NetworkManager::stopAP() {
    if (_currentMode == (WiFiMode_t)2) { // WIFI_AP = 2
        // APモードからSTAモードに切り替え
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        _currentMode = (WiFiMode_t)1; // WIFI_STA = 1
        return connect();
    }
    return true;
}

bool NetworkManager::isAPActive() {
    return (_currentMode == (WiFiMode_t)2); // WIFI_AP = 2
}

IPAddress NetworkManager::getAPIP() {
    return WiFi.softAPIP();
}

WiFiMode_t NetworkManager::getCurrentMode() {
    return _currentMode;
}

bool NetworkManager::setMode(WiFiMode_t mode) {
    if (mode == _currentMode) {
        return true; // 既に設定されているモードと同じ
    }
    
    // モードに応じて処理を分岐
    switch (mode) {
        case WIFI_AP:
            return startAP();
        case WIFI_STA:
        default:
            return connect();
    }
}

bool NetworkManager::updateAPConfig(const String& ssid, const String& password, int channel, bool hidden, int maxConnections) {
    _apSSID = ssid;
    _apPassword = password;
    _apChannel = channel;
    _apHidden = hidden;
    _apMaxConnections = maxConnections;
    
    // 設定を保存
    if (!saveWiFiConfig()) {
        Serial.println("Failed to save WiFi AP configuration!");
        return false;
    }
    
    // 現在APモードの場合は再起動
    if (_currentMode == (WiFiMode_t)2) { // WIFI_AP = 2
        WiFi.softAPdisconnect(true);
        return startAP();
    }
    
    return true;
}
