#include "SplashActivity.h"
#include <M5Unified.h>

SplashActivity::SplashActivity()
    : Activity(0, "SplashActivity"),
      m_startTime(0),
      m_lastUpdateTime(0),
      m_animationProgress(0.0f),
      m_currentInitStep(InitStep::INIT_START),
      m_initSuccess(false),
      m_statusMessage("Initializing..."),
      m_networkManager(nullptr),
      m_webServerManager(nullptr),
      m_ledManager(nullptr),
      m_initCompletedCallback(nullptr)
{
}

SplashActivity::~SplashActivity() {
}

bool SplashActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    return true;
}

bool SplashActivity::onStart() {
    if (!Activity::onStart()) {
        return false;
    }
    return true;
}

bool SplashActivity::onResume() {
    if (!Activity::onResume()) {
        return false;
    }
    
    // アニメーション開始時間を記録
    m_startTime = millis();
    m_lastUpdateTime = m_startTime;
    m_animationProgress = 0.0f;
    
    // 初期化ステップをリセット
    m_currentInitStep = InitStep::INIT_START;
    m_initSuccess = false;
    m_statusMessage = "Initializing...";
    
    // 初期描画
    draw();
    
    return true;
}

void SplashActivity::onPause() {
    Activity::onPause();
}

void SplashActivity::onStop() {
    Activity::onStop();
}

void SplashActivity::onDestroy() {
    Activity::onDestroy();
}

void SplashActivity::setManagers(NetworkManager* networkManager, WebServerManager* webServerManager, LEDManager* ledManager) {
    m_networkManager = networkManager;
    m_webServerManager = webServerManager;
    m_ledManager = ledManager;
}

void SplashActivity::draw() {
    // 背景を黒で塗りつぶす
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // ロゴを描画（アニメーション効果付き）
    float scale = 1.0f + 0.2f * sin(m_animationProgress * PI);
    uint16_t color = M5.Lcd.color565(
        128 + 127 * sin(m_animationProgress * PI),
        128 + 127 * sin(m_animationProgress * PI + PI/3),
        128 + 127 * sin(m_animationProgress * PI + 2*PI/3)
    );
    
    drawLogo(scale, color);
    
    // 初期化ステータスメッセージを表示
    drawStatusMessage();
}

void SplashActivity::update() {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - m_lastUpdateTime;
    
    // 約30fpsでアニメーションを更新
    if (elapsed >= 33) {
        m_lastUpdateTime = currentTime;
        
        // アニメーション進行度を更新（0.0〜1.0の範囲で循環）
        m_animationProgress += 0.05f;
        if (m_animationProgress >= 1.0f) {
            m_animationProgress = 0.0f;
        }
        
        // 初期化ステップを処理
        processInitStep();
        
        // 再描画
        draw();
    }
}

void SplashActivity::drawLogo(float scale, uint16_t color) {
    // 画面中央座標
    int centerX = 160;
    int centerY = 120;
    
    // ロゴのサイズ
    int baseSize = 60;
    int size = baseSize * scale;
    
    // 簡易的なLumiロゴを描画（八角形）
    const int sides = 8;
    float angle = PI / sides;
    
    // 外側の八角形
    for (int i = 0; i < sides; i++) {
        float x1 = centerX + size * cos(angle * (2 * i - 1));
        float y1 = centerY + size * sin(angle * (2 * i - 1));
        float x2 = centerX + size * cos(angle * (2 * i + 1));
        float y2 = centerY + size * sin(angle * (2 * i + 1));
        
        M5.Lcd.drawLine(x1, y1, x2, y2, color);
    }
    
    // 内側の八角形
    int innerSize = size * 0.7f;
    for (int i = 0; i < sides; i++) {
        float x1 = centerX + innerSize * cos(angle * (2 * i - 1));
        float y1 = centerY + innerSize * sin(angle * (2 * i - 1));
        float x2 = centerX + innerSize * cos(angle * (2 * i + 1));
        float y2 = centerY + innerSize * sin(angle * (2 * i + 1));
        
        M5.Lcd.drawLine(x1, y1, x2, y2, color);
    }
    
    // 中央に「Lumi」テキストを表示
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Lumi", centerX, centerY);
}

