#ifndef SPLASH_ACTIVITY_H
#define SPLASH_ACTIVITY_H

#include "../framework/Activity.h"

class SplashActivity : public framework::Activity {
public:
    SplashActivity();
    virtual ~SplashActivity();
    
    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;
    
    void draw();
    void update();
    
private:
    unsigned long m_startTime;
    unsigned long m_lastUpdateTime;
    float m_animationProgress;
    
    // アニメーション用のヘルパーメソッド
    void drawLogo(float scale, uint16_t color);
};

#endif // SPLASH_ACTIVITY_H
