#include "LumiHomeActivity.h"
#include "../framework/EventBus.h"
#include "../core/Constants.h"

#define CORNER_BUTTON_WIDTH 70
#define CORNER_BUTTON_HEIGHT 30

// CRGB色をM5Stack LCD用のuint16_t色に変換する関数
uint16_t crgbToRGB565(CRGB color) {
    return M5.Lcd.color565(color.r, color.g, color.b);
}

LumiHomeActivity::LumiHomeActivity()
    : Activity(0, "LumiHomeActivity"),
      m_settingsButton(nullptr),
      m_resetButton(nullptr),
      m_bottomLeftButton(nullptr),
      m_bottomRightButton(nullptr),
      m_brightnessSlider(nullptr),
      m_valueBrightnessSlider(nullptr),
      m_hueSlider(nullptr),
      m_saturationSlider(nullptr),
      m_ledManager(nullptr),
      m_faceDetector(nullptr),
      m_micManager(nullptr),
      m_currentMode(MODE_TAP),
      m_selectedPatternIndex(0),
      m_isPatternPlaying(false),
      m_currentHue(0),
      m_currentSaturation(255),
      m_currentValueBrightness(255),
      m_isTouchActive(false),
      m_lastTouchX(0),
      m_lastTouchY(0),
      m_isDragging(false),
      m_dragStartFace(-1),
      m_lastDraggedFace(-1),
      m_octagonHandled(false) 
{
    m_currentLedColor = CHSV(m_currentHue, m_currentSaturation, m_currentValueBrightness);
    
    // オクタゴンの中心タッチ検出用の設定
    m_octagonCenter.centerX = 160;
    m_octagonCenter.centerY = 120;
    m_octagonCenter.radius = 30;
    
    // ButtonFragment インスタンスの作成
    m_settingsButton = new ButtonFragment(ID_BUTTON_SETTINGS);
    m_resetButton = new ButtonFragment(ID_BUTTON_RESET);
    m_bottomLeftButton = new ButtonFragment(ID_BUTTON_BOTTOM_LEFT);
    m_bottomRightButton = new ButtonFragment(ID_BUTTON_BOTTOM_RIGHT);
    
    // ButtonFragment の位置とサイズを設定
    m_settingsButton->setDisplayArea(320 - CORNER_BUTTON_WIDTH, 0, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT);
    m_resetButton->setDisplayArea(0, 0, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT);
    m_bottomLeftButton->setDisplayArea(0, 240 - CORNER_BUTTON_HEIGHT, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT);
    m_bottomRightButton->setDisplayArea(320 - CORNER_BUTTON_WIDTH, 240 - CORNER_BUTTON_HEIGHT, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT);
    
    // SliderFragment インスタンスの作成
    m_brightnessSlider = new SliderFragment(ID_SLIDER_BRIGHTNESS);
    m_valueBrightnessSlider = new SliderFragment(ID_SLIDER_VALUE_BRIGHTNESS);
    m_hueSlider = new SliderFragment(ID_SLIDER_HUE);
    m_saturationSlider = new SliderFragment(ID_SLIDER_SATURATION);

    // SliderFragment の位置とサイズを設定
    m_brightnessSlider->setDisplayArea(0, 40, 40, 80);
    m_valueBrightnessSlider->setDisplayArea(0, 120, 40, 80);
    m_hueSlider->setDisplayArea(320 - 40, 40, 40, 80);
    m_saturationSlider->setDisplayArea(320 - 40, 120, 40, 80);

    // ラベルの設定
    // m_settingsButton->setLabel("Settings");
    // m_resetButton->setLabel("Reset");
    // m_bottomLeftButton->setLabel("Reset");
    // m_bottomRightButton->setLabel("Pattern");
}

LumiHomeActivity::~LumiHomeActivity() {
    // リソースの解放
    delete m_settingsButton;
    delete m_resetButton;
    delete m_bottomLeftButton;
    delete m_bottomRightButton;
    delete m_brightnessSlider;
    delete m_valueBrightnessSlider;
    delete m_hueSlider;
    delete m_saturationSlider;
}

bool LumiHomeActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    
    // ButtonFragment の作成
    m_settingsButton->onCreate();
    m_resetButton->onCreate();
    m_bottomLeftButton->onCreate();
    m_bottomRightButton->onCreate();
    
    // SliderFragment の作成
    m_brightnessSlider->onCreate();
    m_valueBrightnessSlider->onCreate();
    m_hueSlider->onCreate();
    m_saturationSlider->onCreate();
    
    // イベントバスへの登録
    framework::EventBus::getInstance().subscribe(this);
    
    return true;
}

