#include "OctaController.h"
#include "Constants.h"

OctaController::OctaController() {
    uiManager = new UIManager();
    ledManager = new LEDManager();
    imuSensor = new IMUSensor();
    faceDetector = new FaceDetector(MAX_FACES); // 最大面数
    stateManager = new StateManager();
    lumiView = new LumiView();
    currentLedColor = CRGB::White; // 初期色を白に設定
}

OctaController::~OctaController() {
    delete uiManager;
    delete ledManager;
    delete imuSensor;
    delete faceDetector;
    delete stateManager;
    delete lumiView;
}

void OctaController::setup() {
    // M5Stack初期化
    M5.begin();
    Serial.begin(115200);
    
    // 各マネージャの初期化
    uiManager->begin();
    ledManager->begin(LED_PIN, (8 * 2) + 1, 1); // PIN, LED数, オフセット
    imuSensor->begin();
    faceDetector->begin(imuSensor);
    stateManager->begin();
    lumiView->begin();

    // 設定の読み込み
    faceDetector->loadFaces();
}

void OctaController::loop() {
    // 先にM5のボタン状態を更新
    M5.update();
    
    // 現在の状態取得
    StateInfo stateInfo = stateManager->getCurrentStateInfo();
    
    // LumiHome状態の場合は専用の処理を行い、他の処理をスキップ
    if (stateInfo.mainState == STATE_LUMI_HOME) {
        processLumiHomeState();
        // LumiHome状態ではUIManagerの操作をスキップして早期リターン
        return;
    }
    
    // LumiHome以外の状態では通常の処理を続行
    
    // UIManagerからボタンイベントを取得
    ButtonEvent buttonEvent = uiManager->getButtonEvents();
    
    // バックボタンが押されたらメインメニューに戻る
    if (buttonEvent.isBackPressed) {
        stateManager->changeState(STATE_LUMI_HOME);
        ledManager->stopPattern();
        ledManager->resetAllLeds();

        LumiHomeSetInitialDraw();
    }
    
    // 現在の状態に応じたボタンイベント処理
    handleButtonEvent(buttonEvent);
    
    // 状態の処理
    stateManager->processCurrentState();
    
    // IMUセンサーの更新
    imuSensor->update();
    
    // 状態に合わせた処理
    switch (stateManager->getCurrentStateInfo().mainState) {
        case STATE_DETECTION:
            processDetectionState();
            break;
        case STATE_CALIBRATION:
            processCalibrationState();
            break;
        case STATE_LED_CONTROL:
            processLEDControlState();
            break;
        default:
            // メインメニュー状態の処理
            break;
    }
    
    // UIManagerによる表示を行う
    uiManager->updateUI(stateManager->getCurrentStateInfo());
    
    // 短い遅延
    delay(10);
}

void OctaController::handleButtonEvent(ButtonEvent event) {
    StateInfo stateInfo = stateManager->getCurrentStateInfo();
    
    // Lumi Home状態のボタン処理
    if (stateInfo.mainState == STATE_LUMI_HOME) {
        return;
    }

    // メインメニューでのボタン処理
    if (stateInfo.mainState == STATE_NONE) {
        if (event.buttonA) stateManager->changeState(STATE_DETECTION);
        if (event.buttonB) stateManager->changeState(STATE_CALIBRATION);
        if (event.buttonC) stateManager->changeState(STATE_LED_CONTROL);
        return;
    }
    
    // 各状態でのボタン処理
    switch (stateInfo.mainState) {
        case STATE_CALIBRATION:
            if (event.buttonA) faceDetector->resetFaces();
            if (event.buttonB) faceDetector->saveFaces();
            if (event.buttonC) faceDetector->loadFaces();
            break;
            
        case STATE_LED_CONTROL:
            processLedControlButtons(event);
            break;
    }
}

// CRGB色をM5Stack LCD用のuint16_t色に変換する関数
uint16_t OctaController::crgbToRGB565(CRGB color) {
    return M5.Lcd.color565(color.r, color.g, color.b);
}


