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

    // 描画と更新のメソッド
    void draw();
    void update();
    void startScreenSaver();
    void stopScreenSaver();
    bool isPlaying() { return m_isPlaying; }
    void setPlaying(bool playing) { m_isPlaying = playing; }

private:
    unsigned long m_startTime;
    unsigned long m_lastUpdateTime;
    float m_animationProgress;
    bool m_isPlaying;

    // パターン切り替え用のメンバ
    int m_currentPatternIndex;
    unsigned long m_lastPatternChangeTime;
    static const unsigned long PATTERN_CHANGE_INTERVAL = 10000; // 10秒ごとにパターン切替

    // 各パターン描画のヘルパーメソッド（全6種類）
    void drawPattern1();
    void drawPattern2();
    void drawPattern3();
    void drawPattern4();
    void drawPattern5();
    void drawPattern6();

    // ダイナミックな色生成（SplashActivity と同様の配色）
    uint16_t getDynamicColor(float offset);
};

#endif // SCREENSAVER_ACTIVITY_H
