#ifndef SETTINGS_ACTIVITY_H
#define SETTINGS_ACTIVITY_H

#include "../framework/Activity.h"
#include "../fragments/ButtonFragment.h"

class SettingsActivity : public framework::Activity {
public:
    SettingsActivity();
    virtual ~SettingsActivity();
    
    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;
    
    virtual bool handleEvent(const framework::Event& event) override;
    
    void initialize();
    void draw();
    
    // 各機能への遷移コールバック設定
    void setDetectionTransitionCallback(std::function<void()> callback) {
        onDetectionRequested = callback;
    }
    
    void setCalibrationTransitionCallback(std::function<void()> callback) {
        onCalibrationRequested = callback;
    }
    
    void setLEDControlTransitionCallback(std::function<void()> callback) {
        onLEDControlRequested = callback;
    }
    
    void setHomeTransitionCallback(std::function<void()> callback) {
        onHomeRequested = callback;
    }
    
private:
    // UIコンポーネント
    ButtonFragment* m_detectionButton;
    ButtonFragment* m_calibrationButton;
    ButtonFragment* m_ledControlButton;
    ButtonFragment* m_homeButton;
    
    // コールバック関数
    std::function<void()> onDetectionRequested;
    std::function<void()> onCalibrationRequested;
    std::function<void()> onLEDControlRequested;
    std::function<void()> onHomeRequested;
    
    // UIコンポーネント用のID定数
    enum {
        ID_NONE = 0,
        ID_BUTTON_DETECTION,
        ID_BUTTON_CALIBRATION,
        ID_BUTTON_LED_CONTROL,
        ID_BUTTON_HOME
    };
};

#endif // SETTINGS_ACTIVITY_H