void OctaController::processLumiHomeState() {
    static bool callbacksInitialized = false;
    static bool initialDraw = true;
    static unsigned long lastDrawTime = 0;
    static bool needsRedraw = false;
    static bool wasDragging = false; // スライダードラッグ状態の記録
    
    // 現在のタッチ状態を取得
    auto touchDetail = M5.Touch.getDetail();
    bool isPressed = touchDetail.isPressed();
    bool wasPressed = touchDetail.wasPressed();
    bool wasReleased = touchDetail.wasReleased();
    
    // 本当に再描画が必要な状態かどうかを判断
    bool requiresRedraw = wasPressed || wasReleased || needsRedraw || initialDraw;
    
    // スライダーのドラッグ中の処理を変更
    // スライダーの状態をチェック
    bool brightnessSliderDragging = lumiView->brightnessSlider.isBeingDragged();
    bool colorSliderDragging = lumiView->colorSlider.isBeingDragged();
    bool sliderDragging = brightnessSliderDragging || colorSliderDragging;
    
    if (sliderDragging) {
        unsigned long currentTime = millis();
        // スライダードラッグ中は50ms間隔で操作中のスライダーのみを再描画
        if (currentTime - lastDrawTime >= 50) {
            // 操作中のスライダーのみを再描画
            if (brightnessSliderDragging) {
                lumiView->drawBrightnessSlider();
            } else if (colorSliderDragging) {
                lumiView->drawColorSlider();
            }
            lastDrawTime = currentTime;
        }
        wasDragging = true;
    } else if (wasDragging && wasReleased) {
        // ドラッグ終了時は必ず更新
        requiresRedraw = true;
        wasDragging = false;
    }
    
    // 最初の1回だけコールバックを設定する
    if (!callbacksInitialized) {
        // 静的変数を直接キャプチャしないようにラムダ内で参照を作成
        lumiView->onSettingsButtonTapped = [this]() {
            Serial.println("====== CHANGING STATE TO MENU ======");
            
            // 画面をクリアして状態を変更
            M5.Lcd.fillScreen(TFT_BLACK);
            stateManager->changeState(STATE_NONE);
            
            // 次のループで確実に初期描画されるようにuiManagerを強制リセット
            uiManager->forcefullyRedraw();
            
            Serial.println("State changed to: " + String(stateManager->getCurrentStateInfo().mainState));
        };
        
        // 面タップでLEDを制御
        lumiView->onFaceTapped = [this](int faceId) {
            // 現在のハイライト状態を切り替え
            int currentHighlight = lumiView->getHighlightedFace();
            
            // デバッグ出力 - タップされた面ID
            Serial.println("Face tapped: " + String(faceId));
            
            if (currentHighlight == faceId) {
                // 同じ面をタップした場合はハイライトを解除
                lumiView->setHighlightedFace(-1);
                
                // LED消灯 - OctagonRingViewの面IDをLEDの面IDに変換
                int ledFaceId = mapViewFaceToLedFace(faceId);
                ledManager->lightFace(ledFaceId, CRGB::Black);
                
                Serial.println("Unhighlighted face: " + String(faceId) + 
                            ", LED off: " + String(ledFaceId));
            } else {
                // 別の面をタップした場合は面をハイライト
                lumiView->setHighlightedFace(faceId);
                
                // 現在の色でLED点灯 - OctagonRingViewの面IDをLEDの面IDに変換
                int ledFaceId = mapViewFaceToLedFace(faceId);
                ledManager->lightFace(ledFaceId, currentLedColor);
                
                // OctagonRingViewのハイライト色も設定
                lumiView->octagon.setHighlightColor(crgbToRGB565(currentLedColor));
                
                Serial.println("Highlighted face: " + String(faceId) + 
                            ", LED on: " + String(ledFaceId));
            }
            
            // 再描画フラグを設定
            needsRedraw = true;
        };
        
        // 中央タップでLEDパターンを切り替え
        lumiView->onCenterTapped = [this]() {
            // パターン切り替えとLED表示
            ledManager->nextPattern();
            ledManager->runPattern(ledManager->getCurrentPatternIndex());
        };
        
        // 明るさスライダーでLED輝度を制御（静的変数のキャプチャを回避）
        lumiView->onBrightnessChanged = [this](int value) {
            uint8_t brightness = map(value, 0, 100, 0, 255);
            ledManager->setBrightness(brightness);
            // 再描画を即時実行する代わりに次回ループで行う
            needsRedraw = true;
        };
        
        // カラースライダーでLED色相を制御
        lumiView->onColorChanged = [this](int value) {
            // 色相を0-255にマップ
            uint8_t hue = map(value, 0, 100, 0, 255);
            currentLedColor = CHSV(hue, 255, 255);
            
            // 現在ハイライトされている面があれば、その色を変更
            int viewFace = lumiView->getHighlightedFace();
            if (viewFace >= 0) {
                // OctagonRingViewの面IDをLEDの面IDに変換
                int ledFace = mapViewFaceToLedFace(viewFace);
                ledManager->lightFace(ledFace, currentLedColor);
                
                // OctagonRingViewのハイライト色も更新
                lumiView->octagon.setHighlightColor(crgbToRGB565(currentLedColor));
            }
            
            // 再描画を即時実行する代わりに次回ループで行う
            needsRedraw = true;
        };
        
        callbacksInitialized = true;
        needsRedraw = true;  // 初回は必ず描画する
    }
    
    // 初回表示または再描画が必要な場合のみ描画
    // スライダードラッグ中はスライダーのみを更新するため除外
    if ((initialDraw || requiresRedraw) && !sliderDragging) {
        // 初回表示または再描画フラグが立っている場合は描画
        lumiView->draw();
        
        if (initialDraw) {
            initialDraw = false;
        }
        
        if (needsRedraw) {
            needsRedraw = false;  // 再描画フラグをリセット
        }
    }
    
    // LumiViewのタッチイベント処理は常に行う（ただし描画とは分離）
    lumiView->handleTouch();
}

