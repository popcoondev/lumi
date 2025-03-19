#include "ScreenSaverActivity.h"
#include <M5Unified.h>
#include <math.h>
#include <stdlib.h>

// ===== コンストラクタ／ライフサイクル =====
ScreenSaverActivity::ScreenSaverActivity()
    : Activity(0, "ScreenSaverActivity"),
      m_startTime(0),
      m_lastUpdateTime(0),
      m_animationProgress(0.0f),
      m_playing(true),
      m_currentPatternIndex(0),
      m_prevPatternIndex(0),
      m_inTransition(false),
      m_lastPatternChangeTime(0),
      m_transitionStartTime(0)
{
}

ScreenSaverActivity::~ScreenSaverActivity() {
}

bool ScreenSaverActivity::onCreate() {
    if (!Activity::onCreate()) return false;
    return true;
}

bool ScreenSaverActivity::onStart() {
    if (!Activity::onStart()) return false;
    return true;
}

bool ScreenSaverActivity::onResume() {
    if (!Activity::onResume()) return false;
    m_startTime = millis();
    m_lastUpdateTime = m_startTime;
    m_animationProgress = 0.0f;
    m_lastPatternChangeTime = m_startTime;
    m_currentPatternIndex = 0;
    m_inTransition = false;
    // Pattern3用ランダムオフセットを初期化（0〜PIの範囲）
    for (int i = 0; i < 50; i++) {
        m_spiralRandomOffsets[i] = random(0, 1000) / 1000.0f * PI;
    }
    startScreenSaver();
    return true;
}

void ScreenSaverActivity::onPause() {
    Activity::onPause();
    stopScreenSaver();
}

void ScreenSaverActivity::onStop() {
    Activity::onStop();
}

void ScreenSaverActivity::onDestroy() {
    Activity::onDestroy();
}

// ===== 再生状態管理 =====
bool ScreenSaverActivity::isPlaying() const {
    return m_playing;
}

void ScreenSaverActivity::setPlaying(bool playing) {
    m_playing = playing;
}

// ===== 描画・更新 =====
void ScreenSaverActivity::draw() {
    // 画面全体を黒でクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    unsigned long currentTime = millis();
    if (m_inTransition) {
        // 移行中はαブレンド（シームレス切替）
        float t = (currentTime - m_transitionStartTime) / (float)TRANSITION_DURATION;
        if (t > 1.0f) t = 1.0f;
        // 前のパターンと新パターンをそれぞれαブレンドして描画
        switch(m_prevPatternIndex) {
            case 0: drawPattern1(1.0f - t); break;
            case 1: drawPattern2(1.0f - t); break;
            case 2: drawPattern3(1.0f - t); break;
            case 3: drawPattern4(1.0f - t); break;
            case 4: drawPattern5(1.0f - t); break;
        }
        switch(m_currentPatternIndex) {
            case 0: drawPattern1(t); break;
            case 1: drawPattern2(t); break;
            case 2: drawPattern3(t); break;
            case 3: drawPattern4(t); break;
            case 4: drawPattern5(t); break;
        }
    } else {
        // 通常は現在のパターンをα=1.0で描画
        switch(m_currentPatternIndex) {
            case 0: drawPattern1(1.0f); break;
            case 1: drawPattern2(1.0f); break;
            case 2: drawPattern3(1.0f); break;
            case 3: drawPattern4(1.0f); break;
            case 4: drawPattern5(1.0f); break;
        }
    }
}

void ScreenSaverActivity::update() {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - m_lastUpdateTime;
    // 約30fps（約33ms毎）の更新
    if (elapsed >= 33) {
        m_lastUpdateTime = currentTime;
        // アニメーション進行度更新（ゆっくり動くように0.01ずつ増加）
        m_animationProgress += 0.01f;
        if (m_animationProgress >= 1.0f) {
            m_animationProgress = 0.0f;
        }
        // パターン切替判定
        if (!m_inTransition && (currentTime - m_lastPatternChangeTime >= PATTERN_DURATION)) {
            m_inTransition = true;
            m_transitionStartTime = currentTime;
            m_prevPatternIndex = m_currentPatternIndex;
            m_currentPatternIndex = (m_currentPatternIndex + 1) % 5;
        }
        if (m_inTransition && (currentTime - m_transitionStartTime >= TRANSITION_DURATION)) {
            m_inTransition = false;
            m_lastPatternChangeTime = currentTime;
        }
        draw();
    }
}

// スクリーンセーバー開始：isPlaying が true の間、update() を呼び出す
void ScreenSaverActivity::startScreenSaver() {
    setPlaying(true);
    while (isPlaying()) {
        update();
    }
}

// 停止
void ScreenSaverActivity::stopScreenSaver() {
    setPlaying(false);
}

// ===== ヘルパー関数 =====
uint16_t ScreenSaverActivity::dimColor(uint16_t color, float alpha) {
    // 16bit color565：R:5bit, G:6bit, B:5bit
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;
    r = (uint8_t)(r * alpha);
    g = (uint8_t)(g * alpha);
    b = (uint8_t)(b * alpha);
    return (r << 11) | (g << 5) | b;
}

uint16_t ScreenSaverActivity::getDynamicColor(float offset) {
    uint8_t r = 128 + 127 * sin((m_animationProgress + offset) * PI * 2);
    uint8_t g = 128 + 127 * sin((m_animationProgress + offset + 0.33f) * PI * 2);
    uint8_t b = 128 + 127 * sin((m_animationProgress + offset + 0.66f) * PI * 2);
    return M5.Lcd.color565(r, g, b);
}

