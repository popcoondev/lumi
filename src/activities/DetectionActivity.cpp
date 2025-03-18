#include "DetectionActivity.h"
#include <M5Unified.h>

DetectionActivity::DetectionActivity()
    : Activity(0, "DetectionActivity"),
      m_ledManager(nullptr),
      m_faceDetector(nullptr),
      m_imuSensor(nullptr),
      m_lumiView(nullptr),
      m_uiManager(nullptr),
      m_homeButton(nullptr),
      m_lastDetectedFace(-1),
      m_currentLedColor(CRGB::White)
{
}

DetectionActivity::~DetectionActivity() {
    delete m_homeButton;
}

bool DetectionActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    
    // ホームボタンの作成
    m_homeButton = new ButtonFragment(ID_BUTTON_HOME);
    m_homeButton->onCreate();
    m_homeButton->setDisplayArea(10, 10, 60, 30);
    m_homeButton->setLabel("Back");
    m_homeButton->setColor(BLACK, TFT_DARKGREY);
    m_homeButton->setFontSize(1);
    m_homeButton->setType(BUTTON_TYPE_TEXT);
    
    // ホームボタンのクリックハンドラ設定
    m_homeButton->setClickHandler([this]() {
        if (onHomeRequested) {
            onHomeRequested();
        }
    });
    
    // Fragmentの追加
    addFragment(m_homeButton, "homeButton");
    
    return true;
}

bool DetectionActivity::onStart() {
    if (!Activity::onStart()) {
        return false;
    }
    return true;
}

bool DetectionActivity::onResume() {
    if (!Activity::onResume()) {
        return false;
    }
    
    // 画面を初期化
    draw();
    
    // 最後に検出された面をリセット
    m_lastDetectedFace = -1;
    
    return true;
}

void DetectionActivity::onPause() {
    // LEDをリセット
    if (m_ledManager) {
        m_ledManager->resetAllLeds();
    }
    
    Activity::onPause();
}

void DetectionActivity::onStop() {
    Activity::onStop();
}

void DetectionActivity::onDestroy() {
    Activity::onDestroy();
}

bool DetectionActivity::handleEvent(const framework::Event& event) {
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
    }
    
    return false;
}

void DetectionActivity::initialize(LEDManager* ledManager, FaceDetector* faceDetector, 
                                 IMUSensor* imuSensor, LumiView* lumiView, UIManager* uiManager) {
    m_ledManager = ledManager;
    m_faceDetector = faceDetector;
    m_imuSensor = imuSensor;
    m_lumiView = lumiView;
    m_uiManager = uiManager;
}

void DetectionActivity::update() {
    // 現在の方向からどの面かを検出
    float x, y, z;
    m_imuSensor->getNormalizedVector(x, y, z);
    int detectedFace = m_faceDetector->getNearestFace(x, y, z);
    
    // 検出面が前回と異なる場合のみ処理
    if (detectedFace != m_lastDetectedFace) {
        // LEDの更新
        updateLEDs(detectedFace);
        
        // 画面表示の更新
        updateDisplay(detectedFace, x, y, z);
        
        // 検出面を記録
        m_lastDetectedFace = detectedFace;
    }
}

void DetectionActivity::updateLEDs(int detectedFace) {
    // 全ての面のLEDをリセット
    for (int i = 0; i < m_faceDetector->getCalibratedFacesCount(); i++) {
        FaceData* faceList = m_faceDetector->getFaceList();
        faceList[i].ledState = 0;
    }
    m_ledManager->resetAllLeds();
    
    // 検出結果の処理
    if (detectedFace != -1) {
        // 検出した面のLEDを点灯
        int ledFaceId = mapViewFaceToLedFace(detectedFace);
        m_ledManager->lightFace(ledFaceId, m_currentLedColor);
        
        // FaceDataの状態も更新
        if (m_faceDetector->getCalibratedFacesCount() > 0) {
            FaceData* faceList = m_faceDetector->getFaceList();
            if (ledFaceId < m_faceDetector->getCalibratedFacesCount()) {
                faceList[ledFaceId].ledState = 1;
                faceList[ledFaceId].ledColor = m_currentLedColor;
            }
        }
        
        // UI更新
        m_uiManager->highlightFace(detectedFace);
        
        // LumiViewのOctagonRingViewも更新
        m_lumiView->setHighlightedFace(detectedFace);
    } else {
        // 未検出時の処理
        m_uiManager->highlightFace(-1);
    }
}

void DetectionActivity::updateDisplay(int detectedFace, float x, float y, float z) {
    // 状態情報を画面に表示
    if (detectedFace != -1) {
        int ledFaceId = mapViewFaceToLedFace(detectedFace);
        
        // 検出情報を表示
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setCursor(10, 180);
        M5.Lcd.print("Detected face: " + String(detectedFace) + "     ");
        M5.Lcd.setCursor(10, 200);
        M5.Lcd.print("LED face: " + String(ledFaceId) + "     ");
    } else {
        // 未検出時の表示
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setCursor(10, 180);
        M5.Lcd.print("No face detected     ");
        M5.Lcd.setCursor(10, 200);
        M5.Lcd.print("                     ");
    }
    
    // センサー値の表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(10, 220);
    M5.Lcd.print("x=" + String(x, 2) + "     ");
    M5.Lcd.setCursor(10, 230);
    M5.Lcd.print("y=" + String(y, 2) + "     ");
    M5.Lcd.setCursor(10, 240);
    M5.Lcd.print("z=" + String(z, 2) + "     ");
}

void DetectionActivity::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // タイトルを描画
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString("Face Detection", 160, 20);
    
    // ホームボタンを描画
    m_homeButton->draw();
    
    // 初期情報表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(10, 180);
    M5.Lcd.print("Waiting for detection...");
}

int DetectionActivity::mapViewFaceToLedFace(int viewFaceId) {
    return viewFaceId; // 必要に応じて実際のマッピングを実装
}

uint16_t DetectionActivity::crgbToRGB565(CRGB color) {
    return M5.Lcd.color565(color.r, color.g, color.b);
}
