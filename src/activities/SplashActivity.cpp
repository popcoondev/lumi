#include "SplashActivity.h"
#include <M5Unified.h>

SplashActivity::SplashActivity()
    : Activity(0, "SplashActivity"),
      m_startTime(0),
      m_lastUpdateTime(0),
      m_animationProgress(0.0f)
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