void LumiHomeActivity::initialize(LEDManager* ledManager, FaceDetector* faceDetector, MicManager* micManager) {
    m_ledManager = ledManager;
    m_faceDetector = faceDetector;
    m_micManager = micManager;
    
    // オクタゴンビューの設定
    m_octagon.setViewPosition(40, 0, 240, 240);
    m_octagon.setMirrored(true);
    m_octagon.rotate(PI); // πラジアン（180度）LEDの向きを合わせるため
    m_octagon.setBackgroundColor(BLACK);
    m_octagon.setFaceDetector(faceDetector);
    
    // ボタンのスタイル設定
    m_settingsButton->setLabel("Settings");
    m_settingsButton->setColor(BLACK, TFT_LIGHTGREY);
    m_settingsButton->setFontSize(1.4);
    m_settingsButton->setType(BUTTON_TYPE_TEXT);

    m_resetButton->setLabel("Reset");
    m_resetButton->setColor(BLACK, TFT_LIGHTGREY);
    m_resetButton->setFontSize(1.4);
    m_resetButton->setType(BUTTON_TYPE_TEXT);
    
    m_bottomLeftButton->setLabel("");
    m_bottomLeftButton->setColor(BLACK, TFT_LIGHTGREY);
    m_bottomLeftButton->setFontSize(1.4);
    m_bottomLeftButton->setType(BUTTON_TYPE_TEXT);
    
    m_bottomRightButton->setLabel("Pattern");
    m_bottomRightButton->setColor(BLACK, TFT_LIGHTGREY);
    m_bottomRightButton->setFontSize(1.4);
    m_bottomRightButton->setType(BUTTON_TYPE_TEXT);
    
    // スライダーのタイトル設定
    m_brightnessSlider->setTitle("B");
    m_valueBrightnessSlider->setTitle("V");
    m_hueSlider->setTitle("H");
    m_saturationSlider->setTitle("S");
    
    // クリックハンドラの設定
    m_settingsButton->setClickHandler([this]() {
        if (onTopRightButtonTapped) {
            onTopRightButtonTapped();
        }
    });
    
    m_resetButton->setClickHandler([this]() {
        if (onTopLeftButtonTapped) {
            onTopLeftButtonTapped();
        }
    });
    
    m_bottomLeftButton->setClickHandler([this]() {
        if (onBottomLeftButtonTapped) {
            onBottomLeftButtonTapped();
        }
    });
    
    m_bottomRightButton->setClickHandler([this]() {
        if (onBottomRightButtonTapped) {
            onBottomRightButtonTapped();
        }
    });
    
    // スライダー値変更ハンドラの設定
    m_brightnessSlider->setValueChangeHandler([this](int value) {
        if (onBrightnessChanged) {
            onBrightnessChanged(value);
        }
    });
    
    m_valueBrightnessSlider->setValueChangeHandler([this](int value) {
        if (onValueBrightnessChanged) {
            onValueBrightnessChanged(value);
        }
    });
    
    m_hueSlider->setValueChangeHandler([this](int value) {
        if (onHueChanged) {
            onHueChanged(value);
        }
    });
    
    m_saturationSlider->setValueChangeHandler([this](int value) {
        if (onSaturationChanged) {
            onSaturationChanged(value);
        }
    });

    // コールバック関数の設定
    onTopRightButtonTapped = [this]() {
        Serial.println("====== CHANGING STATE TO MENU ======");
        
        // 状態変更をリクエスト
        if (onRequestSettingsTransition) {
            onRequestSettingsTransition();
        } else {
            // コールバックが設定されていない場合のフォールバック
            // 画面をクリアするだけ（現在の実装）
            M5.Lcd.fillScreen(TFT_BLACK);
        }
        
        Serial.println("State changed requested");
    };
    
    // 左上ボタン（リセットボタン）
    onTopLeftButtonTapped = [this]() {
        Serial.println("====== RESET BUTTON PRESSED ======");
        
        // 実行中のLEDパターンを停止
        m_ledManager->stopPattern();
        
        // 全てのLEDを消灯
        m_ledManager->resetAllLeds();
        
        // FaceDetectorのLED状態も更新
        if (m_faceDetector->getCalibratedFacesCount() > 0) {
            FaceData* faceList = m_faceDetector->getFaceList();
            for (int i = 0; i < m_faceDetector->getCalibratedFacesCount(); i++) {
                faceList[i].ledState = 0;
            }
        }
        
        // OctagonRingViewの全てのハイライトを解除
        for (int i = 0; i < NUM_FACES; i++) {
            m_octagon.setFaceHighlighted(i, false);
            // 一時的な色も解除
            m_octagon.resetFaceTempColor(i);
        }
        // ハイライト色も初期化
        m_octagon.setHighlightColor(crgbToRGB565(m_currentLedColor));
        
        // フォーカスもすべて解除
        m_octagon.clearAllFocus();
        
        // スライダーを初期位置に戻す
        // 色相を初期値(0)に設定
        m_currentHue = 0;
        m_hueSlider->setValue(0);
        m_hueSlider->draw();
        
        // 彩度を初期値(255)に設定
        m_currentSaturation = 255;
        m_saturationSlider->setValue(100); // 100%
        m_saturationSlider->draw();
        
        // 明度を初期値(255)に設定
        m_currentValueBrightness = 255;
        m_valueBrightnessSlider->setValue(100); // 100%
        m_valueBrightnessSlider->draw();
        
        // 全体の明るさを初期値に設定
        m_ledManager->setBrightness(255);
        m_brightnessSlider->setValue(100); // 100%
        m_brightnessSlider->draw();
        
        // 現在の色を更新
        m_currentLedColor = CHSV(m_currentHue, m_currentSaturation, m_currentValueBrightness);
        
        // センターボタン情報を更新
        updateCenterButtonInfo();
        
        Serial.println("All LEDs reset, focus cleared, and sliders reset");
    };
    
    // 左下ボタン - 機能なし
    onBottomLeftButtonTapped = [this]() {
        // 何も処理を行わない
        Serial.println("====== BOTTOM LEFT BUTTON PRESSED - NO ACTION ======");
    };

    // 面タップでフォーカスをトグル
    onFaceTapped = [this](int faceId) {
        // デバッグ出力 - タップされた面ID
        Serial.println("Face tapped: " + String(faceId));
        
        // 無効な面IDの場合は何もしない
        if (faceId < 0 || faceId >= MAX_FACES) {
            return;
        }
    };
    
    // 右下ボタンでモード切替
    onBottomRightButtonTapped = [this]() {
        // モード切替
        if (m_currentMode == MODE_TAP) {
            // タップモード → パターンモード
            setOperationMode(MODE_PATTERN);
            
            // パターンモードの初期設定
            m_selectedPatternIndex = m_ledManager->getCurrentPatternIndex();
            m_isPatternPlaying = m_ledManager->isPatternRunning();
            
    } else if (m_currentMode == MODE_PATTERN) {
        // パターンモード → リッスンモード
        setOperationMode(MODE_LISTEN);
        
        // マイク入力の開始
        if (micCallback) {
            m_micManager->startTask(micCallback);
        }
        
        // センターボタン情報を更新
        updateCenterButtonInfo();
        } else {
            // リッスンモード → タップモード
            setOperationMode(MODE_TAP);
            
            // マイク停止
            m_micManager->stopTask();
            
            // 全てのLEDを消灯
            m_ledManager->resetAllLeds();
        }
    };
    
    // 中央タップでモードに応じた処理
    onCenterTapped = [this]() {
        if (m_currentMode == MODE_TAP) {
            // タップモードの場合
            int focusedFacesLedState = m_octagon.getFocusedFacesLedState();
            int focusedFacesCount = m_octagon.getFocusedFacesCount();
            
            // フォーカスがない場合は何もしない
            if (focusedFacesCount == 0) {
                return;
            }
            
            // フォーカスされた面のLED状態に基づいて処理
            bool turnOn;
            if (focusedFacesLedState == 0) {
                // すべて消灯の場合は点灯
                turnOn = true;
            } else if (focusedFacesLedState == 2) {
                // すべて点灯の場合は消灯
                turnOn = false;
            } else {
                // 一部点灯の場合は消灯に統一
                turnOn = false;
            }
            
            // フォーカスされた全ての面に対して処理
            for (int viewFaceId = 0; viewFaceId < NUM_FACES; viewFaceId++) {
                if (m_octagon.isFaceFocused(viewFaceId)) {
                    // OctagonRingViewの面IDをLEDの面IDに変換
                    int ledFaceId = mapViewFaceToLedFace(viewFaceId);
                    
                    if (turnOn) {
                        // 点灯処理
                        m_ledManager->lightFace(ledFaceId, m_currentLedColor);
                        
                        // FaceDetectorのLED状態も更新
                        if (m_faceDetector->getCalibratedFacesCount() > 0) {
                            FaceData* faceList = m_faceDetector->getFaceList();
                            if (ledFaceId < m_faceDetector->getCalibratedFacesCount()) {
                                faceList[ledFaceId].ledState = 1;
                                faceList[ledFaceId].ledColor = m_currentLedColor;
                            }
                        }
                        
                        // OctagonRingViewのハイライトも設定
                        m_octagon.setFaceHighlighted(viewFaceId, true);
                    } else {
                        // 消灯処理
                        m_ledManager->lightFace(ledFaceId, CRGB::Black);
                        
                        // FaceDetectorのLED状態も更新
                        if (m_faceDetector->getCalibratedFacesCount() > 0) {
                            FaceData* faceList = m_faceDetector->getFaceList();
                            if (ledFaceId < m_faceDetector->getCalibratedFacesCount()) {
                                faceList[ledFaceId].ledState = 0;
                            }
                        }
                        
                        // OctagonRingViewのハイライトも解除
                        m_octagon.setFaceHighlighted(viewFaceId, false);
                    }
                }
            }
            
            // ハイライト色を設定
            m_octagon.setHighlightColor(crgbToRGB565(m_currentLedColor));
        } else if (m_currentMode == MODE_PATTERN) {
            // パターンモードの場合
            if (m_isPatternPlaying) {
                // パターン停止
                m_ledManager->stopPattern();
                m_isPatternPlaying = false;
            } else {
                // パターン再生
                m_ledManager->runPattern(m_selectedPatternIndex);
                m_isPatternPlaying = true;
                
            }
        } else if (m_currentMode == MODE_LISTEN) {
            // リッスンモードの場合は何もしない
            // 音量に応じて自動的にLEDが点灯するため
            // センターボタン情報は updateCenterButtonInfo() で更新されるので
            // ここでは何もしない
        }
        
        // センターボタン情報を更新
        updateCenterButtonInfo();
    };
    
    // 明るさスライダーでLED輝度を制御
    onBrightnessChanged = [this](int value) {
        uint8_t brightness = map(value, 0, 100, 0, 255);
        m_ledManager->setBrightness(brightness);
        
        // FaceDataの輝度も更新
        if (m_faceDetector->getCalibratedFacesCount() > 0) {
            FaceData* faceList = m_faceDetector->getFaceList();
            for (int i = 0; i < m_faceDetector->getCalibratedFacesCount(); i++) {
                if (faceList[i].ledState == 1) {
                    faceList[i].ledBrightness = brightness;
                }
            }
        }
        
        m_brightnessSlider->draw();
    };

    // 明度スライダーでLED明度を制御
    onValueBrightnessChanged = [this](int value) {
        m_currentValueBrightness = map(value, 0, 100, 0, 255);
        m_currentLedColor = CHSV(m_currentHue, m_currentSaturation, m_currentValueBrightness);
        
        // フォーカスされた面のみ色を更新
        for (int viewFace = 0; viewFace < NUM_FACES; viewFace++) {
            if (m_octagon.isFaceFocused(viewFace) && m_octagon.isFaceHighlighted(viewFace)) {
                // OctagonRingViewの面IDをLEDの面IDに変換
                int ledFace = mapViewFaceToLedFace(viewFace);
                m_ledManager->lightFace(ledFace, m_currentLedColor);
                
                // FaceDataの色も更新
                if (m_faceDetector->getCalibratedFacesCount() > 0) {
                    FaceData* faceList = m_faceDetector->getFaceList();
                    if (ledFace < m_faceDetector->getCalibratedFacesCount()) {
                        faceList[ledFace].ledColor = m_currentLedColor;
                    }
                }
            }
        }
        
        m_valueBrightnessSlider->draw();
    };

    // カラースライダーでLED色相を制御
    onHueChanged = [this](int value) {
        // 色相を0-255にマップ
        m_currentHue = map(value, 0, 100, 0, 255);
        m_currentLedColor = CHSV(m_currentHue, m_currentSaturation, m_currentValueBrightness);
        
        // フォーカスされた面のみ色を更新
        for (int viewFace = 0; viewFace < NUM_FACES; viewFace++) {
            if (m_octagon.isFaceFocused(viewFace) && m_octagon.isFaceHighlighted(viewFace)) {
                // OctagonRingViewの面IDをLEDの面IDに変換
                int ledFace = mapViewFaceToLedFace(viewFace);
                m_ledManager->lightFace(ledFace, m_currentLedColor);
                
                // FaceDataの色も更新
                if (m_faceDetector->getCalibratedFacesCount() > 0) {
                    FaceData* faceList = m_faceDetector->getFaceList();
                    if (ledFace < m_faceDetector->getCalibratedFacesCount()) {
                        faceList[ledFace].ledColor = m_currentLedColor;
                    }
                }
            }
        }
        
        // OctagonRingViewのハイライト色も更新
        m_octagon.setHighlightColor(crgbToRGB565(m_currentLedColor));
        
        m_hueSlider->draw();
    };

    // 彩度スライダーでLED彩度を制御
    onSaturationChanged = [this](int value) {
        // 彩度を0-255にマップ
        m_currentSaturation = map(value, 0, 100, 0, 255);
        m_currentLedColor = CHSV(m_currentHue, m_currentSaturation, m_currentValueBrightness);
        
        // フォーカスされた面のみ色を更新
        for (int viewFace = 0; viewFace < NUM_FACES; viewFace++) {
            if (m_octagon.isFaceFocused(viewFace) && m_octagon.isFaceHighlighted(viewFace)) {
                // OctagonRingViewの面IDをLEDの面IDに変換
                int ledFace = mapViewFaceToLedFace(viewFace);
                m_ledManager->lightFace(ledFace, m_currentLedColor);
                
                // FaceDataの色も更新
                if (m_faceDetector->getCalibratedFacesCount() > 0) {
                    FaceData* faceList = m_faceDetector->getFaceList();
                    if (ledFace < m_faceDetector->getCalibratedFacesCount()) {
                        faceList[ledFace].ledColor = m_currentLedColor;
                    }
                }
            }
        }
        
        // OctagonRingViewのハイライト色も更新
        m_octagon.setHighlightColor(crgbToRGB565(m_currentLedColor));
        
        m_saturationSlider->draw();
    };

    addFragment(m_settingsButton, "settingsButton");
    addFragment(m_resetButton, "resetButton");
    addFragment(m_bottomLeftButton, "bottomLeftButton");
    addFragment(m_bottomRightButton, "bottomRightButton");
    addFragment(m_brightnessSlider, "brightnessSlider");
    addFragment(m_valueBrightnessSlider, "valueBrightnessSlider");
    addFragment(m_hueSlider, "hueSlider");
    addFragment(m_saturationSlider, "saturationSlider");
}

