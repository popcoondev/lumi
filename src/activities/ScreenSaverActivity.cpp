#include "ScreenSaverActivity.h"
#include <M5Unified.h>
#include <math.h>
#include <stdlib.h>
#include "../framework/EventBus.h"
#include "../framework/ActivityManager.h"

// ===== コンストラクタ／ライフサイクル =====
ScreenSaverActivity::ScreenSaverActivity()
    : Activity(0, "ScreenSaverActivity"),
      m_activityManager(nullptr),
      m_startTime(0),
      m_lastUpdateTime(0),
      m_animationProgress(0.0f),
      m_playing(true),
      m_currentPatternIndex(0),
      m_prevPatternIndex(0),
      m_inTransition(false),
      m_lastPatternChangeTime(0),
      m_transitionStartTime(0),
      m_canvas(nullptr)
{
}

ScreenSaverActivity::~ScreenSaverActivity() {
    // キャンバスのクリーンアップ
    if (m_canvas != nullptr) {
        delete m_canvas;
        m_canvas = nullptr;
    }
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
    
    // M5Canvasの初期化
    if (m_canvas == nullptr) {
        m_canvas = new M5Canvas(&M5.Lcd);
        m_canvas->createSprite(320, 240);  // 画面サイズに合わせる
    }
    
    m_startTime = millis();
    m_lastUpdateTime = m_startTime;
    m_animationProgress = 0.0f;
    m_lastPatternChangeTime = m_startTime;
    m_currentPatternIndex = 0;
    m_inTransition = false;
    
    // Pattern3用：各ラインの初期値（画面上のランダムな開始位置、方向、初期長さ0）
    for (int i = 0; i < NUM_LINES; i++) {
        m_lineStartX[i] = random(0, 320);
        m_lineStartY[i] = random(0, 240);
        m_lineAngle[i] = random(0, 628) / 100.0f; // 0〜6.28（ラジアン）
        m_lineLength[i] = 0;
    }
    
    // イベントバスへの登録
    framework::EventBus::getInstance().subscribe(this);
    
    startScreenSaver();
    return true;
}

void ScreenSaverActivity::onPause() {
    Activity::onPause();
    stopScreenSaver();
    
    // キャンバスのクリーンアップ
    if (m_canvas != nullptr) {
        m_canvas->deleteSprite();
        delete m_canvas;
        m_canvas = nullptr;
    }
}

void ScreenSaverActivity::onStop() {
    Activity::onStop();
}

void ScreenSaverActivity::onDestroy() {
    // イベントバスからの登録解除
    framework::EventBus::getInstance().unsubscribe(this);
    
    Activity::onDestroy();
}

// イベント処理
bool ScreenSaverActivity::handleEvent(const framework::Event& event) {
    Serial.println("Event received");
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
    }
    
    // タッチイベントの処理
    if (event.getType() == framework::EventType::TOUCH) {
        Serial.println("Touch event received in ScreenSaver");
        const framework::TouchEvent& touchEvent = static_cast<const framework::TouchEvent&>(event);
        
        // タッチダウンイベントの場合
        if (touchEvent.getAction() == framework::TouchAction::DOWN) {
            Serial.println("Touch detected in ScreenSaver - returning to home");
            
            // スクリーンセーバーを停止
            stopScreenSaver();
            
            // LumiHomeActivityに戻る
            if (m_activityManager) {
                m_activityManager->startActivity("home");
            }
            
            return true;
        }
    }
    
    return false;
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
    if (m_canvas == nullptr) return;
    
    // キャンバスを黒でクリア
    m_canvas->fillScreen(TFT_BLACK);
    
    unsigned long currentTime = millis();
    if (m_inTransition) {
        // 移行中はαブレンドして前パターンと新パターンを描画
        float t = (currentTime - m_transitionStartTime) / (float)TRANSITION_DURATION;
        if (t > 1.0f) t = 1.0f;
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
        // 通常時は現在のパターンをα=1.0で描画
        switch(m_currentPatternIndex) {
            case 0: drawPattern1(1.0f); break;
            case 1: drawPattern2(1.0f); break;
            case 2: drawPattern3(1.0f); break;
            case 3: drawPattern4(1.0f); break;
            case 4: drawPattern5(1.0f); break;
        }
    }
    
    // キャンバスを画面に転送
    m_canvas->pushSprite(0, 0);
}

void ScreenSaverActivity::update() {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - m_lastUpdateTime;
    // 約30fps（約33ms毎）の更新
    if (elapsed >= 33) {
        m_lastUpdateTime = currentTime;
        m_animationProgress += 0.01f;
        if (m_animationProgress >= 1.0f) {
            m_animationProgress = 0.0f;
        }
        // Pattern切替判定
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
        // Pattern3用：ラインの成長更新（各ラインの長さを増加、一定長さでリセット）
        if (m_currentPatternIndex == 2 || m_prevPatternIndex == 2) {
            for (int i = 0; i < NUM_LINES; i++) {
                m_lineLength[i] += 1.0f; // 成長速度
                if (m_lineLength[i] > random(30, 150)) {
                    // 長さが一定以上になったら、新たな開始位置・方向にリセット
                    m_lineStartX[i] = random(0, 320);
                    m_lineStartY[i] = random(0, 240);
                    m_lineAngle[i] = random(0, 628) / 100.0f;
                    m_lineLength[i] = 0;
                }
            }
        }
        draw();
    }
}

