#include "SettingsActivity.h"
#include <M5Unified.h>

SettingsActivity::SettingsActivity()
    : Activity(400, "SettingsActivity"), // Use a unique ID for this activity
      m_detectionButton(nullptr),
      m_calibrationButton(nullptr),
      m_ledControlButton(nullptr),
      m_networkSettingsButton(nullptr),
      m_homeButton(nullptr)
{
}

SettingsActivity::~SettingsActivity() {
    delete m_detectionButton;
    delete m_calibrationButton;
    delete m_ledControlButton;
    delete m_networkSettingsButton;
    delete m_homeButton;
}

bool SettingsActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    
    // ButtonFragment インスタンスの作成
    m_detectionButton = new ButtonFragment(ID_BUTTON_DETECTION);
    m_calibrationButton = new ButtonFragment(ID_BUTTON_CALIBRATION);
    m_ledControlButton = new ButtonFragment(ID_BUTTON_LED_CONTROL);
    m_networkSettingsButton = new ButtonFragment(ID_BUTTON_NETWORK_SETTINGS);
    m_homeButton = new ButtonFragment(ID_BUTTON_HOME);
    
    // ButtonFragment の作成
    m_detectionButton->onCreate();
    m_calibrationButton->onCreate();
    m_ledControlButton->onCreate();
    m_networkSettingsButton->onCreate();
    m_homeButton->onCreate();
    
    return true;
}

void SettingsActivity::initialize() {
    // ボタンの位置とサイズを設定
    int buttonWidth = 100;
    int buttonHeight = 40;
    int buttonSpacing = 10;
    int startY = 50;
    
    int buttonX1 = 60;
    int buttonX2 = buttonX1 + buttonWidth + buttonSpacing;
    
    m_detectionButton->setDisplayArea(buttonX1, startY, buttonWidth, buttonHeight);
    m_calibrationButton->setDisplayArea(buttonX2, startY, buttonWidth, buttonHeight);
    m_ledControlButton->setDisplayArea(buttonX1, startY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
    m_networkSettingsButton->setDisplayArea(buttonX2, startY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
    m_homeButton->setDisplayArea(buttonX1, startY + 2 * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight);
    
    // ボタンのスタイル設定
    m_detectionButton->setLabel("Detection");
    m_detectionButton->setColor(BLACK, TFT_LIGHTGREY);
    m_detectionButton->setFontSize(1.5);
    m_detectionButton->setType(BUTTON_TYPE_TEXT);
    
    m_calibrationButton->setLabel("Calibration");
    m_calibrationButton->setColor(BLACK, TFT_LIGHTGREY);
    m_calibrationButton->setFontSize(1.5);
    m_calibrationButton->setType(BUTTON_TYPE_TEXT);
    
    m_ledControlButton->setLabel("LED Control");
    m_ledControlButton->setColor(BLACK, TFT_LIGHTGREY);
    m_ledControlButton->setFontSize(1.5);
    m_ledControlButton->setType(BUTTON_TYPE_TEXT);
    
    m_networkSettingsButton->setLabel("Network");
    m_networkSettingsButton->setColor(BLACK, TFT_LIGHTGREY);
    m_networkSettingsButton->setFontSize(1.5);
    m_networkSettingsButton->setType(BUTTON_TYPE_TEXT);
    
    m_homeButton->setLabel("Back to Home");
    m_homeButton->setColor(BLACK, TFT_LIGHTGREY);
    m_homeButton->setFontSize(1.5);
    m_homeButton->setType(BUTTON_TYPE_TEXT);
    
    // クリックハンドラの設定
    m_detectionButton->setClickHandler([this]() {
        if (onDetectionRequested) {
            onDetectionRequested();
        }
    });
    
    m_calibrationButton->setClickHandler([this]() {
        if (onCalibrationRequested) {
            onCalibrationRequested();
        }
    });
    
    m_ledControlButton->setClickHandler([this]() {
        if (onLEDControlRequested) {
            onLEDControlRequested();
        }
    });
    
    m_networkSettingsButton->setClickHandler([this]() {
        if (onNetworkSettingsRequested) {
            onNetworkSettingsRequested();
        }
    });
    
    m_homeButton->setClickHandler([this]() {
        if (onHomeRequested) {
            onHomeRequested();
        }
    });
    
    // Fragmentの追加
    addFragment(m_detectionButton, "detectionButton");
    addFragment(m_calibrationButton, "calibrationButton");
    addFragment(m_ledControlButton, "ledControlButton");
    addFragment(m_networkSettingsButton, "networkSettingsButton");
    addFragment(m_homeButton, "homeButton");
}

bool SettingsActivity::onStart() {
    if (!Activity::onStart()) {
        return false;
    }
    return true;
}

bool SettingsActivity::onResume() {
    if (!Activity::onResume()) {
        return false;
    }
    
    // 画面を描画
    draw();
    
    return true;
}

void SettingsActivity::onPause() {
    Activity::onPause();
}

void SettingsActivity::onStop() {
    Activity::onStop();
}

void SettingsActivity::onDestroy() {
    Activity::onDestroy();
}

bool SettingsActivity::handleEvent(const framework::Event& event) {
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
    }
    
    return false;
}

void SettingsActivity::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // タイトルを描画
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString("Settings Menu", 160, 20);
    
    // ボタンを描画
    m_detectionButton->draw();
    m_calibrationButton->draw();
    m_ledControlButton->draw();
    m_networkSettingsButton->draw();
    m_homeButton->draw();
}
