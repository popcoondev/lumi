#include "OctaController.h"
#include "Constants.h"

OctaController::OctaController() {
    uiManager = new UIManager();
    ledManager = new LEDManager();
    imuSensor = new IMUSensor();
    faceDetector = new FaceDetector(MAX_FACES); // 最大面数
    stateManager = new StateManager();
    lumiView = new LumiView();
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
    
    // UIManagerからボタンイベントを取得
    ButtonEvent buttonEvent = uiManager->getButtonEvents();
    
    // バックボタンが押されたらメインメニューに戻る
    if (buttonEvent.isBackPressed) {
        stateManager->changeState(STATE_LUMI_HOME);
        ledManager->stopPattern();
        ledManager->resetAllLeds();
    }
    
    // 現在の状態に応じたボタンイベント処理
    handleButtonEvent(buttonEvent);
    
    // 状態の処理
    stateManager->processCurrentState();
    
    // IMUセンサーの更新
    imuSensor->update();
    
    // 状態に合わせた処理
    switch (stateManager->getCurrentStateInfo().mainState) {
        case STATE_LUMI_HOME:
            processLumiHomeState();
            break;
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
    
    // 状態に応じたUI表示の切り替え
    if (stateManager->getCurrentStateInfo().mainState == STATE_LUMI_HOME) {
        // LumiHome状態では専用のUIを描画（processLumiHomeState内で実行）
        // UIManagerによる表示は行わない
    } else {
        // その他の状態ではUIManagerによる表示を行う
        uiManager->updateUI(stateManager->getCurrentStateInfo());
    }
    
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

void OctaController::processLumiHomeState() {
    static bool callbacksInitialized = false;
    static bool initialDraw = true;
    static unsigned long lastDrawTime = 0;
    static bool needsRedraw = false;
    
    // タッチイベントが発生したか確認
    bool hasTouchEvent = M5.Touch.getDetail().wasPressed() || M5.Touch.getDetail().wasReleased();
    
    // 最初の1回だけコールバックを設定する
    if (!callbacksInitialized) {
        // 設定ボタンタップで設定画面に遷移
        lumiView->onSettingsButtonTapped = [this]() {
            stateManager->changeState(STATE_NONE);
        };
        
        // 面タップでLEDを制御
        lumiView->onFaceTapped = [this, &needsRedraw](int faceId) {
            // 現在のハイライト状態を切り替え
            int currentHighlight = lumiView->getHighlightedFace();
            if (currentHighlight == faceId) {
                lumiView->setHighlightedFace(-1);
                ledManager->lightFace(faceId, CRGB::Black); // LED消灯
            } else {
                lumiView->setHighlightedFace(faceId);
                ledManager->lightFace(faceId, CRGB::White); // LED点灯
            }
            // ハイライト状態が変わったので再描画が必要
            needsRedraw = true;
        };
        
        // 中央タップでLEDパターンを切り替え
        lumiView->onCenterTapped = [this]() {
            // パターン切り替えとLED表示
            ledManager->nextPattern();
            ledManager->runPattern(ledManager->getCurrentPatternIndex());
        };
        
        // 明るさスライダーでLED輝度を制御
        lumiView->onBrightnessChanged = [this, &needsRedraw](int value) {
            uint8_t brightness = map(value, 0, 100, 0, 255);
            ledManager->setBrightness(brightness);
            // スライダーの見た目が変わるので再描画が必要
            needsRedraw = true;
        };
        
        // カラースライダーでLED色相を制御
        lumiView->onColorChanged = [this, &needsRedraw](int value) {
            // 色相を0-255にマップ
            uint8_t hue = map(value, 0, 100, 0, 255);
            CRGB color = CHSV(hue, 255, 255);
            // 現在ハイライトされている面があれば、その色を変更
            int face = lumiView->getHighlightedFace();
            if (face >= 0) {
                ledManager->lightFace(face, color);
            }
            // スライダーの見た目が変わるので再描画が必要
            needsRedraw = true;
        };
        
        callbacksInitialized = true;
        needsRedraw = true;  // 初回は必ず描画する
    }
    
    // 初回表示またはタッチイベント発生時、あるいは再描画フラグが立っている場合のみ描画
    if (initialDraw || hasTouchEvent || needsRedraw) {
        // 現在時刻を取得
        unsigned long currentTime = millis();
        
        // 前回の描画から最小フレーム時間経過しているか
        // または初回表示の場合は無条件に描画
        if (initialDraw || (currentTime - lastDrawTime >= FRAME_TIME)) {
            lumiView->draw();
            lastDrawTime = currentTime;
            initialDraw = false;
            needsRedraw = false;  // 再描画フラグをリセット
        }
    }
    
    // LumiViewのタッチイベント処理は常に行う
    lumiView->handleTouch();
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
                    // 検出した面のLEDを点灯
                    ledManager->lightFace(detectedFace, CRGB::White);
                    
                    // UI更新
                    uiManager->highlightFace(detectedFace);
                    
                    // 状態情報更新
                    StateInfo newInfo = stateInfo;
                    newInfo.mainText = "Detected face: " + String(detectedFace);
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