// スクリーンセーバー開始
void ScreenSaverActivity::startScreenSaver() {
    Serial.println("ScreenSaverActivity::startScreenSaver - Starting screen saver");
    setPlaying(true);
    // ブロッキングループを削除
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
    return m_canvas->color565(r, g, b);
}

// ===== Pattern1: スプラッシュアニメーション風（八角形＋中央テキスト "Lumi"） =====
void ScreenSaverActivity::drawPattern1(float alpha) {
    if (m_canvas == nullptr) return;
    
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
        m_canvas->drawLine(x1, y1, x2, y2, color);
    }
    // 内側の八角形（70%のサイズ）
    int innerSize = size * 0.7;
    for (int i = 0; i < sides; i++) {
        float x1 = centerX + innerSize * cos(angleStep * (2 * i - 1));
        float y1 = centerY + innerSize * sin(angleStep * (2 * i - 1));
        float x2 = centerX + innerSize * cos(angleStep * (2 * i + 1));
        float y2 = centerY + innerSize * sin(angleStep * (2 * i + 1));
        m_canvas->drawLine(x1, y1, x2, y2, color);
    }
    // 中央テキスト
    m_canvas->setTextSize(2);
    m_canvas->setTextColor(color);
    m_canvas->setTextDatum(MC_DATUM);
    m_canvas->drawString("Lumi", centerX, centerY);
}

// ===== Pattern2: オクタゴン展開（中心から大小のオクタゴンが連続して現れる） =====
void ScreenSaverActivity::drawPattern2(float alpha) {
    if (m_canvas == nullptr) return;
    
    int centerX = 160, centerY = 120;
    const int sides = 8;
    // 6段階のオクタゴンを描画
    for (int i = 0; i < 6; i++) {
         // m_animationProgress に段階オフセットを加えて各オクタゴンのフェーズを計算
         float phase = m_animationProgress + i * 0.05f;
         if (phase > 1.0f) phase -= 1.0f;
         // 半径は20～120の範囲でフェーズにより変化
         float radius = 20 + phase * 100;
         uint16_t color = dimColor(getDynamicColor(0.1f + i * 0.05f), alpha);
         float angleOffset = phase * 2 * PI;
         float vertices[sides][2];
         for (int j = 0; j < sides; j++) {
              float angle = angleOffset + (2 * PI * j / sides);
              vertices[j][0] = centerX + radius * cos(angle);
              vertices[j][1] = centerY + radius * sin(angle);
         }
         // オクタゴンのアウトラインを2回描画して太く見せる
         for (int j = 0; j < sides; j++) {
              int next = (j + 1) % sides;
              m_canvas->drawLine(vertices[j][0], vertices[j][1],
                              vertices[next][0], vertices[next][1], color);
              m_canvas->drawLine(vertices[j][0] + 1, vertices[j][1] + 1,
                              vertices[next][0] + 1, vertices[next][1] + 1, color);
         }
    }
}

// ===== Pattern3: ランダムライン成長（画面上のランダムな地点から、ランダム方向へ伸びるライン） =====
void ScreenSaverActivity::drawPattern3(float alpha) {
    if (m_canvas == nullptr) return;
    
    uint16_t color = dimColor(getDynamicColor(0.2f), alpha);
    for (int i = 0; i < NUM_LINES; i++) {
         int x0 = m_lineStartX[i];
         int y0 = m_lineStartY[i];
         int x1 = x0 + (int)(m_lineLength[i] * cos(m_lineAngle[i]));
         int y1 = y0 + (int)(m_lineLength[i] * sin(m_lineAngle[i]));
         m_canvas->drawLine(x0, y0, x1, y1, color);
    }
}

// ===== Pattern4: パースペクティブグリッド（全画面利用・ビート感付き） =====
void ScreenSaverActivity::drawPattern4(float alpha) {
    if (m_canvas == nullptr) return;
    
    // ビート感：sin波によるパルス
    float beat = 0.5f + 0.5f * sin(m_animationProgress * 4 * PI);
    uint16_t color = dimColor(getDynamicColor(0.3f), alpha * beat);
    // 横方向グリッド：画面全体（上下均等に）
    for (int i = 1; i < 10; i++) {
         int y = i * 240 / 10;
         m_canvas->drawLine(0, y, 320, y, color);
    }
    // 縦方向グリッド：画面全体
    for (int i = 0; i <= 10; i++) {
         int x = i * 320 / 10;
         m_canvas->drawLine(x, 0, x, 240, color);
    }
}

// ===== Pattern5: 画面全体ウェーブパターン（ウェーブ感強調） =====
void ScreenSaverActivity::drawPattern5(float alpha) {
    if (m_canvas == nullptr) return;
    
    // 複数の波線を重ねて描画（画面全体をカバー）
    for (int wave = 0; wave < 4; wave++) {
         float phaseShift = wave * PI / 4;
         uint16_t color = dimColor(getDynamicColor(0.4f + wave * 0.07f), alpha);
         int prevX = 0;
         int prevY = (int)(120 + 60 * sin(m_animationProgress * 2 * PI + phaseShift));
         for (int x = 0; x <= 320; x += 2) {
              int y = (int)(120 + 60 * sin(x * 0.03f + m_animationProgress * 2 * PI + phaseShift)
                            + 30 * sin(m_animationProgress * PI + wave));
              m_canvas->drawLine(prevX, prevY, x, y, color);
              prevX = x;
              prevY = y;
         }
    }
}