bool LumiHomeActivity::onStart() {
    if (!Activity::onStart()) {
        return false;
    }
    return true;
}

bool LumiHomeActivity::onResume() {
    if (!Activity::onResume()) {
        return false;
    }

    draw();
    
    return true;
}

void LumiHomeActivity::onPause() {
    Activity::onPause();
}

void LumiHomeActivity::onStop() {
    Activity::onStop();
}

void LumiHomeActivity::onDestroy() {
    // イベントバスからの登録解除
    framework::EventBus::getInstance().unsubscribe(this);
    // framework::EventBus::getInstance().unsubscribe(m_settingsButton.get());
    // framework::EventBus::getInstance().unsubscribe(m_resetButton.get());
    // framework::EventBus::getInstance().unsubscribe(m_bottomLeftButton.get());
    // framework::EventBus::getInstance().unsubscribe(m_bottomRightButton.get());
    
    // ButtonFragment の破棄
    m_settingsButton->onDestroy();
    m_resetButton->onDestroy();
    m_bottomLeftButton->onDestroy();
    m_bottomRightButton->onDestroy();
    
    Activity::onDestroy();
}

bool LumiHomeActivity::handleEvent(const framework::Event& event) {
    Serial.println("Event received");
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
    }
    
    // イベントタイプに応じた処理
    switch (event.getType()) {
        case framework::EventType::TOUCH: {
            const framework::TouchEvent& touchEvent = static_cast<const framework::TouchEvent&>(event);
            // タッチイベント処理
            handleTouch();
            return true;
        }
        
        case framework::EventType::BUTTON: {
            const framework::ButtonEvent& buttonEvent = static_cast<const framework::ButtonEvent&>(event);
            // ボタンイベント処理
            uint32_t buttonId = buttonEvent.getButtonId();
            
            if (buttonEvent.getAction() == framework::ButtonAction::CLICK) {
                // ボタンIDに応じた処理
                if (buttonId == ID_BUTTON_SETTINGS && onTopRightButtonTapped) {
                    onTopRightButtonTapped();
                    return true;
                }
                else if (buttonId == ID_BUTTON_RESET && onTopLeftButtonTapped) {
                    onTopLeftButtonTapped();
                    return true;
                }
                else if (buttonId == ID_BUTTON_BOTTOM_LEFT && onBottomLeftButtonTapped) {
                    onBottomLeftButtonTapped();
                    return true;
                }
                else if (buttonId == ID_BUTTON_BOTTOM_RIGHT && onBottomRightButtonTapped) {
                    onBottomRightButtonTapped();
                    return true;
                }
            }
            return false;
        }
        
        case framework::EventType::SLIDER: {
            const framework::SliderEvent& sliderEvent = static_cast<const framework::SliderEvent&>(event);
            // スライダーイベント処理
            uint32_t sliderId = sliderEvent.getSliderId();
            float value = sliderEvent.getValue();
            
            // スライダーIDに応じて対応するコールバックを呼び出す
            if (sliderId == ID_SLIDER_BRIGHTNESS && onBrightnessChanged) {
                onBrightnessChanged(static_cast<int>(value));
                return true;
            }
            else if (sliderId == ID_SLIDER_HUE && onHueChanged) {
                onHueChanged(static_cast<int>(value));
                return true;
            }
            else if (sliderId == ID_SLIDER_SATURATION && onSaturationChanged) {
                onSaturationChanged(static_cast<int>(value));
                return true;
            }
            else if (sliderId == ID_SLIDER_VALUE_BRIGHTNESS && onValueBrightnessChanged) {
                onValueBrightnessChanged(static_cast<int>(value));
                return true;
            }
            
            return false;
        }
        
        default:
            return false;
    }
}

