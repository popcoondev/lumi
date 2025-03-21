#include "NetworkSettingsActivity.h"
#include <M5Unified.h>
#include <WiFiType.h>

NetworkSettingsActivity::NetworkSettingsActivity()
    : Activity(500, "NetworkSettingsActivity"), // Use a unique ID for this activity
      m_networkManager(nullptr),
      m_homeButton(nullptr),
      m_ssidTextView(nullptr),
      m_ipAddressTextView(nullptr),
      m_macAddressTextView(nullptr),
      m_signalStrengthTextView(nullptr),
      m_modeTextView(nullptr),
      m_apSSIDTextView(nullptr),
      m_apIPTextView(nullptr),
      m_toggleModeButton(nullptr),
      m_lastUpdateTime(0)
{
}

NetworkSettingsActivity::~NetworkSettingsActivity() {
    delete m_homeButton;
    delete m_ssidTextView;
    delete m_ipAddressTextView;
    delete m_macAddressTextView;
    delete m_signalStrengthTextView;
    delete m_modeTextView;
    delete m_apSSIDTextView;
    delete m_apIPTextView;
    delete m_toggleModeButton;
}

bool NetworkSettingsActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    
    // ButtonFragment インスタンスの作成
    m_homeButton = new ButtonFragment(ID_BUTTON_HOME);
    m_homeButton->onCreate();
    
    m_toggleModeButton = new ButtonFragment(ID_BUTTON_TOGGLE_MODE);
    m_toggleModeButton->onCreate();
    
    // TextView インスタンスの作成 - STAモード
    m_ssidTextView = new TextView();
    m_ipAddressTextView = new TextView();
    m_macAddressTextView = new TextView();
    m_signalStrengthTextView = new TextView();
    
    // TextView インスタンスの作成 - APモード
    m_modeTextView = new TextView();
    m_apSSIDTextView = new TextView();
    m_apIPTextView = new TextView();
    
    // TextViewの初期化 - STAモード
    m_ssidTextView->begin();
    m_ipAddressTextView->begin();
    m_macAddressTextView->begin();
    m_signalStrengthTextView->begin();
    
    // TextViewの初期化 - APモード
    m_modeTextView->begin();
    m_apSSIDTextView->begin();
    m_apIPTextView->begin();
    
    return true;
}

void NetworkSettingsActivity::initialize(NetworkManager* networkManager) {
    Serial.println("NetworkSettingsActivity::initialize called");
    m_networkManager = networkManager;
    
    if (!m_networkManager) {
        Serial.println("ERROR: NetworkManager is null in NetworkSettingsActivity::initialize");
    }
    
    // 共通設定
    int textWidth = 280;
    int textHeight = 30;
    int textStartY = 50;
    int textSpacing = 10;
    int buttonWidth = 200;
    int buttonHeight = 40;
    
    // モード表示用TextView
    m_modeTextView->setPosition(20, textStartY, textWidth, textHeight);
    m_modeTextView->setFontSize(2);
    m_modeTextView->setColor(TFT_YELLOW);
    m_modeTextView->setBackgroundColor(TFT_BLACK);
    
    // STAモード用のTextView
    m_ssidTextView->setPosition(20, textStartY + textHeight + textSpacing, textWidth, textHeight);
    m_ssidTextView->setFontSize(2);
    m_ssidTextView->setColor(TFT_WHITE);
    m_ssidTextView->setBackgroundColor(TFT_BLACK);
    
    m_ipAddressTextView->setPosition(20, textStartY + 2 * (textHeight + textSpacing), textWidth, textHeight);
    m_ipAddressTextView->setFontSize(2);
    m_ipAddressTextView->setColor(TFT_WHITE);
    m_ipAddressTextView->setBackgroundColor(TFT_BLACK);
    
    m_macAddressTextView->setPosition(20, textStartY + 3 * (textHeight + textSpacing), textWidth, textHeight);
    m_macAddressTextView->setFontSize(2);
    m_macAddressTextView->setColor(TFT_WHITE);
    m_macAddressTextView->setBackgroundColor(TFT_BLACK);
    
    m_signalStrengthTextView->setPosition(20, textStartY + 4 * (textHeight + textSpacing), textWidth, textHeight);
    m_signalStrengthTextView->setFontSize(2);
    m_signalStrengthTextView->setColor(TFT_WHITE);
    m_signalStrengthTextView->setBackgroundColor(TFT_BLACK);
    
    // APモード用のTextView
    m_apSSIDTextView->setPosition(20, textStartY + textHeight + textSpacing, textWidth, textHeight);
    m_apSSIDTextView->setFontSize(2);
    m_apSSIDTextView->setColor(TFT_WHITE);
    m_apSSIDTextView->setBackgroundColor(TFT_BLACK);
    
    m_apIPTextView->setPosition(20, textStartY + 2 * (textHeight + textSpacing), textWidth, textHeight);
    m_apIPTextView->setFontSize(2);
    m_apIPTextView->setColor(TFT_WHITE);
    m_apIPTextView->setBackgroundColor(TFT_BLACK);
    
    // ホームボタン
    m_homeButton->setDisplayArea(60, textStartY + 6 * (textHeight + textSpacing), buttonWidth, buttonHeight);
    m_homeButton->setLabel("Back to Settings");
    m_homeButton->setColor(BLACK, TFT_LIGHTGREY);
    m_homeButton->setFontSize(1.5);
    m_homeButton->setType(BUTTON_TYPE_TEXT);
    
    // モード切替ボタン
    m_toggleModeButton->setDisplayArea(60, textStartY + 5 * (textHeight + textSpacing), buttonWidth, buttonHeight);
    m_toggleModeButton->setLabel("Toggle WiFi Mode");
    m_toggleModeButton->setColor(BLACK, TFT_CYAN);
    m_toggleModeButton->setFontSize(1.5);
    m_toggleModeButton->setType(BUTTON_TYPE_TEXT);
    
    // クリックハンドラの設定
    m_homeButton->setClickHandler([this]() {
        if (onHomeRequested) {
            onHomeRequested();
        }
    });
    
    m_toggleModeButton->setClickHandler([this]() {
        toggleWiFiMode();
    });
    
    // Fragmentの追加
    addFragment(m_homeButton, "homeButton");
    addFragment(m_toggleModeButton, "toggleModeButton");
    
    // 初回更新
    update();
}

