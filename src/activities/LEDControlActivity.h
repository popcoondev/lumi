#ifndef LED_CONTROL_ACTIVITY_H
#define LED_CONTROL_ACTIVITY_H

#include "../framework/Activity.h"
#include "../led/LEDManager.h"
#include "../fragments/ButtonFragment.h"

class LEDControlActivity : public framework::Activity {
public:
    LEDControlActivity();
    virtual ~LEDControlActivity();
    
    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;
    
    virtual bool handleEvent(const framework::Event& event) override;
    
    void initialize(LEDManager* ledManager);
    void draw();
    
    // ホーム画面遷移用のコールバック設定
    void setHomeTransitionCallback(std::function<void()> callback) {
        onHomeRequested = callback;
    }
    
private:
    LEDManager* m_ledManager;
    
    ButtonFragment* m_playPauseButton;
    ButtonFragment* m_prevButton;
    ButtonFragment* m_nextButton;
    ButtonFragment* m_homeButton;
    ButtonFragment* m_modeButton; // パターンモード切替ボタン
    
    // FPS制御関連のボタン
    ButtonFragment* m_fpsToggleButton;
    ButtonFragment* m_fps30Button;
    ButtonFragment* m_fps60Button;
    ButtonFragment* m_fps120Button;
    
    bool m_isPlaying;
    bool m_isJsonPattern; // JSONパターンモードかどうか
    int m_currentJsonPatternIndex; // 現在のJSONパターンインデックス
    
    // FPS表示用のタイマー
    unsigned long m_lastFpsUpdateTime;
    
    // コールバック関数
    std::function<void()> onHomeRequested;
    
    // ヘルパーメソッド
    void updatePatternInfo();
    void updateFpsInfo();
    void togglePlayPause();
    void nextPattern();
    void prevPattern();
    void togglePatternMode(); // パターンモード（通常/JSON）の切り替え
    void toggleFpsControl(); // FPS制御の有効/無効を切り替え
    void setFps(uint16_t fps); // FPSを設定
    
    // UIコンポーネント用のID定数
    enum {
        ID_NONE = 0,
        ID_BUTTON_PLAY_PAUSE,
        ID_BUTTON_PREV,
        ID_BUTTON_NEXT,
        ID_BUTTON_HOME,
        ID_BUTTON_MODE,
        ID_BUTTON_FPS_TOGGLE,
        ID_BUTTON_FPS_30,
        ID_BUTTON_FPS_60,
        ID_BUTTON_FPS_120
    };
};

#endif // LED_CONTROL_ACTIVITY_H
