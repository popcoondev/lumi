#include "OctaController.h"
#include "Constants.h"

OctaController::OctaController() {
    uiManager = new UIManager();
    ledManager = new LEDManager();
    imuSensor = new IMUSensor();
    faceDetector = new FaceDetector(MAX_FACES); // 最大面数
    stateManager = new StateManager();
    lumiView = new LumiView();
    micManager = new MicManager();
    currentHue = 0;           // 初期色相を0（赤）に設定
    currentSaturation = 255;  // 初期彩度を最大に設定
    currentValueBrightness = 255; // 初期明度を最大に設定
    currentLedColor = CHSV(currentHue, currentSaturation, currentSaturation); // 初期色を設定

    micCallback = [this](const std::array<double, 8>& bandLevels, double bpm) {
        Serial.println("FFT callback received");
        // デバッグ出力
        for (int i = 0; i < 8; i++) {
            Serial.print(bandLevels[i]);
            Serial.print(" ");
        }
        Serial.println();

        // 各面の LED を FFT 結果に基づいて更新
        for (int face = 0; face < NUM_FACES; face++) {
            // ここでは bandLevels[face] の値を 0～100 と仮定して 0～255 にマッピング
            // uint8_t brightness = constrain(map(bandLevels[face], 0, 100, 0, 255), 0, 255);
            uint8_t brightness = constrain(map(bandLevels[face], 0, 20000, 0, 255), 0, 255);

            // 低音側から高音側へグラデーション（例：色相を 0～160 にマッピング）
            CHSV color = CHSV(map(face, 0, NUM_FACES - 1, 0, 160), 255, brightness);
            int ledFaceId = mapViewFaceToLedFace(face);
            ledManager->lightFace(ledFaceId, color);

            // 一定以上の輝度なら Octagon の面をハイライト
            lumiView->octagon.setFaceHighlighted(face, brightness > 50);
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
        // int centerFace = NUM_FACES / 2;
        // CHSV dominantColor = CHSV(0, 0, constrain(map(maxLevel, 0, 100, 0, 255), 0, 255));
        // ledManager->lightFace(mapViewFaceToLedFace(centerFace), dominantColor);

        // 中央画面にデバッグ情報を表示
        String bpmText = bpm > 0 ? String(bpm) : "detecting...";
        String levelText = "bands: " + String(dominantBand) + "\nbpm: " + bpmText;
        lumiView->drawCenterButtonInfo(levelText, TFT_CYAN);
        Serial.println(levelText);
    };
}

OctaController::~OctaController() {
    delete uiManager;
    delete ledManager;
    delete imuSensor;
    delete faceDetector;
    delete stateManager;
    delete lumiView;
    delete micManager;
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
    lumiView->setLedPatterns(ledManager->getPatternCount());
    
    // OctagonRingViewにFaceDetectorを設定
    lumiView->octagon.setFaceDetector(faceDetector);

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
    bool hueSliderDragging = lumiView->hueSlider.isBeingDragged();
    bool saturationSliderDragging = lumiView->saturationSlider.isBeingDragged();
    bool valueBrightnessSliderDragging = lumiView->valueBrightnessSlider.isBeingDragged();
    bool sliderDragging = brightnessSliderDragging || hueSliderDragging || saturationSliderDragging || valueBrightnessSliderDragging;
    
    // リッスンモードの場合はマイク入力を処理
    if (lumiView->getOperationMode() == LumiView::MODE_LISTEN) {
        // static unsigned long lastMicUpdateTime = 0;
        // static int soundLevel = 0;
        // static int prevSoundLevel = 0;
        
        // unsigned long currentTime = millis();
        
        // // 100msごとにマイク入力を更新
        // if (currentTime - lastMicUpdateTime > 100) {
        //     // マイクからの入力を取得
        //     int16_t sample[128];
            
        //     // マイクからサンプルを読み取り
        //     size_t count = M5.Mic.record(sample, 128, 16000);
            
        //     // 音量レベルを計算（絶対値の平均）
        //     int sum = 0;
            
        //     for (int i = 0; i < count; i++) {
        //         sum += abs(sample[i]);
        //     }
            
        //     // 平均音量を計算し、0-100の範囲にマッピング
        //     soundLevel = map(sum / count, 0, 2000, 0, 100);
        //     soundLevel = constrain(soundLevel, 0, 100);
            
        //     // 音量が変化した場合のみLEDを更新
        //     if (abs(soundLevel - prevSoundLevel) > 5) {
        //         prevSoundLevel = soundLevel;
                
        //         // 音量に応じてランダムに面を点灯
        //         // 音量が大きいほど多くの面が点灯
        //         int facesToLight = map(soundLevel, 0, 100, 0, NUM_FACES);
                
        //         // 全ての面をリセット
        //         ledManager->resetAllLeds();
        //         for (int i = 0; i < NUM_FACES; i++) {
        //             lumiView->octagon.setFaceHighlighted(i, false);
        //         }
                
        //         // ランダムな面を点灯
        //         for (int i = 0; i < facesToLight; i++) {
        //             int faceId = random(NUM_FACES);
                    
        //             // ランダムな色を生成
        //             CRGB color = CHSV(random(256), 255, 255);
                    
        //             // LEDを点灯
        //             int ledFaceId = mapViewFaceToLedFace(faceId);
        //             ledManager->lightFace(ledFaceId, color);
                    
        //             // OctagonRingViewのハイライトも設定
        //             lumiView->octagon.setFaceHighlighted(faceId, true);
        //         }
                
        //         // 音量レベルを表示
        //         String levelText = "Level: " + String(soundLevel);
        //         lumiView->drawCenterButtonInfo(levelText, TFT_CYAN);
        //     }
            
        //     lastMicUpdateTime = currentTime;
        // }
    }
    
    if (sliderDragging) {
        unsigned long currentTime = millis();
        // スライダードラッグ中は50ms間隔で操作中のスライダーのみを再描画
        if (currentTime - lastDrawTime >= 50) {
            // 操作中のスライダーのみを再描画
            if (brightnessSliderDragging) {
                lumiView->drawBrightnessSlider();
            } else if (hueSliderDragging) {
                lumiView->drawHueSlider();
            } else if (saturationSliderDragging) {
                lumiView->drawSaturationSlider();
            } else if (valueBrightnessSliderDragging) {
                lumiView->drawValueBrightnessSlider();
            }
            else {
                // 何もしない
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
        lumiView->onTopRightButtonTapped = [this]() {
            Serial.println("====== CHANGING STATE TO MENU ======");
            
            // 画面をクリアして状態を変更
            M5.Lcd.fillScreen(TFT_BLACK);
            stateManager->changeState(STATE_NONE);
            
            // 次のループで確実に初期描画されるようにuiManagerを強制リセット
            uiManager->forcefullyRedraw();
            
            Serial.println("State changed to: " + String(stateManager->getCurrentStateInfo().mainState));
        };
        
        // 左上ボタン（リセットボタン）
        lumiView->onTopLeftButtonTapped = [this]() {
            Serial.println("====== RESET BUTTON PRESSED ======");
            
            // 実行中のLEDパターンを停止
            ledManager->stopPattern();
            
            // 全てのLEDを消灯
            ledManager->resetAllLeds();
            
            // FaceDetectorのLED状態も更新
            if (faceDetector->getCalibratedFacesCount() > 0) {
                FaceData* faceList = faceDetector->getFaceList();
                for (int i = 0; i < faceDetector->getCalibratedFacesCount(); i++) {
                    faceList[i].ledState = 0;
                }
            }
            
            // OctagonRingViewの全てのハイライトを解除
            for (int i = 0; i < NUM_FACES; i++) {
                lumiView->octagon.setFaceHighlighted(i, false);
            }
            
            // 再描画フラグを設定
            needsRedraw = true;
            
            // センターボタン情報を更新
            lumiView->updateCenterButtonInfo();
            
            Serial.println("All LEDs reset");
        };
        
        // 左下ボタン（リセットボタン）- 全LED消灯
        lumiView->onBottomLeftButtonTapped = [this]() {
            Serial.println("====== BOTTOM LEFT RESET BUTTON PRESSED ======");
            
            // 実行中のLEDパターンを停止
            ledManager->stopPattern();
            
            // 全てのLEDを消灯
            ledManager->resetAllLeds();
            
            // FaceDetectorのLED状態も更新
            if (faceDetector->getCalibratedFacesCount() > 0) {
                FaceData* faceList = faceDetector->getFaceList();
                for (int i = 0; i < faceDetector->getCalibratedFacesCount(); i++) {
                    faceList[i].ledState = 0;
                }
            }
            
            // OctagonRingViewの全てのハイライトを解除
            for (int i = 0; i < NUM_FACES; i++) {
                lumiView->octagon.setFaceHighlighted(i, false);
            }
            
            // フォーカスもすべて解除
            lumiView->clearAllFocus();
            
            // 再描画フラグを設定
            needsRedraw = true;
            
            // センターボタン情報を更新
            lumiView->updateCenterButtonInfo();
            
            Serial.println("All LEDs reset and focus cleared");
        };

        // 面タップでフォーカスをトグル（LumiView内で処理済み）
        lumiView->onFaceTapped = [this](int faceId) {
            // デバッグ出力 - タップされた面ID
            Serial.println("Face tapped: " + String(faceId));
            
            // 無効な面IDの場合は何もしない
            if (faceId < 0 || faceId >= MAX_FACES) {
                return;
            }
            
            // フォーカス処理はLumiView内で実装済み
            // ここでは何もしない
        };
        
        // 右下ボタンでモード切替
        lumiView->onBottomRightButtonTapped = [this]() {
            // モード切替
            if (lumiView->getOperationMode() == LumiView::MODE_TAP) {
                // タップモード → パターンモード
                lumiView->setOperationMode(LumiView::MODE_PATTERN);
                
                // パターンモードの初期設定
                lumiView->selectedPatternIndex = ledManager->getCurrentPatternIndex();
                lumiView->isPatternPlaying = ledManager->isPatternRunning();
                
            } else if (lumiView->getOperationMode() == LumiView::MODE_PATTERN) {
                // パターンモード → リッスンモード
                lumiView->setOperationMode(LumiView::MODE_LISTEN);
                
                // パターン実行中の場合は停止
                if (ledManager->isPatternRunning()) {
                    ledManager->stopPattern();
                }
                
                // マイク開始
                micManager->startTask(micCallback);
                
                // 中央にマイクモードを表示
                lumiView->drawCenterButtonInfo("MIC", TFT_CYAN);
            } else {
                // リッスンモード → タップモード
                lumiView->setOperationMode(LumiView::MODE_TAP);
                
                // マイク停止
                micManager->stopTask();
                
                // 全てのLEDを消灯
                ledManager->resetAllLeds();
            }
            
            // 再描画
            needsRedraw = true;
        };
        
        // 中央タップでモードに応じた処理
        lumiView->onCenterTapped = [this]() {
            if (lumiView->getOperationMode() == LumiView::MODE_TAP) {
                // タップモードの場合
                int focusedFacesLedState = lumiView->getFocusedFacesLedState();
                int focusedFacesCount = lumiView->getFocusedFacesCount();
                
                // フォーカスがない場合は何もしない
                if (focusedFacesCount == 0) {
                    return;
                }
                
                // フォーカスされた面のLED状態に基づいて処理
                bool turnOn = (focusedFacesLedState == 0 || focusedFacesLedState == 1);
                
                // フォーカスされた全ての面に対して処理
                for (int viewFaceId = 0; viewFaceId < NUM_FACES; viewFaceId++) {
                    if (lumiView->isFaceFocused(viewFaceId)) {
                        // OctagonRingViewの面IDをLEDの面IDに変換
                        int ledFaceId = mapViewFaceToLedFace(viewFaceId);
                        
                        if (turnOn) {
                            // 点灯処理
                            ledManager->lightFace(ledFaceId, currentLedColor);
                            
                            // FaceDetectorのLED状態も更新
                            if (faceDetector->getCalibratedFacesCount() > 0) {
                                FaceData* faceList = faceDetector->getFaceList();
                                if (ledFaceId < faceDetector->getCalibratedFacesCount()) {
                                    faceList[ledFaceId].ledState = 1;
                                    faceList[ledFaceId].ledColor = currentLedColor;
                                }
                            }
                            
                            // OctagonRingViewのハイライトも設定
                            lumiView->octagon.setFaceHighlighted(viewFaceId, true);
                        } else {
                            // 消灯処理
                            ledManager->lightFace(ledFaceId, CRGB::Black);
                            
                            // FaceDetectorのLED状態も更新
                            if (faceDetector->getCalibratedFacesCount() > 0) {
                                FaceData* faceList = faceDetector->getFaceList();
                                if (ledFaceId < faceDetector->getCalibratedFacesCount()) {
                                    faceList[ledFaceId].ledState = 0;
                                }
                            }
                            
                            // OctagonRingViewのハイライトも解除
                            lumiView->octagon.setFaceHighlighted(viewFaceId, false);
                        }
                    }
                }
                
                // ハイライト色を設定
                lumiView->octagon.setHighlightColor(crgbToRGB565(currentLedColor));
            } else if (lumiView->getOperationMode() == LumiView::MODE_PATTERN) {
                // パターンモードの場合
                if (lumiView->isPatternPlaying) {
                    // パターン停止
                    ledManager->stopPattern();
                    lumiView->isPatternPlaying = false;
                } else {
                    // パターン再生
                    ledManager->runPattern(lumiView->selectedPatternIndex);
                    lumiView->isPatternPlaying = true;
                    
                }
            } else if (lumiView->getOperationMode() == LumiView::MODE_LISTEN) {
                // リッスンモードの場合は何もしない
                // 音量に応じて自動的にLEDが点灯するため
                lumiView->drawCenterButtonInfo("MIC", TFT_CYAN);
            }
            
            // 再描画フラグを設定
            needsRedraw = true;
            
            // センターボタン情報を更新
            lumiView->updateCenterButtonInfo();
        };
        
        // 明るさスライダーでLED輝度を制御（静的変数のキャプチャを回避）
        lumiView->onBrightnessChanged = [this](int value) {
            uint8_t brightness = map(value, 0, 100, 0, 255);
            ledManager->setBrightness(brightness);
            
            // FaceDataの輝度も更新
            if (faceDetector->getCalibratedFacesCount() > 0) {
                FaceData* faceList = faceDetector->getFaceList();
                for (int i = 0; i < faceDetector->getCalibratedFacesCount(); i++) {
                    if (faceList[i].ledState == 1) {
                        faceList[i].ledBrightness = brightness;
                    }
                }
            }
            
            // 再描画を即時実行する代わりに次回ループで行う
            needsRedraw = true;
            
            // センターボタン情報を更新
            lumiView->updateCenterButtonInfo();
            
        };
        
        // 明度スライダーでLED明度を制御
        lumiView->onValueBrightnessChanged = [this](int value) {
            currentValueBrightness = map(value, 0, 100, 0, 255);
            currentLedColor = CHSV(currentHue, currentSaturation, currentValueBrightness);
            
            // フォーカスされた面のみ色を更新
            for (int viewFace = 0; viewFace < NUM_FACES; viewFace++) {
                if (lumiView->isFaceFocused(viewFace) && lumiView->octagon.isFaceHighlighted(viewFace)) {
                    // OctagonRingViewの面IDをLEDの面IDに変換
                    int ledFace = mapViewFaceToLedFace(viewFace);
                    ledManager->lightFace(ledFace, currentLedColor);
                    
                    // FaceDataの色も更新
                    if (faceDetector->getCalibratedFacesCount() > 0) {
                        FaceData* faceList = faceDetector->getFaceList();
                        if (ledFace < faceDetector->getCalibratedFacesCount()) {
                            faceList[ledFace].ledColor = currentLedColor;
                        }
                    }
                }
            }
            
            // 再描画を即時実行する代わりに次回ループで行う
            needsRedraw = true;
            
            // センターボタン情報を更新
            lumiView->updateCenterButtonInfo();
            
        };
        

        // カラースライダーでLED色相を制御
        lumiView->onHueChanged = [this](int value) {
            // 色相を0-255にマップ
            currentHue = map(value, 0, 100, 0, 255);
            currentLedColor = CHSV(currentHue, currentSaturation, currentValueBrightness);
            
            // フォーカスされた面のみ色を更新
            for (int viewFace = 0; viewFace < NUM_FACES; viewFace++) {
                if (lumiView->isFaceFocused(viewFace) && lumiView->octagon.isFaceHighlighted(viewFace)) {
                    // OctagonRingViewの面IDをLEDの面IDに変換
                    int ledFace = mapViewFaceToLedFace(viewFace);
                    ledManager->lightFace(ledFace, currentLedColor);
                    
                    // FaceDataの色も更新
                    if (faceDetector->getCalibratedFacesCount() > 0) {
                        FaceData* faceList = faceDetector->getFaceList();
                        if (ledFace < faceDetector->getCalibratedFacesCount()) {
                            faceList[ledFace].ledColor = currentLedColor;
                        }
                    }
                }
            }
            
            // OctagonRingViewのハイライト色も更新
            lumiView->octagon.setHighlightColor(crgbToRGB565(currentLedColor));
            
            // 再描画を即時実行する代わりに次回ループで行う
            needsRedraw = true;
            
            // センターボタン情報を更新
            lumiView->updateCenterButtonInfo();
           
        };
        
        // 彩度スライダーでLED彩度を制御
        lumiView->onSaturationChanged = [this](int value) {
            // 彩度を0-255にマップ
            currentSaturation = map(value, 0, 100, 0, 255);
            currentLedColor = CHSV(currentHue, currentSaturation, currentValueBrightness);
            
            // フォーカスされた面のみ色を更新
            for (int viewFace = 0; viewFace < NUM_FACES; viewFace++) {
                if (lumiView->isFaceFocused(viewFace) && lumiView->octagon.isFaceHighlighted(viewFace)) {
                    // OctagonRingViewの面IDをLEDの面IDに変換
                    int ledFace = mapViewFaceToLedFace(viewFace);
                    ledManager->lightFace(ledFace, currentLedColor);
                    
                    // FaceDataの色も更新
                    if (faceDetector->getCalibratedFacesCount() > 0) {
                        FaceData* faceList = faceDetector->getFaceList();
                        if (ledFace < faceDetector->getCalibratedFacesCount()) {
                            faceList[ledFace].ledColor = currentLedColor;
                        }
                    }
                }
            }
            
            // OctagonRingViewのハイライト色も更新
            lumiView->octagon.setHighlightColor(crgbToRGB565(currentLedColor));
            
            // 再描画を即時実行する代わりに次回ループで行う
            needsRedraw = true;
            
            // センターボタン情報を更新
            lumiView->updateCenterButtonInfo();
            
        };
        
        callbacksInitialized = true;
        needsRedraw = true;  // 初回は必ず描画する
    }
    
    // 初回表示または再描画が必要な場合のみ描画
    // スライダードラッグ中はスライダーのみを更新するため除外
    if ((initialDraw || requiresRedraw) && !sliderDragging) {
        // 初回表示または再描画フラグが立っている場合は描画
        lumiView->draw();
        
        // センターボタン情報を表示
        lumiView->updateCenterButtonInfo();
        
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
                    
                    // FaceDataの状態も更新
                    if (faceDetector->getCalibratedFacesCount() > 0) {
                        FaceData* faceList = faceDetector->getFaceList();
                        if (ledFaceId < faceDetector->getCalibratedFacesCount()) {
                            faceList[ledFaceId].ledState = 1;
                            faceList[ledFaceId].ledColor = currentLedColor;
                        }
                    }
                    
                    // UI更新 - ハイライト色も設定
                    uiManager->highlightFace(detectedFace);
                    
                    // LumiViewのOctagonRingViewも更新
                    lumiView->octagon.setFaceHighlighted(detectedFace, true);
                    lumiView->octagon.setHighlightColor(crgbToRGB565(currentLedColor));
                    
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