void LumiHomeActivity::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // オクタゴンを描画
    m_octagon.draw();
    
    // 四隅のボタンを描画
    m_settingsButton->draw();
    m_resetButton->draw();
    m_bottomLeftButton->draw();
    m_bottomRightButton->draw();
    
    // スライダーを描画
    m_brightnessSlider->draw();
    m_valueBrightnessSlider->draw();
    m_hueSlider->draw();
    m_saturationSlider->draw();
    
    // センターボタン情報を更新
    updateCenterButtonInfo();
}

void LumiHomeActivity::handleTouch() {
    // M5.Touch更新はOctaControllerで行われていることを前提
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();

    // タッチ座標取得
    int touchX = touch.x;
    int touchY = touch.y;
    
    // タッチ状態の更新
    m_isTouchActive = isPressed;
    
    // タッチ座標の記録
    static int touchStartX = 0;
    static int touchStartY = 0;
    
    if (isPressed) {
        m_lastTouchX = touchX;
        m_lastTouchY = touchY;
    }
    
    // タッチ開始位置を記録
    if (wasPressed) {
        touchStartX = touchX;
        touchStartY = touchY;
    }
    
    // 処理フラグをリセット
    m_octagonHandled = false;
    
    // オクタゴンの中心タップ判定
    if (wasPressed && m_octagon.getCenterButton().containsPoint(touchX, touchY)) {
        m_activeTouchedUI = TouchedUI(ID_OCTAGON_CENTER);
        m_octagon.getCenterButton().setPressed(true);
        m_octagonHandled = true;  // タッチを処理したとマーク
    }
    
    // モードに応じた処理
    if (m_currentMode == MODE_TAP) {
        // タップモードの場合
        
        // オクタゴンの面タップ判定
        if (wasPressed) {
            int faceId = getTappedFace(touchX, touchY);
            if (faceId >= 0) {
                m_activeTouchedUI = TouchedUI(ID_OCTAGON_FACE_BASE + faceId, faceId);
                // タップ視覚フィードバック - 一時的に色を変更
                uint16_t originalColor = m_octagon.getFaceColor(faceId);
                m_octagon.setFaceTempColor(faceId, TFT_DARKGREY);
                m_octagon.drawFace(faceId);  // 指定した面だけを再描画
                
                // ドラッグ選択の開始
                m_isDragging = true;
                m_dragStartFace = faceId;
                m_lastDraggedFace = faceId;
                
                // フォーカス状態をトグル
                m_octagon.setFaceFocused(faceId, !m_octagon.isFaceFocused(faceId));
                m_octagon.drawFace(faceId);  // フォーカス状態を反映して再描画
                
                // センターボタン情報を更新
                updateCenterButtonInfo();
                
                m_octagonHandled = true;  // タッチを処理したとマーク
            }
        }

        // タッチ継続中の処理
        if (isPressed && !wasPressed && !wasReleased) {
            // オクタゴン中心のドラッグ離脱検出
            if (m_activeTouchedUI.id == ID_OCTAGON_CENTER && !isCenterTapped(touchX, touchY)) {
                // センター領域から外れた - 視覚フィードバックを元に戻す
                // 中心座標を計算
                int centerX = 160; // オクタゴンの中心X座標
                int centerY = 120; // オクタゴンの中心Y座標
                int radius = 30;   // 中心円の半径
                
                M5.Lcd.fillCircle(centerX, centerY, radius * 0.5f, BLACK);
                m_octagon.drawCenter();  // センター部分のみ再描画
                
                m_octagonHandled = true;  // タッチを処理したとマーク
            }
            
            // 面のドラッグ選択処理
            if (m_isDragging) {
                int currentFaceId = getTappedFace(touchX, touchY);
                if (m_currentMode == MODE_TAP) {
                    if (currentFaceId >= 0 && currentFaceId != m_lastDraggedFace) {
                        // 新しい面にドラッグされた
                        m_lastDraggedFace = currentFaceId;
                        
                        // 新しい面のフォーカス状態を設定（最初にタップした面と同じ状態に）
                        bool shouldFocus = m_octagon.isFaceFocused(m_dragStartFace);
                        m_octagon.setFaceFocused(currentFaceId, shouldFocus);
                        m_octagon.drawFace(currentFaceId);  // 再描画
                        
                        // センターボタン情報を更新
                        updateCenterButtonInfo();
                        
                        m_octagonHandled = true;  // タッチを処理したとマーク
                    }
                }
            }
        }
    }
    else if (m_currentMode == MODE_PATTERN) {
        // パターンモードの場合
        if (wasPressed) {
            int faceId = getTappedFace(touchX, touchY);
            if (faceId == 3) {
                updatePatternSelection(-1);
                m_octagonHandled = true;  // タッチを処理したとマーク
            } else if (faceId == 7) {
                updatePatternSelection(1);
                m_octagonHandled = true;  // タッチを処理したとマーク
            }
        }
    }
    
    // タッチ終了時の処理
    if (wasReleased) {
        // タップ判定のための距離計算
        int dx = touchX - touchStartX;
        int dy = touchY - touchStartY;
        int distSquared = dx * dx + dy * dy;
        const int TAP_THRESHOLD_SQUARED = 100; // タップと判定する最大距離の二乗（10ピクセル）
        bool isTap = (distSquared <= TAP_THRESHOLD_SQUARED);
        
        if (m_activeTouchedUI.id == ID_OCTAGON_CENTER) {
            // ボタンの押下状態を解除
            m_octagon.getCenterButton().setPressed(false);
            
            // タップと判定された場合のみアクション実行
            if (m_octagon.getCenterButton().containsPoint(touchStartX, touchStartY) && isTap) {
                // パターンモードの場合、パターンの再生/停止を実行
                if (m_currentMode == MODE_PATTERN) {
                    m_isPatternPlaying = !m_isPatternPlaying;
                    if (m_isPatternPlaying) {
                        m_ledManager->runPattern(m_selectedPatternIndex);
                    } else {
                        m_ledManager->stopPattern();
                    }
                } else {
                    // 他のモードの場合は通常のコールバックを呼び出す
                    if (onCenterTapped) {
                        onCenterTapped();
                    }
                }
            }
            
            // センターボタン情報を更新
            updateCenterButtonInfo();
            
            m_octagonHandled = true;  // タッチを処理したとマーク
        }
        else if (m_activeTouchedUI.id >= ID_OCTAGON_FACE_BASE) {
            int faceId = m_activeTouchedUI.data;
            
            // 一時的な色を元に戻す
            m_octagon.resetFaceTempColor(faceId);
            m_octagon.drawFace(faceId);
            
            // ドラッグ選択の終了
            m_isDragging = false;
            m_dragStartFace = -1;
            m_lastDraggedFace = -1;
            
            // タップ判定
            if (isTap) {
                // MODE_PATTERNでのFace3/Face7のタップ処理
                if (m_currentMode == MODE_PATTERN) {
                    if (faceId == 3) {
                        updatePatternSelection(-1);
                        m_octagonHandled = true;
                    } else if (faceId == 7) {
                        updatePatternSelection(1);
                        m_octagonHandled = true;
                    }
                }
                
                // 同じ面上でリリースされた場合のみアクション実行
                int releasedFaceId = getTappedFace(touchX, touchY);
                if (releasedFaceId == faceId && onFaceTapped) {
                    onFaceTapped(faceId);
                }
            }
            
            m_octagonHandled = true;  // タッチを処理したとマーク
        }
        
        // タッチ情報をリセット
        m_activeTouchedUI = TouchedUI();
    }
}

