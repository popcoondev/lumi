#ifndef DETECTION_ACTIVITY_H
#define DETECTION_ACTIVITY_H

#include "../framework/Activity.h"
#include "../system/FaceDetector.h"
#include "../led/LEDManager.h"
#include "../ui/UIManager.h"
#include "../ui/views/LumiView.h"
#include "../ui/components/ActionBar.h"
#include "../ui/components/TextView.h"
#include "../fragments/ButtonFragment.h"

class DetectionActivity : public framework::Activity {
public:
    DetectionActivity();
    virtual ~DetectionActivity();
    
    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;
    
    virtual bool handleEvent(const framework::Event& event) override;
    
    void initialize(LEDManager* ledManager, FaceDetector* faceDetector, 
                   IMUSensor* imuSensor, LumiView* lumiView, UIManager* uiManager);
    void update();
    void draw();
    
    // ホーム画面遷移用のコールバック設定
    void setHomeTransitionCallback(std::function<void()> callback) {
        onHomeRequested = callback;
    }
    
private:
    LEDManager* m_ledManager;
    FaceDetector* m_faceDetector;
    IMUSensor* m_imuSensor;
    LumiView* m_lumiView;
    UIManager* m_uiManager;
    
    ButtonFragment* m_homeButton;
    
    int m_lastDetectedFace;
    CRGB m_currentLedColor;
    
    // コールバック関数
    std::function<void()> onHomeRequested;
    
    // ヘルパーメソッド
    void updateLEDs(int detectedFace);
    void updateDisplay(int detectedFace, float x, float y, float z);
    int mapViewFaceToLedFace(int viewFaceId);
    uint16_t crgbToRGB565(CRGB color);
    
    // UIコンポーネント用のID定数
    enum {
        ID_NONE = 0,
        ID_BUTTON_HOME
    };
};

#endif // DETECTION_ACTIVITY_H
