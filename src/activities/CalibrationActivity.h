#ifndef CALIBRATION_ACTIVITY_H
#define CALIBRATION_ACTIVITY_H

#include "../framework/Activity.h"
#include "../system/FaceDetector.h"
#include "../ui/UIManager.h"
#include "../fragments/ButtonFragment.h"

class CalibrationActivity : public framework::Activity {
public:
    CalibrationActivity();
    virtual ~CalibrationActivity();
    
    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;
    
    virtual bool handleEvent(const framework::Event& event) override;
    
    void initialize(FaceDetector* faceDetector, IMUSensor* imuSensor, UIManager* uiManager);
    void update();
    void draw();
    
    // ホーム画面遷移用のコールバック設定
    void setHomeTransitionCallback(std::function<void()> callback) {
        onHomeRequested = callback;
    }
    
private:
    FaceDetector* m_faceDetector;
    IMUSensor* m_imuSensor;
    UIManager* m_uiManager;
    
    ButtonFragment* m_resetButton;
    ButtonFragment* m_saveButton;
    ButtonFragment* m_loadButton;
    ButtonFragment* m_homeButton;
    
    enum CalibrationState {
        WAIT_STABLE,
        DETECT_FACE
    };
    
    CalibrationState m_calibrationState;
    
    // コールバック関数
    std::function<void()> onHomeRequested;
    
    // UIコンポーネント用のID定数
    enum {
        ID_NONE = 0,
        ID_BUTTON_RESET,
        ID_BUTTON_SAVE,
        ID_BUTTON_LOAD,
        ID_BUTTON_HOME
    };
};

#endif // CALIBRATION_ACTIVITY_H