// ===== Pattern1: スプラッシュアニメーション風（八角形＋中央テキスト "Lumi"） =====
void ScreenSaverActivity::drawPattern1(float alpha) {
    int centerX = 160;
    int centerY = 120;
    int baseSize = 60;
    float scale = 1.0f + 0.2f * sin(m_animationProgress * PI);
    int size = baseSize * scale;
    uint16_t color = dimColor(getDynamicColor(0.0f), alpha);
    const int sides = 8;
    float angleStep = PI / sides;
    
    // 外側の八角形
    for (int i = 0; i < sides; i++) {
        float x1 = centerX + size * cos(angleStep * (2 * i - 1));
        float y1 = centerY + size * sin(angleStep * (2 * i - 1));
        float x2 = centerX + size * cos(angleStep * (2 * i + 1));
        float y2 = centerY + size * sin(angleStep * (2 * i + 1));
        M5.Lcd.drawLine(x1, y1, x2, y2, color);
    }
    // 内側の八角形（70%のサイズ）
    int innerSize = size * 0.7;
    for (int i = 0; i < sides; i++) {
        float x1 = centerX + innerSize * cos(angleStep * (2 * i - 1));
        float y1 = centerY + innerSize * sin(angleStep * (2 * i - 1));
        float x2 = centerX + innerSize * cos(angleStep * (2 * i + 1));
        float y2 = centerY + innerSize * sin(angleStep * (2 * i + 1));
        M5.Lcd.drawLine(x1, y1, x2, y2, color);
    }
    // 中央に「Lumi」テキスト
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Lumi", centerX, centerY);
}

// ===== Pattern2: 回転するオクタゴン（ワイヤーフレーム） =====
void ScreenSaverActivity::drawPattern2(float alpha) {
    int centerX = 160, centerY = 120;
    float radius = 50;
    uint16_t color = dimColor(getDynamicColor(0.1f), alpha);
    const int sides = 8;
    float angleOffset = m_animationProgress * 2 * PI;
    float vertices[8][2];
    for (int i = 0; i < sides; i++) {
         float angle = angleOffset + (2 * PI * i / sides);
         vertices[i][0] = centerX + radius * cos(angle);
         vertices[i][1] = centerY + radius * sin(angle);
    }
    for (int i = 0; i < sides; i++) {
         int next = (i + 1) % sides;
         M5.Lcd.drawLine(vertices[i][0], vertices[i][1],
                         vertices[next][0], vertices[next][1], color);
    }
}

// ===== Pattern3: 3D螺旋（各再生時にランダムオフセットを加える） =====
void ScreenSaverActivity::drawPattern3(float alpha) {
    int centerX = 160, centerY = 120;
    const int numPoints = 50;
    uint16_t color = dimColor(getDynamicColor(0.2f), alpha);
    int prevX = centerX, prevY = centerY;
    float perspective = 200.0f;
    for (int i = 0; i < numPoints; i++) {
         float randomOffset = m_spiralRandomOffsets[i];
         float theta = i * 0.3f + m_animationProgress * 2 * PI + randomOffset;
         float radius = i * 2.0f;
         float z = 100 + 20 * sin(m_animationProgress * 2 * PI + i + randomOffset);
         int x = centerX + (int)(radius * cos(theta) * (perspective / (z)));
         int y = centerY + (int)(radius * sin(theta) * (perspective / (z)));
         if (i > 0) {
              M5.Lcd.drawLine(prevX, prevY, x, y, color);
         }
         prevX = x;
         prevY = y;
    }
}

// ===== Pattern4: パースペクティブグリッド（ビート感付き） =====
void ScreenSaverActivity::drawPattern4(float alpha) {
    int centerX = 160, centerY = 120;
    uint16_t baseColor = getDynamicColor(0.3f);
    // パルス（ビート）効果：0〜1の範囲
    float beat = 0.5f + 0.5f * sin(m_animationProgress * 4 * PI);
    uint16_t color = dimColor(baseColor, alpha * beat);
    // 横方向グリッド（下部から中央に向かって収束）
    for (int i = 1; i <= 10; i++) {
        int y = centerY + i * (240 - centerY) / 10;
        M5.Lcd.drawLine(0, y, 320, y, color);
    }
    // 縦方向グリッド（左右から中央に向かって収束）
    for (int i = 0; i <= 10; i++) {
        int x = i * 320 / 10;
        M5.Lcd.drawLine(x, centerY, x, 240, color);
    }
}

// ===== Pattern5: 画面全体を使ったウェーブパターン（ウェーブ感あり） =====
void ScreenSaverActivity::drawPattern5(float alpha) {
    // 複数のサイン波を描画
    for (int wave = 0; wave < 3; wave++) {
         float phaseShift = wave * PI / 2;
         uint16_t color = dimColor(getDynamicColor(0.4f + wave * 0.05f), alpha);
         int prevX = 0;
         int prevY = 120;
         for (int x = 0; x <= 320; x += 4) {
              float waveAmplitude = 20 + 10 * sin(m_animationProgress * 2 * PI + wave);
              float frequency = 0.02f;
              int y = 120 + (int)(waveAmplitude * sin(x * frequency + m_animationProgress * 2 * PI + phaseShift));
              if (x > 0) {
                  M5.Lcd.drawLine(prevX, prevY, x, y, color);
              }
              prevX = x;
              prevY = y;
         }
    }
    // 背景のウェーブ状塗りつぶし
    for (int x = 0; x < 320; x += 2) {
         float waveAmplitude = 20 + 10 * sin(m_animationProgress * 2 * PI);
         int y = 120 + (int)(waveAmplitude * sin(x * 0.02f + m_animationProgress * 2 * PI));
         uint16_t fillColor = dimColor(getDynamicColor(0.4f), alpha * 0.5f);
         M5.Lcd.drawFastVLine(x, y, 240 - y, fillColor);
    }
}