// センターボタン情報の描画
void LumiHomeActivity::drawCenterButtonInfo(const String& text, uint16_t color) {
    int centerX = 160; // オクタゴンの中心X座標
    int centerY = 120; // オクタゴンの中心Y座標

    // 中央円を背景色で描画 - 完全に消去するために少し大きめに描画
    float innerRadius = 30; // 中心円の半径
    M5.Lcd.fillCircle(centerX, centerY, innerRadius + 2, BLACK);
    
    // テキスト表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(color, BLACK); // 背景色も指定して確実に消去
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString(text, centerX, centerY);
}

// オクタゴンの中心がタップされたか判定
bool LumiHomeActivity::isCenterTapped(int x, int y) {
    int centerX = m_octagonCenter.centerX;
    int centerY = m_octagonCenter.centerY;
    float innerRadius = m_octagonCenter.radius;
    
    // 中心からの距離を計算
    float dx = x - centerX;
    float dy = y - centerY;
    float distSq = dx * dx + dy * dy;
    
    // 中心円内かどうかを判定
    return distSq < innerRadius * innerRadius;
}

// オクタゴンの面がタップされたか判定
int LumiHomeActivity::getTappedFace(int x, int y) {
    return m_octagon.getFaceAtPoint(x, y);
}

// パターン選択の更新
void LumiHomeActivity::updatePatternSelection(int moveCount) {    
    // パターン数（LEDManagerから取得する想定）
    const int patternCount = m_ledManager->getPatternCount();

    // 選択中のパターンを更新
    m_selectedPatternIndex = (m_selectedPatternIndex + moveCount + patternCount) % patternCount;
    
    // センターボタン情報を更新
    updateCenterButtonInfo();
}

