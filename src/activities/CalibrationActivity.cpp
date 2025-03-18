#include "CalibrationActivity.h"
#include <M5Unified.h>

CalibrationActivity::CalibrationActivity()
    : Activity(0, "CalibrationActivity"),
      m_faceDetector(nullptr),
      m_imuSensor(nullptr),
      m_uiManager(nullptr),
      m_resetButton(nullptr),
      m_saveButton(nullptr),
      m_loadButton(nullptr),
      m_homeButton(nullptr),
      m_calibrationState(WAIT_STABLE)
{
}

CalibrationActivity::~CalibrationActivity() {
    delete m_resetButton;
    delete m_saveButton;
    delete m_loadButton;
    delete m_homeButton;
}

bool CalibrationActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    
    // ButtonFragment インスタンスの作成
    m_resetButton = new ButtonFragment(ID_BUTTON_RESET);
    m_saveButton = new ButtonFragment(ID_BUTTON_SAVE);
    m_loadButton = new ButtonFragment(ID_BUTTON_LOAD);
    m_homeButton = new ButtonFragment(ID_BUTTON_HOME);
    
    // ButtonFragment の作成
    m_resetButton->onCreate();
    m_saveButton->onCreate();
    m_loadButton->onCreate();
    m_homeButton->onCreate();
    
    // ボタンの位置とサイズを設定 - サイズを大きくして視認性を向上
    m_resetButton->setDisplayArea(20, 60, 80, 40);
    m_saveButton->setDisplayArea(120, 60, 80, 40);
    m_loadButton->setDisplayArea(220, 60, 80, 40);
    m_homeButton->setDisplayArea(120, 220, 80, 40);
    
    // ボタンのスタイル設定 - 色を変更して視認性を向上
    m_resetButton->setLabel("Reset");
    m_resetButton->setColor(TFT_RED, TFT_LIGHTGREY);
    m_resetButton->setFontSize(1.5);
    m_resetButton->setType(BUTTON_TYPE_TEXT);
    
    m_saveButton->setLabel("Save");
    m_saveButton->setColor(TFT_GREEN, TFT_LIGHTGREY);
    m_saveButton->setFontSize(1.5);
    m_saveButton->setType(BUTTON_TYPE_TEXT);
    
    m_loadButton->setLabel("Load");
    m_loadButton->setColor(TFT_YELLOW, TFT_LIGHTGREY);
    m_loadButton->setFontSize(1.5);
    m_loadButton->setType(BUTTON_TYPE_TEXT);
    
    m_homeButton->setLabel("Home");
    m_homeButton->setColor(TFT_BLUE, TFT_LIGHTGREY);
    m_homeButton->setFontSize(1.5);
    m_homeButton->setType(BUTTON_TYPE_TEXT);
    
    // クリックハンドラの設定
    m_resetButton->setClickHandler([this]() {
        m_faceDetector->resetFaces();
        draw(); // 画面を更新
    });
    
    m_saveButton->setClickHandler([this]() {
        m_faceDetector->saveFaces();
        m_uiManager->showDialog("Info", "Faces saved successfully.", DIALOG_OK);
    });
    
    m_loadButton->setClickHandler([this]() {
        m_faceDetector->loadFaces();
        m_uiManager->showDialog("Info", "Faces loaded successfully.", DIALOG_OK);
    });
    
    m_homeButton->setClickHandler([this]() {
        Serial.println("CalibrationActivity: Home button clicked");
        if (onHomeRequested) {
            Serial.println("CalibrationActivity: Calling onHomeRequested callback");
            onHomeRequested();
        } else {
            Serial.println("CalibrationActivity: onHomeRequested callback is not set");
        }
    });
    
    // Fragmentの追加
    addFragment(m_resetButton, "resetButton");
    addFragment(m_saveButton, "saveButton");
    addFragment(m_loadButton, "loadButton");
    addFragment(m_homeButton, "homeButton");
    
    return true;
}

