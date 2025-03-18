#include "NetworkSettingsActivity.h"
#include <M5Unified.h>

NetworkSettingsActivity::NetworkSettingsActivity()
    : Activity(0, "NetworkSettingsActivity"),
      m_networkManager(nullptr),
      m_homeButton(nullptr),
      m_ssidTextView(nullptr),
      m_ipAddressTextView(nullptr),
      m_macAddressTextView(nullptr),
      m_signalStrengthTextView(nullptr),
      m_lastUpdateTime(0)
{
}

NetworkSettingsActivity::~NetworkSettingsActivity() {
    delete m_homeButton;
    delete m_ssidTextView;
    delete m_ipAddressTextView;
    delete m_macAddressTextView;
    delete m_signalStrengthTextView;
}

bool NetworkSettingsActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    
    // ButtonFragment インスタンスの作成
    m_homeButton = new ButtonFragment(ID_BUTTON_HOME);
    m_homeButton->onCreate();
    
    // TextView インスタンスの作成
    m_ssidTextView = new TextView();
    m_ipAddressTextView = new TextView();
    m_macAddressTextView = new TextView();
    m_signalStrengthTextView = new TextView();
    
    // TextViewの初期化
    m_ssidTextView->begin();
    m_ipAddressTextView->begin();
    m_macAddressTextView->begin();
    m_signalStrengthTextView->begin();
    
    return true;
}

void NetworkSettingsActivity::initialize(NetworkManager* networkManager) {
    m_networkManager = networkManager;
    
    // ボタンの位置とサイズを設定
    int buttonWidth = 200;
    int buttonHeight = 40;
    int startY = 200;
    
    m_homeButton->setDisplayArea(60, startY, buttonWidth, buttonHeight);
    
    // ボタンのスタイル設定
    m_homeButton->setLabel("Back to Settings");
    m_homeButton->setColor(BLACK, TFT_LIGHTGREY);
    m_homeButton->setFontSize(1.5);
    m_homeButton->setType(BUTTON_TYPE_TEXT);
    
    // クリックハンドラの設定
    m_homeButton->setClickHandler([this]() {
        if (onHomeRequested) {
            onHomeRequested();
        }
    });
    
    // TextViewの位置とスタイル設定
    int textWidth = 280;
    int textHeight = 30;
    int textStartY = 50;
    int textSpacing = 10;
    
    m_ssidTextView->setPosition(20, textStartY, textWidth, textHeight);
    m_ssidTextView->setFontSize(2);
    m_ssidTextView->setColor(TFT_WHITE);
    m_ssidTextView->setBackgroundColor(TFT_BLACK);
    
    m_ipAddressTextView->setPosition(20, textStartY + textHeight + textSpacing, textWidth, textHeight);
    m_ipAddressTextView->setFontSize(2);
    m_ipAddressTextView->setColor(TFT_WHITE);
    m_ipAddressTextView->setBackgroundColor(TFT_BLACK);
    
    m_macAddressTextView->setPosition(20, textStartY + 2 * (textHeight + textSpacing), textWidth, textHeight);
    m_macAddressTextView->setFontSize(2);
    m_macAddressTextView->setColor(TFT_WHITE);
    m_macAddressTextView->setBackgroundColor(TFT_BLACK);
    
    m_signalStrengthTextView->setPosition(20, textStartY + 3 * (textHeight + textSpacing), textWidth, textHeight);
    m_signalStrengthTextView->setFontSize(2);
    m_signalStrengthTextView->setColor(TFT_WHITE);
    m_signalStrengthTextView->setBackgroundColor(TFT_BLACK);
    
    // Fragmentの追加
    addFragment(m_homeButton, "homeButton");
    
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
    
    // 画面を描画
    draw();
    
    // 最終更新時間をリセット
    m_lastUpdateTime = 0;
    
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
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
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
    
    // ネットワーク情報を表示
    m_ssidTextView->draw();
    m_ipAddressTextView->draw();
    m_macAddressTextView->draw();
    m_signalStrengthTextView->draw();
    
    // ボタンを描画
    m_homeButton->draw();
    
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
            
        } else {
            Serial.println("NetworkManager not available");
            m_ssidTextView->setText("SSID: NetworkManager not available");
            m_ipAddressTextView->setText("IP: NetworkManager not available");
            m_macAddressTextView->setText("MAC: NetworkManager not available");
            m_signalStrengthTextView->setText("Signal: NetworkManager not available");
        }
        
    }
    Serial.println("Network settings updated");
}
