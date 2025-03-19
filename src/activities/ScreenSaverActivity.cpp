#include "ScreenSaverActivity.h"
#include <M5Unified.h>
#include <math.h>

ScreenSaverActivity::ScreenSaverActivity()
    : Activity(0, "ScreenSaverActivity"),
      m_startTime(0),
      m_lastUpdateTime(0),
      m_animationProgress(0.0f),
      m_currentPatternIndex(0),
      m_lastPatternChangeTime(0)
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

void ScreenSaverActivity::draw() {
    // 背景を黒で塗りつぶす
    M5.Lcd.fillScreen(TFT_BLACK);
    // 現在のパターンに応じた描画を実行
    switch(m_currentPatternIndex) {
        case 0: drawPattern1(); break;
        case 1: drawPattern2(); break;
        case 2: drawPattern3(); break;
        case 3: drawPattern4(); break;
        case 4: drawPattern5(); break;
        case 5: drawPattern6(); break;
        default: drawPattern1(); break;
    }
}

void ScreenSaverActivity::update() {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - m_lastUpdateTime;
    // 約30fpsで更新（33ms毎）
    if (elapsed >= 33) {
        m_lastUpdateTime = currentTime;
        // 進行速度をゆっくりにするため、増加量を小さく（0.01f）しています
        m_animationProgress += 0.01f;
        if (m_animationProgress >= 1.0f) {
            m_animationProgress = 0.0f;
        }
        
        // パターン切替のタイミングチェック
        if (currentTime - m_lastPatternChangeTime >= PATTERN_CHANGE_INTERVAL) {
            m_currentPatternIndex = (m_currentPatternIndex + 1) % 6;
            m_lastPatternChangeTime = currentTime;
        }
        draw();
    }
}

// ダイナミックな色生成（SplashActivity と同様の配色）
uint16_t ScreenSaverActivity::getDynamicColor(float offset) {
    uint8_t r = 128 + 127 * sin((m_animationProgress + offset) * PI * 2);
    uint8_t g = 128 + 127 * sin((m_animationProgress + offset + 0.33f) * PI * 2);
    uint8_t b = 128 + 127 * sin((m_animationProgress + offset + 0.66f) * PI * 2);
    return M5.Lcd.color565(r, g, b);
}

// パターン1: SplashActivity のロゴ風（八角形＋中央テキスト）
void ScreenSaverActivity::drawPattern1() {
    int centerX = 160;
    int centerY = 120;
    
    int baseSize = 60;
    float scale = 1.0f + 0.3f * sin(m_animationProgress * PI * 2);
    int size = baseSize * scale;
    
    const int sides = 8;
    float angleStep = 2 * PI / sides;
    float rotation = m_animationProgress * 2 * PI;
    uint16_t dynamicColor = getDynamicColor(0.0f);
    
    for (int i = 0; i < sides; i++) {
        float x1 = centerX + size * cos(rotation + i * angleStep);
        float y1 = centerY + size * sin(rotation + i * angleStep);
        float x2 = centerX + size * cos(rotation + (i + 1) * angleStep);
        float y2 = centerY + size * sin(rotation + (i + 1) * angleStep);
        M5.Lcd.drawLine(x1, y1, x2, y2, dynamicColor);
    }
    
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(dynamicColor);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Screensaver", centerX, centerY);
}

// パターン2: 回転する星形パターン
void ScreenSaverActivity::drawPattern2() {
    int centerX = 160;
    int centerY = 120;
    int numPoints = 5;  // 星の頂点数（実際は10頂点の交互ポリゴン）
    float outerRadius = 60 + 10 * sin(m_animationProgress * PI * 2);
    float innerRadius = outerRadius * 0.5;
    int totalVertices = numPoints * 2;
    float angleOffset = m_animationProgress * 2 * PI; // 回転
    uint16_t color = getDynamicColor(0.2f);
    float verticesX[10], verticesY[10];
    
    for (int i = 0; i < totalVertices; i++) {
        float angle = angleOffset + i * PI / numPoints;
        float r = (i % 2 == 0) ? outerRadius : innerRadius;
        verticesX[i] = centerX + r * cos(angle);
        verticesY[i] = centerY + r * sin(angle);
    }
    for (int i = 0; i < totalVertices; i++) {
        int next = (i + 1) % totalVertices;
        M5.Lcd.drawLine(verticesX[i], verticesY[i], verticesX[next], verticesY[next], color);
    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Star", centerX, centerY);
}

// パターン3: 拡大・縮小する円のパターン
void ScreenSaverActivity::drawPattern3() {
    int centerX = 160;
    int centerY = 120;
    uint16_t color = getDynamicColor(0.3f);
    // 複数の円を描画
    for (int i = 1; i <= 3; i++) {
        int radius = 30 * i + 10 * sin(m_animationProgress * PI * 2 + i);
        M5.Lcd.drawCircle(centerX, centerY, radius, color);
    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Circles", centerX, centerY);
}

// パターン4: 動く矩形（バウンス）パターン
void ScreenSaverActivity::drawPattern4() {
    int rectWidth = 40, rectHeight = 30;
    int centerX = 160, centerY = 120;
    uint16_t color = getDynamicColor(0.4f);
    // m_animationProgress に基づいて矩形の中心位置を円軌道上に配置
    int x = centerX + 80 * cos(m_animationProgress * 2 * PI);
    int y = centerY + 60 * sin(m_animationProgress * 2 * PI);
    int rectX = x - rectWidth / 2;
    int rectY = y - rectHeight / 2;
    M5.Lcd.drawRect(rectX, rectY, rectWidth, rectHeight, color);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Rect", centerX, centerY);
}

// パターン5: 斜めに流れるラインパターン
void ScreenSaverActivity::drawPattern5() {
    uint16_t color = getDynamicColor(0.5f);
    // 画面全体に対角線状のラインを一定間隔で描画し、進行度によりオフセット
    for (int i = -50; i < 320; i += 40) {
        int offset = (int)(m_animationProgress * 40);
        M5.Lcd.drawLine(i + offset, 0, i + offset - 20, 240, color);
    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Diagonal", 160, 120);
}

// パターン6: カラフルなグリッドパターン
void ScreenSaverActivity::drawPattern6() {
    int gridSize = 40;
    uint16_t color;
    for (int x = 0; x < 320; x += gridSize) {
        for (int y = 0; y < 240; y += gridSize) {
            color = getDynamicColor((float)(x + y) / 320.0f);
            M5.Lcd.drawRect(x, y, gridSize, gridSize, color);
        }
    }
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(getDynamicColor(0.6f));
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString("Grid", 160, 120);
}

// スクリーンセーバー開始（シンプルなループ処理例）
void ScreenSaverActivity::startScreenSaver() {
    while (isPlaying()) {
        update();
    }
}

// スクリーンセーバー停止
void ScreenSaverActivity::stopScreenSaver() {
    setPlaying(false);
}
