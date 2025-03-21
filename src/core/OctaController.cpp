#include "OctaController.h"
#include "Constants.h"

OctaController::OctaController() {
    uiManager = new UIManager();
    ledManager = new LEDManager();
    imuSensor = new IMUSensor();
    faceDetector = new FaceDetector(MAX_FACES); // 最大面数
    stateManager = new StateManager();
    activityManager = new framework::ActivityManager();
    lumiView = new LumiView();
    lumiHomeActivity = new LumiHomeActivity();
    splashActivity = new SplashActivity();
    settingsActivity = new SettingsActivity();
    detectionActivity = new DetectionActivity();
    calibrationActivity = new CalibrationActivity();
    ledControlActivity = new LEDControlActivity();
    networkSettingsActivity = new NetworkSettingsActivity();
    screenSaverActivity = new ScreenSaverActivity();
    micManager = new MicManager();
    networkManager = new NetworkManager();
    webServerManager = new WebServerManager(ledManager);
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
    delete activityManager;
    delete lumiView;
    delete lumiHomeActivity;
    delete splashActivity;
    delete settingsActivity;
    delete detectionActivity;
    delete calibrationActivity;
    delete ledControlActivity;
    delete networkSettingsActivity;
    delete screenSaverActivity;
    delete micManager;
    delete networkManager;
    delete webServerManager;
}

