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
      m_brightnessSlider(0, 40, 40, 80),
      m_valueBrightnessSlider(0, 120, 40, 80),
      m_hueSlider(320 - 40, 40, 40, 80),
      m_saturationSlider(320 - 40, 120, 40, 80),
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
      m_lastDraggedFace(-1)
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
    
    // ラベルの設定
    // m_settingsButton->setLabel("Settings");
    // m_resetButton->setLabel("Reset");
    // m_bottomLeftButton->setLabel("Reset");
    // m_bottomRightButton->setLabel("Pattern");
}

LumiHomeActivity::~LumiHomeActivity() {
    // リソースの解放
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
    
    // スライダーのIDを設定
    m_brightnessSlider.setId(ID_SLIDER_BRIGHTNESS);
    m_valueBrightnessSlider.setId(ID_SLIDER_VALUE_BRIGHTNESS);
    m_hueSlider.setId(ID_SLIDER_HUE);
    m_saturationSlider.setId(ID_SLIDER_SATURATION);
    
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
    
    m_bottomLeftButton->setLabel("Reset");
    m_bottomLeftButton->setColor(BLACK, TFT_LIGHTGREY);
    m_bottomLeftButton->setFontSize(1.4);
    m_bottomLeftButton->setType(BUTTON_TYPE_TEXT);
    
    m_bottomRightButton->setLabel("Pattern");
    m_bottomRightButton->setColor(BLACK, TFT_LIGHTGREY);
    m_bottomRightButton->setFontSize(1.4);
    m_bottomRightButton->setType(BUTTON_TYPE_TEXT);
    
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
    
    // スライダーのタイトル設定
    m_brightnessSlider.setTitle("B");
    m_valueBrightnessSlider.setTitle("V");
    m_hueSlider.setTitle("H");
    m_saturationSlider.setTitle("S");
    
    // コールバック関数の設定
    onTopRightButtonTapped = [this]() {
        Serial.println("====== CHANGING STATE TO MENU ======");
        
        // 画面をクリアして状態を変更
        M5.Lcd.fillScreen(TFT_BLACK);
        
        // 次のループで確実に初期描画されるようにuiManagerを強制リセット
        // ここではStateManagerを直接操作しないため、OctaControllerに任せる
        
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
        }
        
        // センターボタン情報を更新
        updateCenterButtonInfo();
        
        Serial.println("All LEDs reset");
    };
    
    // 左下ボタン（リセットボタン）- 全LED消灯
    onBottomLeftButtonTapped = [this]() {
        Serial.println("====== BOTTOM LEFT RESET BUTTON PRESSED ======");
        
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
        }
        
        // フォーカスもすべて解除
        m_octagon.clearAllFocus();
        
        // センターボタン情報を更新
        updateCenterButtonInfo();
        
        Serial.println("All LEDs reset and focus cleared");
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
            
            // パターン実行中の場合は停止
            if (m_ledManager->isPatternRunning()) {
                m_ledManager->stopPattern();
            }
            
            // マイク開始
            m_micManager->startTask([this](const std::array<double, 8>& bandLevels, double bpm) {
                Serial.println("FFT callback received");
                // デバッグ出力
                for (int i = 0; i < 8; i++) {
                    Serial.print(bandLevels[i]);
                    Serial.print(" ");
                }
                Serial.println();

                // 各面の LED を FFT 結果に基づいて更新
                for (int face = 0; face < NUM_FACES; face++) {
                    uint8_t brightness = constrain(map(bandLevels[face], 0, 20000, 0, 255), 0, 255);

                    // 低音側から高音側へグラデーション（例：色相を 0～160 にマッピング）
                    CHSV color = CHSV(map(face, 0, NUM_FACES - 1, 0, 160), 255, brightness);
                    int ledFaceId = mapViewFaceToLedFace(face);
                    m_ledManager->lightFace(ledFaceId, color);

                    // 一定以上の輝度なら Octagon の面をハイライト
                    m_octagon.setFaceHighlighted(face, brightness > 50);
                }

                // ドミナントな帯域（最も大きな振幅）の検出と中央面の強調表示
                int dominantBand = 0;
                double maxLevel = 0;
                for (int i = 0; i < 8; i++) {
                    if (bandLevels[i] > maxLevel) {
                        maxLevel = bandLevels[i];
                        dominantBand = i;
                    }
                }

                // 中央画面にデバッグ情報を表示
                String bpmText = bpm > 0 ? String(bpm) : "detecting...";
                String levelText = "bands: " + String(dominantBand) + "\nbpm: " + bpmText;
                drawCenterButtonInfo(levelText, TFT_CYAN);
                Serial.println(levelText);
            });
            
            // 中央にマイクモードを表示
            drawCenterButtonInfo("MIC", TFT_CYAN);
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
            bool turnOn = (focusedFacesLedState == 0 || focusedFacesLedState == 1);
            
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
            drawCenterButtonInfo("MIC", TFT_CYAN);
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
        
        m_brightnessSlider.draw();
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
        
        m_valueBrightnessSlider.draw();
        
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
        
        m_hueSlider.draw();

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
        
        m_saturationSlider.draw();
    };

    addFragment(m_settingsButton, "settingsButton");
    addFragment(m_resetButton, "resetButton");
    addFragment(m_bottomLeftButton, "bottomLeftButton");
    addFragment(m_bottomRightButton, "bottomRightButton");

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
            // 実装は省略
            return true;
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
    m_brightnessSlider.draw();
    m_valueBrightnessSlider.draw();
    m_hueSlider.draw();
    m_saturationSlider.draw();
    
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
    if (isPressed) {
        m_lastTouchX = touchX;
        m_lastTouchY = touchY;
    }
    
    // ボタンのタッチ処理は ButtonFragment が自動的に処理するため不要
    
    // スライダーのタッチ処理
    if (checkSliderTouch(m_brightnessSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkSliderTouch(m_hueSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkSliderTouch(m_saturationSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkSliderTouch(m_valueBrightnessSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    
    // オクタゴンの中心タップ判定
    if (wasPressed && isCenterTapped(touchX, touchY)) {
        m_activeTouchedUI = TouchedUI(ID_OCTAGON_CENTER);
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
                    }
                }
            }
        }
    }
    else if (m_currentMode == MODE_PATTERN) {
        // パターンモードの場合
        if (wasPressed) {
            if (isCenterTapped(touchX, touchY)) {
                // センターがタップされた場合はパターン再生トグル
                m_isPatternPlaying = !m_isPatternPlaying;
                updateCenterButtonInfo();
            }

            int faceId = getTappedFace(touchX, touchY);
            if (faceId == 3) {
                updatePatternSelection(-1);
            } else if (faceId == 7) {
                updatePatternSelection(1);
            }
        }
    }
    
    // タッチ終了時の処理
    if (wasReleased) {
        if (m_activeTouchedUI.id == ID_OCTAGON_CENTER) {
            // センター再描画で視覚フィードバックを元に戻す
            m_octagon.drawCenter();
            updateCenterButtonInfo(); // 情報表示を更新
            
            // 同じ領域上でリリースされた場合のみアクション実行
            if (isCenterTapped(touchX, touchY) && onCenterTapped) {
                onCenterTapped();
            }
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
            
            // 同じ面上でリリースされた場合のみアクション実行
            int releasedFaceId = getTappedFace(touchX, touchY);
            if (releasedFaceId == faceId && onFaceTapped) {
                onFaceTapped(faceId);
            }
        }
        
        // タッチ情報をリセット
        m_activeTouchedUI = TouchedUI();
    }
}