// センターボタン情報表示の更新
void LumiHomeActivity::updateCenterButtonInfo() {
    String text;
    uint16_t color;
    
    if (m_currentMode == MODE_TAP) {
        // タップモードの場合
        int ledState = m_octagon.getFocusedFacesLedState();
        int focusCount = m_octagon.getFocusedFacesCount();
        
        switch (ledState) {
            case -1: // フォーカスなし
                text = "";
                color = WHITE;
                break;
            case 0: // すべて消灯
                text = "ON " + String(focusCount);
                color = TFT_GREEN;
                break;
            case 2: // すべて点灯
                text = "OFF " + String(focusCount);
                color = TFT_RED;
                break;
            case 1: // 一部点灯
                // フォーカスされた面の中で点灯している面の数を取得
                int onCount = 0;
                for (int i = 0; i < NUM_FACES; i++) {
                    if (m_octagon.isFaceFocused(i) && m_octagon.isFaceHighlighted(i)) {
                        onCount++;
                    }
                }
                text = "TOGGLE " + String(onCount) + "/" + String(focusCount);
                color = TFT_YELLOW;
                break;
        }
    } else if (m_currentMode == MODE_PATTERN) {
        // パターンモードの場合
        String patternName = "Pattern " + String(m_selectedPatternIndex + 1);
        if (m_isPatternPlaying) {
            text = "STOP " + patternName;
            color = TFT_RED;
        } else {
            text = "PLAY " + patternName;
            color = TFT_GREEN;
        }
    } else if (m_currentMode == MODE_LISTEN) {
        // リッスンモードの場合
        text = "MIC";
        color = TFT_CYAN;
    }
    
    // CenterButtonクラスのsetInfo()メソッドを使用
    m_octagon.setCenterInfo(text, color);
}