bool NetworkSettingsActivity::onStart() {
    if (!Activity::onStart()) {
        return false;
    }
    return true;
}

bool NetworkSettingsActivity::onResume() {
    if (!Activity::onResume()) {
        return false;
    }
    
    // NetworkManagerが設定されていることを確認
    if (!m_networkManager) {
        Serial.println("ERROR: NetworkManager is null in NetworkSettingsActivity::onResume");
    }
    
    // 画面を描画
    draw();
    
    // 最終更新時間をリセット
    m_lastUpdateTime = 0;
    
    // 強制的に情報を更新
    update();
    
    return true;
}

void NetworkSettingsActivity::onPause() {
    Activity::onPause();
}

void NetworkSettingsActivity::onStop() {
    Activity::onStop();
}

void NetworkSettingsActivity::onDestroy() {
    Activity::onDestroy();
}

bool NetworkSettingsActivity::handleEvent(const framework::Event& event) {
    // イベントタイプを確認
    if (event.getType() == framework::EventType::TOUCH) {
        const framework::TouchEvent& touchEvent = static_cast<const framework::TouchEvent&>(event);
        
        // タッチイベントの詳細をログ出力
        Serial.print("NetworkSettingsActivity: Touch event received - ");
        Serial.print("x=");
        Serial.print(touchEvent.getX());
        Serial.print(", y=");
        Serial.print(touchEvent.getY());
        Serial.print(", action=");
        Serial.println(static_cast<int>(touchEvent.getAction()));
    }
    
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
    }
    
    // ボタンのイベント処理を明示的に呼び出す
    if (event.getType() == framework::EventType::TOUCH) {
        const framework::TouchEvent& touchEvent = static_cast<const framework::TouchEvent&>(event);
        
        // ホームボタンのイベント処理
        if (m_homeButton && m_homeButton->handleEvent(touchEvent)) {
            Serial.println("NetworkSettingsActivity: Home button handled the event");
            return true;
        }
        
        // モード切替ボタンのイベント処理
        if (m_toggleModeButton && m_toggleModeButton->handleEvent(touchEvent)) {
            Serial.println("NetworkSettingsActivity: Toggle mode button handled the event");
            return true;
        }
    }
    
    return false;
}

void NetworkSettingsActivity::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // タイトルを描画
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString("Network Settings", 160, 20);
    
    // モード表示
    m_modeTextView->draw();
    
    // 現在のモードに応じて表示を切り替え
    if (m_networkManager && m_networkManager->getCurrentMode() == (WiFiMode_t)2) { // WIFI_AP = 2
        // APモードの情報を表示
        m_apSSIDTextView->draw();
        m_apIPTextView->draw();
    } else {
        // STAモードの情報を表示
        m_ssidTextView->draw();
        m_ipAddressTextView->draw();
        m_macAddressTextView->draw();
        m_signalStrengthTextView->draw();
    }
    
    // ボタンを描画
    m_homeButton->draw();
    m_toggleModeButton->draw();
    
    // デバッグ出力
    Serial.println("Network settings drawn");
}