void OctaController::LumiHomeSetInitialDraw() {
    static bool *pInitialDraw = nullptr;
    if (!pInitialDraw) {
        // 静的変数のアドレスを取得するトリック
        processLumiHomeState(); // 一度呼び出す
        void *framePtr = __builtin_frame_address(0);
        pInitialDraw = static_cast<bool*>(framePtr) - sizeof(bool); // initialDrawのアドレスを推測
    }
    if (pInitialDraw) {
        *pInitialDraw = true;
    }
}

void OctaController::processDetectionState() {
    StateInfo stateInfo = stateManager->getCurrentStateInfo();
    SubState subState = stateInfo.subState;
    
    static int lastDetectedFace = -1;

    switch (subState) {
        case STATE_DETECTION_INIT:
            stateManager->changeSubState(STATE_DETECTION_DETECT_FACE);
            break;
            
        case STATE_DETECTION_DETECT_FACE:
            // 現在の方向からどの面かを検出
            float x, y, z;
            imuSensor->getNormalizedVector(x, y, z);
            int detectedFace = faceDetector->getNearestFace(x, y, z);
            
            // 検出面が前回と異なる場合のみ処理
            if (detectedFace != lastDetectedFace) {
                // 全ての面のLEDをリセット
                for (int i = 0; i < faceDetector->getCalibratedFacesCount(); i++) {
                    FaceData* faceList = faceDetector->getFaceList();
                    faceList[i].ledState = 0;
                }
                ledManager->resetAllLeds();
                
                // 検出結果の処理
                if (detectedFace != -1) {
                    // 検出した面のLEDを点灯 - 面IDからLED番号へのマッピング修正
                    // LEDの物理的なレイアウトとOctagonRingViewの論理的なレイアウトを一致させる
                    int ledFaceId = mapViewFaceToLedFace(detectedFace);
                    ledManager->lightFace(ledFaceId, currentLedColor);
                    
                    // UI更新 - ハイライト色も設定
                    uiManager->highlightFace(detectedFace);
                    // UIManagerのOctagonRingViewにアクセスできないため、直接色は設定できない
                    // 代わりにLumiViewのoctagonに色を設定する
                    if (stateInfo.mainState == STATE_DETECTION) {
                        // 検出モードでのみ色を設定
                        lumiView->octagon.setHighlightColor(crgbToRGB565(currentLedColor));
                    }
                    
                    // 状態情報更新
                    StateInfo newInfo = stateInfo;
                    newInfo.mainText = "Detected face: " + String(detectedFace) + 
                                      "\nLED face: " + String(ledFaceId);
                    newInfo.subText = "x=" + String(x) + "\ny=" + String(y) + "\nz=" + String(z);
                    stateManager->updateStateInfo(newInfo);
                } else {
                    // 未検出時の処理
                    uiManager->highlightFace(-1);
                    
                    StateInfo newInfo = stateInfo;
                    newInfo.mainText = "No face detected";
                    newInfo.subText = "x=" + String(x) + "\ny=" + String(y) + "\nz=" + String(z);
                    stateManager->updateStateInfo(newInfo);
                }

                // 検出面を記録
                lastDetectedFace = detectedFace;
            }
            break;
    }
}