// スライダータッチ処理のヘルパーメソッド
bool LumiHomeActivity::checkSliderTouch(Slider& slider, int touchX, int touchY, bool isPressed, bool wasPressed, bool wasReleased) {
    // スライダー領域内のタッチかチェック
    bool isTouchInSlider = slider.containsPoint(touchX, touchY);
    
    // タッチ開始時
    if (wasPressed && isTouchInSlider) {
        m_activeTouchedUI = TouchedUI(slider.getId());
        slider.handleTouch(touchX, touchY, true);
        
        // スライダー値変更のコールバック
        int id = slider.getId();
        int value = slider.getValue();
        
        if (id == ID_SLIDER_BRIGHTNESS && onBrightnessChanged) {
            onBrightnessChanged(value);
        }
        else if (id == ID_SLIDER_HUE && onHueChanged) {
            onHueChanged(value);
        }
        else if (id == ID_SLIDER_SATURATION && onSaturationChanged) {
            onSaturationChanged(value);
        }
        else if (id == ID_SLIDER_VALUE_BRIGHTNESS && onValueBrightnessChanged) {
            onValueBrightnessChanged(value);
        }
        
        return true;
    }
    
    // ドラッグ中の処理
    if (isPressed && !wasPressed && !wasReleased && m_activeTouchedUI.id == slider.getId()) {
        if (slider.handleTouch(touchX, touchY, true)) {
            // スライダー値変更のコールバック
            int id = slider.getId();
            int value = slider.getValue();
            
            if (id == ID_SLIDER_BRIGHTNESS && onBrightnessChanged) {
                onBrightnessChanged(value);
            }
            else if (id == ID_SLIDER_HUE && onHueChanged) {
                onHueChanged(value);
            }
            else if (id == ID_SLIDER_SATURATION && onSaturationChanged) {
                onSaturationChanged(value);
            }
            else if (id == ID_SLIDER_VALUE_BRIGHTNESS && onValueBrightnessChanged) {
                onValueBrightnessChanged(value);
            }
        }
        return true;
    }
    
    // タッチ終了時
    if (wasReleased && m_activeTouchedUI.id == slider.getId()) {
        slider.handleTouch(touchX, touchY, false);
        m_activeTouchedUI = TouchedUI();
        return true;
    }
    
    return false;
}

// センターボタン情報の描画
void LumiHomeActivity::drawCenterButtonInfo(const String& text, uint16_t color) {
    int centerX = 160; // オクタゴンの中心X座標
    int centerY = 120; // オクタゴンの中心Y座標

    // 中央円を背景色で描画
    float innerRadius = 30; // 中心円の半径
    M5.Lcd.fillCircle(centerX, centerY, innerRadius, BLACK);
    
    // テキスト表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(color);
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
        
    // テキスト情報を表示
    drawCenterButtonInfo(text, color);
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
    
    // ボタンを再描画
    m_bottomRightButton->draw();
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