// 操作モード設定
void LumiHomeActivity::setOperationMode(OperationMode mode) {
    m_currentMode = mode;
    
    // モードに応じたボタンラベルの設定
    if (mode == MODE_TAP) {
        m_bottomRightButton->setLabel("Pattern");
    } else if (mode == MODE_PATTERN) {
        m_bottomRightButton->setLabel("Listen");
    } else if (mode == MODE_LISTEN) {
        m_bottomRightButton->setLabel("Tap");
    }
    
    // フォーカスをクリア
    m_octagon.clearAllFocus();
    
    // 画面全体を再描画
    draw();
}

// OctagonRingViewの面IDをLEDの面IDに変換
int LumiHomeActivity::mapViewFaceToLedFace(int viewFaceId) {
    return viewFaceId; // 範囲外の場合はそのまま返す
}

// LEDの面IDをOctagonRingViewの面IDに変換
int LumiHomeActivity::mapLedFaceToViewFace(int ledFaceId) {
    // LEDManagerの面ID (0,1,2,3,4,5,6,7) を
    // OctagonRingViewの面ID (0,7,6,5,4,3,2,1) に変換
    static const int ledToViewMap[MAX_FACES] = {
        0, // 0 → 0
        7, // 1 → 7
        6, // 2 → 6
        5, // 3 → 5
        4, // 4 → 4
        3, // 5 → 3
        2, // 6 → 2
        1  // 7 → 1
    };
    
    if (ledFaceId >= 0 && ledFaceId < MAX_FACES) {
        return ledToViewMap[ledFaceId];
    }
    
    return ledFaceId; // 範囲外の場合はそのまま返す
}