void OctaController::setup() {
    // M5Stack初期化
    M5.begin();
    Serial.begin(115200);
    
    // 各マネージャの初期化（SplashActivityで使用するものを除く）
    uiManager->begin();
    ledManager->begin(LED_PIN, (8 * 2) + 1, 1); // PIN, LED数, オフセット
    imuSensor->begin();
    faceDetector->begin(imuSensor);
    stateManager->begin();
    lumiView->begin();
    lumiView->setLedPatterns(ledManager->getPatternCount());
    
    // OctagonRingViewにFaceDetectorを設定
    lumiView->octagon.setFaceDetector(faceDetector);
    
    // Activityの登録
    activityManager->registerActivity("splash", splashActivity);
    activityManager->registerActivity("home", lumiHomeActivity);
    activityManager->registerActivity("settings", settingsActivity);
    activityManager->registerActivity("detection", detectionActivity);
    activityManager->registerActivity("calibration", calibrationActivity);
    activityManager->registerActivity("ledcontrol", ledControlActivity);
    activityManager->registerActivity("networksettings", networkSettingsActivity);
    activityManager->registerActivity("screensaver", screenSaverActivity);
    
    // 各Activityの初期化
    splashActivity->onCreate();
    lumiHomeActivity->onCreate();
    settingsActivity->onCreate();
    detectionActivity->onCreate();
    calibrationActivity->onCreate();
    ledControlActivity->onCreate();
    networkSettingsActivity->onCreate();
    screenSaverActivity->onCreate();
    
    // ScreenSaverActivityにActivityManagerを設定
    screenSaverActivity->setActivityManager(activityManager);
    
    // SplashActivityに必要なマネージャーを設定
    splashActivity->setManagers(networkManager, webServerManager, ledManager);
    
    // SplashActivityの初期化完了時のコールバックを設定
    splashActivity->setInitCompletedCallback([this]() {
        // LumiHomeActivityに遷移
        activityManager->startActivity("home");
    });
    
    // SettingsActivityの初期化
    settingsActivity->initialize();
    
    // SettingsActivityのコールバック設定
    settingsActivity->setDetectionTransitionCallback([this]() {
        activityManager->startActivity("detection");
    });
    
    settingsActivity->setCalibrationTransitionCallback([this]() {
        activityManager->startActivity("calibration");
    });
    
    settingsActivity->setLEDControlTransitionCallback([this]() {
        activityManager->startActivity("ledcontrol");
    });
    
    settingsActivity->setHomeTransitionCallback([this]() {
        activityManager->startActivity("home");
    });
    
    settingsActivity->setNetworkSettingsTransitionCallback([this]() {
        Serial.println("SettingsActivity: Network button pressed");
        
        // LumiHomeActivityのListenモードが誤って起動されないようにする対策
        if (lumiHomeActivity->getOperationMode() == LumiHomeActivity::MODE_LISTEN) {
            Serial.println("Stopping Listen mode before transition");
            lumiHomeActivity->setOperationMode(LumiHomeActivity::MODE_TAP);
            micManager->stopTask();
        }
        
        activityManager->startActivity("networksettings");
    });
    
    // NetworkSettingsActivityの初期化
    // 毎回アクティビティ遷移時に再初期化するため、ここでは初期化しない
    // networkSettingsActivity->initialize(networkManager);
    
    // NetworkSettingsActivityのホーム画面遷移コールバックを設定
    networkSettingsActivity->setHomeTransitionCallback([this]() {
        Serial.println("NetworkSettingsActivity: Back to Settings button pressed");
        
        // LumiHomeActivityのListenモードが誤って起動されないようにする対策
        if (lumiHomeActivity->getOperationMode() == LumiHomeActivity::MODE_LISTEN) {
            Serial.println("Stopping Listen mode before transition");
            lumiHomeActivity->setOperationMode(LumiHomeActivity::MODE_TAP);
            micManager->stopTask();
        }
        
        activityManager->startActivity("settings");
    });
    
    // LumiHomeActivityの初期化
    lumiHomeActivity->initialize(ledManager, faceDetector, micManager);
    
    // DetectionActivityの初期化
    detectionActivity->initialize(ledManager, faceDetector, imuSensor, lumiView, uiManager);
    
    // CalibrationActivityの初期化
    calibrationActivity->initialize(faceDetector, imuSensor, uiManager);
    
    // LEDControlActivityの初期化
    ledControlActivity->initialize(ledManager);
    
    // 設定画面遷移用のコールバックを設定
    lumiHomeActivity->setSettingsTransitionCallback([this]() {
        activityManager->startActivity("settings");
    });
    
    // スクリーンセーバー画面遷移用のコールバックを設定
    lumiHomeActivity->setScreenSaverTransitionCallback([this]() {
        activityManager->startActivity("screensaver");
    });
    
    // 各Activityのホーム画面遷移コールバックを設定
    detectionActivity->setHomeTransitionCallback([this]() {
        Serial.println("OctaController: Detection -> Home callback triggered");
        activityManager->startActivity("home");
    });
    
    calibrationActivity->setHomeTransitionCallback([this]() {
        Serial.println("OctaController: Calibration -> Home callback triggered");
        activityManager->startActivity("home");
    });
    
    ledControlActivity->setHomeTransitionCallback([this]() {
        Serial.println("OctaController: LEDControl -> Home callback triggered");
        activityManager->startActivity("home");
    });
    
    // マイク入力処理用のコールバックを設定
    lumiHomeActivity->setMicCallback(micCallback);

    // 設定の読み込み
    faceDetector->loadFaces();
    
    // JSONパターンの読み込み
    if (ledManager->loadJsonPatternsFromDirectory("/leds")) {
        Serial.println("JSON patterns loaded successfully");
        Serial.print("Pattern count: ");
        Serial.println(ledManager->getJsonPatternCount());
    } else {
        Serial.println("Failed to load JSON patterns");
    }
    
    // スプラッシュ画面の表示（初期化処理はSplashActivity内で行われる）
    activityManager->startActivity("splash");
    Serial.println("Splash screen displayed, initialization started");
}