int OctaController::mapViewFaceToLedFace(int viewFaceId) {
    return viewFaceId; // 範囲外の場合はそのまま返す
}


int OctaController::mapLedFaceToViewFace(int ledFaceId) {
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

void OctaController::processCalibrationState() {
    StateInfo stateInfo = stateManager->getCurrentStateInfo();
    SubState subState = stateInfo.subState;
    
    static unsigned long stableStartTime = 0;
    
    switch (subState) {
        case STATE_CALIBRATION_INIT:
            stateManager->changeSubState(STATE_CALIBRATION_WAIT_STABLE);
            break;
            
        case STATE_CALIBRATION_WAIT_STABLE:
            {
                StateInfo newInfo = stateInfo;
                newInfo.mainText = "Waiting for calibration...\n" + 
                                  String(faceDetector->getCalibratedFacesCount()) + 
                                  " faces calibrated";
                
                float x, y, z;
                imuSensor->getNormalizedVector(x, y, z);
                newInfo.subText = "x=" + String(imuSensor->getAccX()) + 
                                 "\ny=" + String(imuSensor->getAccY()) + 
                                 "\nz=" + String(imuSensor->getAccZ());
                
                stateManager->updateStateInfo(newInfo);
                
                // デバイスが安定しているか確認
                if (imuSensor->isStable(0.2, 4000)) { // しきい値、安定時間
                    stateManager->changeSubState(STATE_CALIBRATION_DETECT_FACE);
                }
            }
            break;
            
        case STATE_CALIBRATION_DETECT_FACE:
            {
                float x, y, z;
                imuSensor->getNormalizedVector(x, y, z);
                int detectedFace = faceDetector->getNearestFace(x, y, z);
                
                if (faceDetector->getCalibratedFacesCount() >= 8) {
                    // 面リストがいっぱい
                    uiManager->showDialog("Info", "Face list is full. Please save or reset.", DIALOG_OK);
                    stateManager->changeSubState(STATE_CALIBRATION_WAIT_STABLE);
                } else if (detectedFace != -1) {
                    // 既存の面を検出
                    String message = "Detected existing face: " + String(detectedFace) + 
                                    "\ndetect value: " + String(x) + ", " + 
                                    String(y) + ", " + String(z);
                    uiManager->showDialog("Info", message, DIALOG_OK);
                    stateManager->changeSubState(STATE_CALIBRATION_WAIT_STABLE);
                } else {
                    // 新しい面を検出
                    String message = "New face detected. Add to list?\ndetect value: " + 
                                    String(x) + ", " + String(y) + ", " + String(z);
                    DialogResult result = uiManager->showDialog("Confirm", message, DIALOG_OK_CANCEL);
                    
                    if (result == DIALOG_OK_PRESSED) {
                        faceDetector->calibrateNewFace();
                    }
                    
                    stateManager->changeSubState(STATE_CALIBRATION_WAIT_STABLE);
                }
                imuSensor->resetStableTime();
            }
            break;
    }
}

void OctaController::processLEDControlState() {
    StateInfo stateInfo = stateManager->getCurrentStateInfo();
    SubState subState = stateInfo.subState;
    
    switch (subState) {
        case STATE_LED_CONTROL_INIT:
            {
                StateInfo newInfo = stateInfo;
                newInfo.buttonALabel = "play";
                newInfo.buttonBLabel = "prev";
                newInfo.buttonCLabel = "next";
                newInfo.mainText = "Pattern: " + ledManager->getCurrentPatternName();
                stateManager->updateStateInfo(newInfo);
                stateManager->changeSubState(STATE_LED_CONTROL_PAUSED);
            }
            break;
            
        case STATE_LED_CONTROL_PAUSED:
        case STATE_LED_CONTROL_PLAYING:
            // ボタン処理はprocessLedControlButtonsで行う
            break;
    }
}

void OctaController::processLedControlButtons(ButtonEvent event) {
    StateInfo stateInfo = stateManager->getCurrentStateInfo();
    SubState subState = stateInfo.subState;
    
    if (subState == STATE_LED_CONTROL_PAUSED) {
        if (event.buttonA) {
            // 再生ボタン
            ledManager->runPattern(ledManager->getCurrentPatternIndex());
            
            StateInfo newInfo = stateInfo;
            newInfo.buttonALabel = "pause";
            newInfo.mainText = "Playing: " + ledManager->getCurrentPatternName();
            stateManager->updateStateInfo(newInfo);
            stateManager->changeSubState(STATE_LED_CONTROL_PLAYING);
        }
        if (event.buttonB) {
            // 前のパターン
            ledManager->prevPattern();
            
            StateInfo newInfo = stateInfo;
            newInfo.mainText = "Pattern: " + ledManager->getCurrentPatternName();
            stateManager->updateStateInfo(newInfo);
        }
        if (event.buttonC) {
            // 次のパターン
            ledManager->nextPattern();
            
            StateInfo newInfo = stateInfo;
            newInfo.mainText = "Pattern: " + ledManager->getCurrentPatternName();
            stateManager->updateStateInfo(newInfo);
        }
    } else if (subState == STATE_LED_CONTROL_PLAYING) {
        if (event.buttonA) {
            // 一時停止ボタン
            ledManager->stopPattern();
            
            StateInfo newInfo = stateInfo;
            newInfo.buttonALabel = "play";
            newInfo.mainText = "Paused: " + ledManager->getCurrentPatternName();
            stateManager->updateStateInfo(newInfo);
            stateManager->changeSubState(STATE_LED_CONTROL_PAUSED);
        }
        if (event.buttonB) {
            // 前のパターン
            ledManager->prevPattern();
            ledManager->stopPattern();
            ledManager->runPattern(ledManager->getCurrentPatternIndex());
            
            StateInfo newInfo = stateInfo;
            newInfo.mainText = "Playing: " + ledManager->getCurrentPatternName();
            stateManager->updateStateInfo(newInfo);
        }
        if (event.buttonC) {
            // 次のパターン
            ledManager->nextPattern();
            ledManager->stopPattern();
            ledManager->runPattern(ledManager->getCurrentPatternIndex());
            
            StateInfo newInfo = stateInfo;
            newInfo.mainText = "Playing: " + ledManager->getCurrentPatternName();
            stateManager->updateStateInfo(newInfo);
        }
    }
}
