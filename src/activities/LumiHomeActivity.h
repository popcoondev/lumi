#ifndef LUMI_HOME_ACTIVITY_H
#define LUMI_HOME_ACTIVITY_H

#include "../framework/Activity.h"
#include "../ui/views/OctagonRingView.h"
#include "../ui/components/Button.h"
#include "../ui/views/LumiView.h" // Sliderクラスの定義のため
#include "../led/LEDManager.h"
#include "../system/FaceDetector.h"
#include "../mic/MicManager.h"

class LumiHomeActivity : public framework::Activity {
public:
    // 操作モード（LumiViewと同じ定義）
    enum OperationMode {
        MODE_TAP,       // タップ操作モード（面の個別制御）
        MODE_PATTERN,   // パターン再生モード（LEDパターン選択・再生）
        MODE_LISTEN     // マイク入力モード（音量に応じてLEDをランダム点灯）
    };

    // コンストラクタ
    LumiHomeActivity();
    
    // デストラクタ
    virtual ~LumiHomeActivity();
    
    // ライフサイクルメソッド
    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;
    
    // イベント処理
    virtual bool handleEvent(const framework::Event& event) override;
    
    // 初期化と描画
    void initialize(LEDManager* ledManager, FaceDetector* faceDetector, MicManager* micManager);
    void draw();
    
    // タッチイベント処理
    void handleTouch();
    
    // 操作モード設定
    void setOperationMode(OperationMode mode);
    OperationMode getOperationMode() const { return m_currentMode; }
    
    // OctaControllerとの連携用メソッド
    void updateCenterButtonInfo();

private:
    // UIコンポーネント
    OctagonRingView m_octagon;
    Button m_settingsButton;
    Button m_resetButton;
    Button m_bottomLeftButton;
    Button m_bottomRightButton;
    Slider m_brightnessSlider;
    Slider m_valueBrightnessSlider;
    Slider m_hueSlider;
    Slider m_saturationSlider;
    
    // マネージャー
    LEDManager* m_ledManager;
    FaceDetector* m_faceDetector;
    MicManager* m_micManager;
    
    // タッチ検出用の領域
    struct {
        int centerX, centerY;
        int radius;
    } m_octagonCenter;
    
    // 状態管理
    OperationMode m_currentMode;
    int m_selectedPatternIndex;
    bool m_isPatternPlaying;
    CRGB m_currentLedColor;
    uint8_t m_currentHue;
    uint8_t m_currentSaturation;
    uint8_t m_currentValueBrightness;
    bool m_isTouchActive;
    int m_lastTouchX, m_lastTouchY;
    
    // ドラッグ選択用の変数
    bool m_isDragging;
    int m_dragStartFace;
    int m_lastDraggedFace;
    
    // タッチされたUI要素
    TouchedUI m_activeTouchedUI;
    
    // UIコンポーネント用のID定数（LumiViewと同じ定義）
    enum {
        ID_NONE = 0,
        ID_BUTTON_RESET,
        ID_BUTTON_SETTINGS,
        ID_BUTTON_BOTTOM_LEFT,
        ID_BUTTON_BOTTOM_RIGHT,
        ID_SLIDER_BRIGHTNESS,
        ID_SLIDER_VALUE_BRIGHTNESS,
        ID_SLIDER_HUE,
        ID_SLIDER_SATURATION,
        ID_OCTAGON_CENTER,
        ID_OCTAGON_FACE_BASE = 100  // 面IDは100+faceIndexとなる
    };
    
    // ヘルパーメソッド
    int getTappedFace(int x, int y);
    bool isCenterTapped(int x, int y);
    bool checkButtonTouch(Button& button, int touchX, int touchY, bool isPressed, bool wasPressed, bool wasReleased);
    bool checkSliderTouch(Slider& slider, int touchX, int touchY, bool isPressed, bool wasPressed, bool wasReleased);
    void drawCenterButtonInfo(const String& text, uint16_t color);
    int mapViewFaceToLedFace(int viewFaceId);
    int mapLedFaceToViewFace(int ledFaceId);
    void updatePatternSelection(int moveCount);
    
    // コールバック関数
    std::function<void(int)> onFaceTapped;
    std::function<void()> onCenterTapped;
    std::function<void()> onTopRightButtonTapped;
    std::function<void()> onTopLeftButtonTapped;
    std::function<void()> onBottomLeftButtonTapped;
    std::function<void()> onBottomRightButtonTapped;
    std::function<void(int)> onBrightnessChanged;
    std::function<void(int)> onHueChanged;
    std::function<void(int)> onSaturationChanged;
    std::function<void(int)> onValueBrightnessChanged;
};

#endif // LUMI_HOME_ACTIVITY_H
