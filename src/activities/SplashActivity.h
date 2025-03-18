#ifndef SPLASH_ACTIVITY_H
#define SPLASH_ACTIVITY_H

#include "../framework/Activity.h"
#include "../network/NetworkManager.h"
#include "../network/WebServerManager.h"
#include "../led/LEDManager.h"

// 初期化ステップを定義
enum class InitStep {
    INIT_START,
    INIT_NETWORK,
    INIT_WEBSERVER,
    INIT_SERVICES,
    INIT_COMPLETE
};

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
    
    // 初期化に必要なマネージャーを設定
    void setManagers(NetworkManager* networkManager, WebServerManager* webServerManager, LEDManager* ledManager);
    
    // 初期化完了時のコールバック設定
    void setInitCompletedCallback(std::function<void()> callback) {
        m_initCompletedCallback = callback;
    }
    
private:
    unsigned long m_startTime;
    unsigned long m_lastUpdateTime;
    float m_animationProgress;
    
    // 初期化関連
    InitStep m_currentInitStep;
    bool m_initSuccess;
    String m_statusMessage;
    
    // 各種マネージャー
    NetworkManager* m_networkManager;
    WebServerManager* m_webServerManager;
    LEDManager* m_ledManager;
    
    // 初期化完了時のコールバック
    std::function<void()> m_initCompletedCallback;
    
    // アニメーション用のヘルパーメソッド
    void drawLogo(float scale, uint16_t color);
    
    // 初期化処理のヘルパーメソッド
    void processInitStep();
    void drawStatusMessage();
    bool initNetwork();
    bool initWebServer();
    bool initServices();
};

#endif // SPLASH_ACTIVITY_H