void OctaController::loop() {
    // 先にM5のボタン状態を更新
    M5.update();
    
    // 現在のActivityがSplashActivityの場合
    if (activityManager->getCurrentActivity() == splashActivity) {
        // スプラッシュ画面のアニメーションと初期化処理を更新
        splashActivity->update();
        
        // 初期化中は他の処理をスキップ
        return;
    }
    
    // ネットワーク接続を維持
    networkManager->update();
    
    // 現在のActivityがLumiHomeActivityの場合
    if (activityManager->getCurrentActivity() == lumiHomeActivity) {
        processLumiHomeState();
        return;
    }
    
    // 現在のActivityがSettingsActivityの場合
    if (activityManager->getCurrentActivity() == settingsActivity) {
        processSettingsState();
        return;
    }
    
    // 現在のActivityがDetectionActivityの場合
    if (activityManager->getCurrentActivity() == detectionActivity) {
        processDetectionActivityState();
        return;
    }
    
    // 現在のActivityがCalibrationActivityの場合
    if (activityManager->getCurrentActivity() == calibrationActivity) {
        processCalibrationActivityState();
        return;
    }
    
    // 現在のActivityがLEDControlActivityの場合
    if (activityManager->getCurrentActivity() == ledControlActivity) {
        processLEDControlActivityState();
        return;
    }
    
    // 現在のActivityがNetworkSettingsActivityの場合
    if (activityManager->getCurrentActivity() == networkSettingsActivity) {
        processNetworkSettingsActivityState();
        return;
    }
    
    // 現在のActivityがScreenSaverActivityの場合
    if (activityManager->getCurrentActivity() == screenSaverActivity) {
        processScreenSaverActivityState();
        return;
    }
    
    // 現在の状態取得
    StateInfo stateInfo = stateManager->getCurrentStateInfo();
    
    // LumiHome状態の場合は専用の処理を行い、他の処理をスキップ
    if (stateInfo.mainState == STATE_LUMI_HOME) {
        // LumiHomeActivityに切り替え
        activityManager->startActivity("home");
        return;
    }
    
    // STATE_NONEの場合はSettingsActivityに切り替え
    if (stateInfo.mainState == STATE_NONE) {
        activityManager->startActivity("settings");
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

void OctaController::processSettingsState() {
    // M5.Touchの状態を取得
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();
    
    if (wasPressed || isPressed || wasReleased) {
        framework::TouchAction action;
        if (wasPressed) action = framework::TouchAction::DOWN;
        else if (wasReleased) action = framework::TouchAction::UP;
        else action = framework::TouchAction::MOVE;
        
        framework::TouchEvent touchEvent(action, touch.x, touch.y);
        settingsActivity->handleEvent(touchEvent);
    }
}

void OctaController::processDetectionActivityState() {
    // 通常の更新処理
    detectionActivity->update();
    
    // M5.Touchの状態を取得
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();
    
    if (wasPressed || isPressed || wasReleased) {
        framework::TouchAction action;
        if (wasPressed) action = framework::TouchAction::DOWN;
        else if (wasReleased) action = framework::TouchAction::UP;
        else action = framework::TouchAction::MOVE;
        
        framework::TouchEvent touchEvent(action, touch.x, touch.y);
        detectionActivity->handleEvent(touchEvent);
    }
}

void OctaController::processCalibrationActivityState() {
    // 通常の更新処理
    calibrationActivity->update();
    
    // M5.Touchの状態を取得
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();
    
    if (wasPressed || isPressed || wasReleased) {
        framework::TouchAction action;
        if (wasPressed) action = framework::TouchAction::DOWN;
        else if (wasReleased) action = framework::TouchAction::UP;
        else action = framework::TouchAction::MOVE;
        
        framework::TouchEvent touchEvent(action, touch.x, touch.y);
        calibrationActivity->handleEvent(touchEvent);
    }
}

void OctaController::processLEDControlActivityState() {
    // デバッグログ追加
    // Serial.println("OctaController::processLEDControlActivityState() - Processing touch events");
    
    // M5.Touchの状態を取得
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();
    
    if (wasPressed || isPressed || wasReleased) {
        // デバッグログ追加
        Serial.println("OctaController: Touch detected - isPressed=" + String(isPressed) + 
                       ", wasPressed=" + String(wasPressed) + 
                       ", wasReleased=" + String(wasReleased) + 
                       ", x=" + String(touch.x) + ", y=" + String(touch.y));
        
        framework::TouchAction action;
        if (wasPressed) action = framework::TouchAction::DOWN;
        else if (wasReleased) action = framework::TouchAction::UP;
        else action = framework::TouchAction::MOVE;
        
        // デバッグログ追加
        Serial.println("OctaController: Creating TouchEvent with action=" + String((int)action) + 
                       ", x=" + String(touch.x) + ", y=" + String(touch.y));
        
        framework::TouchEvent touchEvent(action, touch.x, touch.y);
        
        // デバッグログ追加
        Serial.println("OctaController: Forwarding event to ledControlActivity->handleEvent()");
        
        bool handled = ledControlActivity->handleEvent(touchEvent);
        
        // デバッグログ追加
        Serial.println("OctaController: Event handled by LEDControlActivity: " + String(handled ? "true" : "false"));
    }
}

void OctaController::processScreenSaverActivityState() {
    
    // スクリーンセーバーの更新処理を追加
    if (screenSaverActivity->isPlaying()) {
        screenSaverActivity->update();
    }
    
    // M5.Touchの状態を取得
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();
    
    if (wasPressed || isPressed || wasReleased) {
        Serial.println("Touch detected in OctaController::processScreenSaverActivityState");
        framework::TouchAction action;
        if (wasPressed) action = framework::TouchAction::DOWN;
        else if (wasReleased) action = framework::TouchAction::UP;
        else action = framework::TouchAction::MOVE;
        
        framework::TouchEvent touchEvent(action, touch.x, touch.y);
        screenSaverActivity->handleEvent(touchEvent);
    }
}

void OctaController::processNetworkSettingsActivityState() {
    // NetworkSettingsActivityが初期化されていない場合は初期化する
    static bool initialized = false;
    if (!initialized) {
        Serial.println("Initializing NetworkSettingsActivity in processNetworkSettingsActivityState");
        networkSettingsActivity->initialize(networkManager);
        initialized = true;
    }
    
    // ネットワーク情報の定期更新
    networkSettingsActivity->update();
    
    // M5.Touchの状態を取得
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();
    
    if (wasPressed || isPressed || wasReleased) {
        framework::TouchAction action;
        if (wasPressed) action = framework::TouchAction::DOWN;
        else if (wasReleased) action = framework::TouchAction::UP;
        else action = framework::TouchAction::MOVE;
        
        framework::TouchEvent touchEvent(action, touch.x, touch.y);
        networkSettingsActivity->handleEvent(touchEvent);
    }
    
    // // 画面を定期的に更新
    // static unsigned long lastDrawTime = 0;
    // unsigned long currentTime = millis();
    // if (currentTime - lastDrawTime >= 1000) { // 1秒ごとに更新
    //     lastDrawTime = currentTime;
    //     networkSettingsActivity->draw();
    // }
}

void OctaController::processLumiHomeState() {
    static bool initialDraw = true;
    
    if (initialDraw) {
        // 初回描画
        lumiHomeActivity->draw();
        initialDraw = false;
    }
    
    // M5.Touchの状態を取得
    auto touch = M5.Touch.getDetail();
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();
    
    if (wasPressed || isPressed || wasReleased) {
        // まず、OctagonRingViewとセンターの処理を優先
        // これによりタップによるLED点灯機能を保持
        lumiHomeActivity->handleTouch();
        
        // その後、UI要素のイベント処理
        // ただし、OctagonRingViewがタッチを処理した場合は
        // 他のUI要素には伝播させない
        if (!lumiHomeActivity->isOctagonHandled()) {
            framework::TouchAction action;
            if (wasPressed) action = framework::TouchAction::DOWN;
            else if (wasReleased) action = framework::TouchAction::UP;
            else action = framework::TouchAction::MOVE;
            
            framework::TouchEvent touchEvent(action, touch.x, touch.y);
            lumiHomeActivity->handleEvent(touchEvent);
        }
    }
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
