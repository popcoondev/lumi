#ifndef SCREENSAVER_ACTIVITY_H
#define SCREENSAVER_ACTIVITY_H

#include "../framework/Activity.h"
#include <functional>

class ScreenSaverActivity : public framework::Activity {
public:
    ScreenSaverActivity();
    virtual ~ScreenSaverActivity();

    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;

    // 描画・更新処理
    void draw();
    void update();

    // スクリーンセーバーの開始／停止（ブロッキング例）
    void startScreenSaver();
    void stopScreenSaver();

    // 再生状態管理
    bool isPlaying() const;
    void setPlaying(bool playing);

private:
    unsigned long m_startTime;
    unsigned long m_lastUpdateTime;
    // アニメーション進行用（各パターン共通パラメータ）
    float m_animationProgress;
    
    // 再生状態フラグ
    bool m_playing;

    // パターン切替管理
    int m_currentPatternIndex;
    int m_prevPatternIndex;
    bool m_inTransition;
    unsigned long m_lastPatternChangeTime;
    unsigned long m_transitionStartTime;
    static const unsigned long PATTERN_DURATION = 10000;   // 各パターン表示時間（10秒）
    static const unsigned long TRANSITION_DURATION = 1000;   // 切替移行時間（1秒）

    // 各パターン描画（α値付き：0.0〜1.0）
    void drawPattern1(float alpha);
    void drawPattern2(float alpha);
    void drawPattern3(float alpha);
    void drawPattern4(float alpha);
    void drawPattern5(float alpha);

    // ヘルパー：指定の色に対してαブレンド（明るさ調整）
    uint16_t dimColor(uint16_t color, float alpha);

    // 共通のダイナミック色生成（スプラッシュと同様）
    uint16_t getDynamicColor(float offset);

    // --- Pattern2: オクタゴン展開用（特別なメンバは不要） ---

    // --- Pattern3: ランダムライン成長用 ---
    static const int NUM_LINES = 50;
    float m_lineStartX[NUM_LINES];
    float m_lineStartY[NUM_LINES];
    float m_lineAngle[NUM_LINES];
    float m_lineLength[NUM_LINES];
};

#endif // SCREENSAVER_ACTIVITY_H
