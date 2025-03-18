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
    
    bool m_isPlaying;
    
    // コールバック関数
    std::function<void()> onHomeRequested;
    
    // ヘルパーメソッド
    void updatePatternInfo();
    void togglePlayPause();
    void nextPattern();
    void prevPattern();
    
    // UIコンポーネント用のID定数
    enum {
        ID_NONE = 0,
        ID_BUTTON_PLAY_PAUSE,
        ID_BUTTON_PREV,
        ID_BUTTON_NEXT,
        ID_BUTTON_HOME
    };
};

#endif // LED_CONTROL_ACTIVITY_H
