#include "NetworkManager.h"

NetworkManager::NetworkManager() {
    _isConnected = false;
    _lastReconnectAttempt = 0;
}

NetworkManager::~NetworkManager() {
    // WiFiを切断
    WiFi.disconnect(true);
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
    
    // WiFiモードをSTAに設定
    WiFi.mode(WIFI_STA);
    
    // WiFiに接続
    return connect();
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
    // 接続が切れた場合、一定間隔で再接続を試みる
    if (!isConnected() && millis() - _lastReconnectAttempt > _reconnectInterval) {
        _lastReconnectAttempt = millis();
        Serial.println("Attempting to reconnect to WiFi...");
        connect();
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
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        Serial.print("Failed to parse WiFi config file: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // 設定を読み込む
    _ssid = doc["ssid"].as<String>();
    _password = doc["password"].as<String>();
    
    Serial.println("WiFi configuration loaded successfully!");
    return true;
}

bool NetworkManager::saveWiFiConfig() {
    // JSONを作成
    StaticJsonDocument<512> doc;
    doc["ssid"] = _ssid;
    doc["password"] = _password;
    
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