void SplashActivity::drawStatusMessage() {
    // ステータスメッセージを画面下部に表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(m_statusMessage, 160, 220);
    
    // 初期化進捗バーを表示
    int progressBarWidth = 200;
    int progressBarHeight = 10;
    int progressBarX = 160 - progressBarWidth / 2;
    int progressBarY = 230;
    
    // 進捗バーの枠
    M5.Lcd.drawRect(progressBarX, progressBarY, progressBarWidth, progressBarHeight, TFT_WHITE);
    
    // 進捗状況に応じて塗りつぶし
    int progress = 0;
    switch (m_currentInitStep) {
        case InitStep::INIT_START:
            progress = 0;
            break;
        case InitStep::INIT_NETWORK:
            progress = 25;
            break;
        case InitStep::INIT_WEBSERVER:
            progress = 50;
            break;
        case InitStep::INIT_SERVICES:
            progress = 75;
            break;
        case InitStep::INIT_COMPLETE:
            progress = 100;
            break;
    }
    
    int fillWidth = (progressBarWidth - 2) * progress / 100;
    if (fillWidth > 0) {
        M5.Lcd.fillRect(progressBarX + 1, progressBarY + 1, fillWidth, progressBarHeight - 2, TFT_GREEN);
    }
}

void SplashActivity::processInitStep() {
    // 最低でも2秒間はスプラッシュ画面を表示
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - m_startTime;
    
    if (elapsedTime < 2000) {
        return;
    }
    
    // 初期化ステップに応じた処理
    switch (m_currentInitStep) {
        case InitStep::INIT_START:
            m_statusMessage = "Starting initialization...";
            m_currentInitStep = InitStep::INIT_NETWORK;
            break;
            
        case InitStep::INIT_NETWORK:
            m_statusMessage = "Connecting to WiFi...";
            if (initNetwork()) {
                m_currentInitStep = InitStep::INIT_WEBSERVER;
            }
            break;
            
        case InitStep::INIT_WEBSERVER:
            m_statusMessage = "Starting web server...";
            if (initWebServer()) {
                m_currentInitStep = InitStep::INIT_SERVICES;
            }
            break;
            
        case InitStep::INIT_SERVICES:
            m_statusMessage = "Starting services...";
            if (initServices()) {
                m_currentInitStep = InitStep::INIT_COMPLETE;
                m_statusMessage = "Initialization complete!";
                m_initSuccess = true;
            }
            break;
            
        case InitStep::INIT_COMPLETE:
            // 初期化完了後、コールバックを呼び出してLumiHomeActivityに遷移
            if (m_initCompletedCallback) {
                m_initCompletedCallback();
            }
            break;
    }
}

bool SplashActivity::initNetwork() {
    if (!m_networkManager) {
        Serial.println("NetworkManager not set");
        return false;
    }
    
    // ネットワーク初期化
    if (m_networkManager->begin()) {
        Serial.println("Network manager initialized successfully");
        return true;
    } else {
        Serial.println("Failed to initialize network manager");
        m_statusMessage = "WiFi connection failed!";
        return false;
    }
}

bool SplashActivity::initWebServer() {
    if (!m_webServerManager) {
        Serial.println("WebServerManager not set");
        return false;
    }
    
    // Webサーバー初期化
    if (m_webServerManager->begin()) {
        Serial.println("Web server manager initialized successfully");
        m_webServerManager->start();
        Serial.println("Web server started");
        
        // IPアドレスの表示
        if (m_networkManager) {
            Serial.print("Device IP address: ");
            Serial.println(m_networkManager->getLocalIP());
            m_statusMessage = "Web server started at " + m_networkManager->getLocalIP().toString();
        }
        return true;
    } else {
        Serial.println("Failed to initialize web server manager");
        m_statusMessage = "Web server initialization failed!";
        return false;
    }
}

bool SplashActivity::initServices() {
    // 各種サービスの初期化
    // ここでは単純に成功を返す
    return true;
}