bool CalibrationActivity::onStart() {
    if (!Activity::onStart()) {
        return false;
    }
    return true;
}

bool CalibrationActivity::onResume() {
    if (!Activity::onResume()) {
        return false;
    }
    
    // 状態を初期化
    m_calibrationState = WAIT_STABLE;
    m_imuSensor->resetStableTime();
    
    // 画面を描画
    draw();
    
    return true;
}

void CalibrationActivity::onPause() {
    Activity::onPause();
}

void CalibrationActivity::onStop() {
    Activity::onStop();
}

void CalibrationActivity::onDestroy() {
    Activity::onDestroy();
}

bool CalibrationActivity::handleEvent(const framework::Event& event) {
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
    }
    
    return false;
}

void CalibrationActivity::initialize(FaceDetector* faceDetector, IMUSensor* imuSensor, UIManager* uiManager) {
    m_faceDetector = faceDetector;
    m_imuSensor = imuSensor;
    m_uiManager = uiManager;
}

void CalibrationActivity::update() {
    switch (m_calibrationState) {
        case WAIT_STABLE:
            {
                float x, y, z;
                m_imuSensor->getNormalizedVector(x, y, z);
                
                // 状態情報を表示
                M5.Lcd.setTextSize(1);
                M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
                M5.Lcd.setCursor(10, 120);
                M5.Lcd.print("Waiting for calibration...\n");
                M5.Lcd.print(String(m_faceDetector->getCalibratedFacesCount()) + " faces calibrated");
                M5.Lcd.setCursor(10, 150);
                M5.Lcd.print("x=" + String(m_imuSensor->getAccX()) + 
                           "\ny=" + String(m_imuSensor->getAccY()) + 
                           "\nz=" + String(m_imuSensor->getAccZ()));
                
                // デバイスが安定しているか確認
                if (m_imuSensor->isStable(0.2, 4000)) { // しきい値、安定時間
                    m_calibrationState = DETECT_FACE;
                }
            }
            break;
            
        case DETECT_FACE:
            {
                float x, y, z;
                m_imuSensor->getNormalizedVector(x, y, z);
                int detectedFace = m_faceDetector->getNearestFace(x, y, z);
                
                if (m_faceDetector->getCalibratedFacesCount() >= 8) {
                    // 面リストがいっぱい
                    m_uiManager->showDialog("Info", "Face list is full. Please save or reset.", DIALOG_OK);
                    m_calibrationState = WAIT_STABLE;
                } else if (detectedFace != -1) {
                    // 既存の面を検出
                    String message = "Detected existing face: " + String(detectedFace) + 
                                    "\ndetect value: " + String(x) + ", " + 
                                    String(y) + ", " + String(z);
                    m_uiManager->showDialog("Info", message, DIALOG_OK);
                    m_calibrationState = WAIT_STABLE;
                } else {
                    // 新しい面を検出
                    String message = "New face detected. Add to list?\ndetect value: " + 
                                    String(x) + ", " + String(y) + ", " + String(z);
                    DialogResult result = m_uiManager->showDialog("Confirm", message, DIALOG_OK_CANCEL);
                    
                    if (result == DIALOG_OK_PRESSED) {
                        m_faceDetector->calibrateNewFace();
                    }
                    
                    m_calibrationState = WAIT_STABLE;
                }
                m_imuSensor->resetStableTime();
            }
            break;
    }
}

void CalibrationActivity::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // タイトルを描画
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString("Face Calibration", 160, 20);
    
    // ボタンを描画
    m_resetButton->draw();
    m_saveButton->draw();
    m_loadButton->draw();
    m_homeButton->draw();
    
    // 状態情報を表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(10, 120);
    M5.Lcd.print("Waiting for calibration...\n");
    M5.Lcd.print(String(m_faceDetector->getCalibratedFacesCount()) + " faces calibrated");
}