void NetworkSettingsActivity::update() {
    unsigned long currentTime = millis();
    
    // 一定間隔でネットワーク情報を更新
    if (currentTime - m_lastUpdateTime >= m_updateInterval || m_lastUpdateTime == 0) {
        m_lastUpdateTime = currentTime;
        
        // デバッグ出力
        Serial.println("Updating network information...");
        
        if (m_networkManager) {
            // 現在のWiFiモードを表示
            WiFiMode_t currentMode = m_networkManager->getCurrentMode();
            String modeStr = (currentMode == (WiFiMode_t)2) ? "Access Point (AP)" : "Station (STA)"; // WIFI_AP = 2
            m_modeTextView->setText("Mode: " + modeStr);
            
            // モードに応じた情報を更新
            if (currentMode == (WiFiMode_t)2) { // WIFI_AP = 2
                updateAPModeInfo();
            } else {
                updateSTAModeInfo();
            }
        } else {
            Serial.println("NetworkManager not available");
            m_modeTextView->setText("Mode: NetworkManager not available");
            m_ssidTextView->setText("SSID: NetworkManager not available");
            m_ipAddressTextView->setText("IP: NetworkManager not available");
            m_macAddressTextView->setText("MAC: NetworkManager not available");
            m_signalStrengthTextView->setText("Signal: NetworkManager not available");
        }
    }
}

// STAモード用の情報表示
void NetworkSettingsActivity::updateSTAModeInfo() {
    // WiFi SSID
    String ssid = WiFi.SSID();
    Serial.print("SSID: ");
    Serial.println(ssid);
    m_ssidTextView->setText("SSID: " + (ssid.length() > 0 ? ssid : "Not connected"));
    
    // IP アドレス
    IPAddress ip = m_networkManager->getLocalIP();
    String ipStr = ip.toString();
    Serial.print("IP: ");
    Serial.println(ipStr);
    m_ipAddressTextView->setText("IP: " + (ipStr != "0.0.0.0" ? ipStr : "Not assigned"));
    
    // MAC アドレス
    String mac = m_networkManager->getMacAddress();
    Serial.print("MAC: ");
    Serial.println(mac);
    m_macAddressTextView->setText("MAC: " + mac);
    
    // 信号強度
    int rssi = m_networkManager->getRSSI();
    String rssiStr;
    if (rssi <= 0 && rssi > -50) {
        rssiStr = "Excellent";
    } else if (rssi <= -50 && rssi > -70) {
        rssiStr = "Good";
    } else if (rssi <= -70 && rssi > -80) {
        rssiStr = "Fair";
    } else if (rssi <= -80) {
        rssiStr = "Weak";
    } else {
        rssiStr = "Unknown";
    }
    Serial.print("RSSI: ");
    Serial.println(rssi);
    m_signalStrengthTextView->setText("Signal: " + rssiStr + " (" + String(rssi) + " dBm)");
}

// APモード用の情報表示
void NetworkSettingsActivity::updateAPModeInfo() {
    // AP SSID
    String apSSID = WiFi.softAPSSID();
    Serial.print("AP SSID: ");
    Serial.println(apSSID);
    m_apSSIDTextView->setText("AP SSID: " + apSSID);
    
    // AP IP アドレス
    IPAddress apIP = m_networkManager->getAPIP();
    String apIPStr = apIP.toString();
    Serial.print("AP IP: ");
    Serial.println(apIPStr);
    m_apIPTextView->setText("AP IP: " + apIPStr);
}

// WiFiモード切替処理
void NetworkSettingsActivity::toggleWiFiMode() {
    if (!m_networkManager) {
        Serial.println("NetworkManager not available");
        return;
    }
    
    WiFiMode_t currentMode = m_networkManager->getCurrentMode();
    bool success = false;
    
    if (currentMode == (WiFiMode_t)1) { // WIFI_STA = 1
        // STAモードからAPモードに切り替え
        Serial.println("Switching from STA to AP mode");
        success = m_networkManager->setMode((WiFiMode_t)2); // WIFI_AP = 2
    } else {
        // APモードからSTAモードに切り替え
        Serial.println("Switching from AP to STA mode");
        success = m_networkManager->setMode((WiFiMode_t)1); // WIFI_STA = 1
    }
    
    if (success) {
        Serial.println("WiFi mode switched successfully");
    } else {
        Serial.println("Failed to switch WiFi mode");
    }
    
    // 設定を保存
    m_networkManager->saveWiFiConfig();
    
    // 画面を更新
    draw();
    update();
}
