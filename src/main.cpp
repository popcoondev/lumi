#include <Arduino.h>
#include <M5Unified.h>
#include "Toolbar.h"
#include <FastLED.h>
#include "TextView.h"

TextView mainTextView;
TextView subTextView;

// センサーのしきい値と判定時間
#define STABLE_THRESHOLD 0.2 // しきい値 (m/s^2)
#define STABLE_DURATION 1000 // 判定時間 (ms)
#define TIMEOUT_DURATION 120000 // タイムアウト (ms)

Toolbar toolbar;

#define LED_PIN 8    // Port.B (GPIO8)
#define NUM_LEDS 30  // LEDの数（必要に応じて変更）
uint8_t brightness = 255;  // 初期輝度（255: 最大）
bool ledState = false;

CRGB leds[NUM_LEDS];
// 色セットリスト
CRGB colors[] = {
    CRGB::Red,
    CRGB::Green,
    CRGB::Blue,
    CRGB::Yellow,
    CRGB::Purple,
    CRGB::Cyan,
    CRGB::White,
    CRGB::Black,
};

// IMU（MPU6886）データ
float accX, accY, accZ;

// 判定用の変数
int lastFace = -1;
unsigned long stableStartTime = 0;
unsigned long calibrationStartTime = 0;

// キャリブレーション用データ（最大20面）
struct FaceData {
    int id;
    float x, y, z; // 重力加速度の理想値
};
FaceData faceList[20];  // キャリブレーションした面データ
int calibratedFaces = 0; // 登録済みの面数

// システム状態
enum Mode { NONE, DETECTION, CALIBRATION, LED_CONTROL };
Mode currentMode = NONE; // 初期モードは面検出

// 最も近い面を探す
int getNearestFace(float x, float y, float z) {
    int closestFace = -1;
    float minDistance = 9999;

    for (int i = 0; i < calibratedFaces; i++) {
        float dx = faceList[i].x - x;
        float dy = faceList[i].y - y;
        float dz = faceList[i].z - z;
        float distance = sqrt(dx * dx + dy * dy + dz * dz);

        if (distance < minDistance) {
            minDistance = distance;
            closestFace = faceList[i].id;
        }
    }
    return closestFace;
}

// 面の判定（機能1）
void detectFace() {
    // faceList が空の場合は何もしない
    if (calibratedFaces == 0) {
        Serial.println("No Calibration Data!");
        return;
    }
    // faceList がある場合はリスト内の情報を表示
    else {
        Serial.println("Calibration Data:");
        for (int i = 0; i < calibratedFaces; i++) {
            Serial.printf("Face %d: (%.2f, %.2f, %.2f)\n", faceList[i].id, faceList[i].x, faceList[i].y, faceList[i].z);
        }
    }
    

    M5.Imu.getAccel(&accX, &accY, &accZ);

    // 正規化
    float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
    float normX = accX / mag;
    float normY = accY / mag;
    float normZ = accZ / mag;

    // どの面に近いか判定
    int detectedFace = getNearestFace(normX, normY, normZ);

    // しきい値内で安定しているか判定
    if (detectedFace != lastFace) {
        stableStartTime = millis();  // 新しい面が検出されたら時間リセット
    }

    if (millis() - stableStartTime > STABLE_DURATION) {
        // 判定時間内で変わっていなければ確定
        if (detectedFace != lastFace) {
            lastFace = detectedFace;
            mainTextView.setText(String("Face: " + String(lastFace)).c_str());
        }
    }

    delay(1000); // **安定判定のための待ち時間**
}

// キャリブレーション（機能2）
void calibrateFace() {
    if (millis() - calibrationStartTime > TIMEOUT_DURATION) {
        Serial.println("Calibration Timed Out");
        currentMode = NONE;
        return;
    }

    M5.Imu.getAccel(&accX, &accY, &accZ);

    // 正規化
    float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
    float normX = accX / mag;
    float normY = accY / mag;
    float normZ = accZ / mag;

    int detectedFace = getNearestFace(normX, normY, normZ);

    if (detectedFace == -1) {
        mainTextView.setText("Detect new face. Add?");
        toolbar.setButtonLabel(BTN_A, "OK");
        toolbar.setButtonLabel(BTN_B, "Cancel");
        toolbar.setButtonLabel(BTN_C, " ");
    } else {
        mainTextView.setText(String("Face: " + String(detectedFace) + " detected. Add?").c_str());
        toolbar.setButtonLabel(BTN_A, "OK");
        toolbar.setButtonLabel(BTN_B, " ");
        toolbar.setButtonLabel(BTN_C, " ");
    }
    
    ButtonID pressed = toolbar.getPressedButton();
    if (pressed == BTN_A && detectedFace == -1) {
        if (calibratedFaces < 20) {
            faceList[calibratedFaces] = {calibratedFaces, normX, normY, normZ};
            Serial.printf("Face %d Calibrated!\n", calibratedFaces);
            calibratedFaces++;
        }
    } else if (pressed == BTN_B) {
        currentMode = NONE;
    }
    delay(300);
    toolbar.draw();
}

// LED制御（機能3）
void controlLEDs() {
    // 機能1で取得した lastFace に対応するLEDを点灯
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(10, 10);
    mainTextView.setText("LED Control");
    
    // 実際のLED制御処理をここに追加する
}

// システムモードの変更（タッチで変更）
void checkModeChange() {
    ButtonID pressed = toolbar.getPressedButton();

    if (pressed != BTN_NONE) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(10, 10);
        
        switch (pressed) {
            case BTN_A:
                currentMode = DETECTION;
                mainTextView.setText("Mode: Face Detection");
                Serial.println("Mode: Face Detection");
                toolbar.setButtonLabel(BTN_A, "DETECT");
                toolbar.setButtonLabel(BTN_B, "CALIB");
                toolbar.setButtonLabel(BTN_C, "LED");
                break;

            case BTN_B:
                currentMode = CALIBRATION;
                mainTextView.setText("Mode: Calibration");
                Serial.println("Mode: Calibration");
                toolbar.setButtonLabel(BTN_A, "Back");
                toolbar.setButtonLabel(BTN_B, "Add");
                toolbar.setButtonLabel(BTN_C, "Reset");
                break;

            case BTN_C:
                currentMode = LED_CONTROL;
                mainTextView.setText("Mode: LED Control");
                Serial.println("Mode: LED Control");
                toolbar.setButtonLabel(BTN_A, "DETECT");
                toolbar.setButtonLabel(BTN_B, "CALIB");
                toolbar.setButtonLabel(BTN_C, "LED");
                break;

            default:
                break;
        }

        delay(200);  // **ボタンの誤反応防止**
        toolbar.draw();  // **ツールバーを再描画**
    }
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    Serial.println("System Initialized");

    // M5.Lcd.setTextSize(2);
    // M5.Lcd.fillScreen(BLACK);

    mainTextView.begin();
    // toolbarを除く画面全体にテキストを表示
    mainTextView.setPosition(0, 0, SCREEN_WIDTH/3*2, SCREEN_HEIGHT-TOOLBAR_HEIGHT);
    mainTextView.setFontSize(2);
    mainTextView.setColor(RED);
    mainTextView.setBackgroundColor(YELLOW);
    mainTextView.setText("Hi, Please touch the toolbar buttons.");
    mainTextView.draw();
    

    subTextView.begin();
    subTextView.setPosition(SCREEN_WIDTH/3*2, 0, SCREEN_WIDTH/3, SCREEN_HEIGHT-TOOLBAR_HEIGHT);
    subTextView.setFontSize(2);
    subTextView.setColor(BLUE);
    subTextView.setBackgroundColor(GREEN);
    subTextView.setText("Sub Text View");
    subTextView.draw();

    // WS2812B LEDテープのセットアップ
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();  // 初期化（すべてのLEDを消灯）
    FastLED.setBrightness(10); // 0~255の範囲で設定可能
    // すべてのLEDを点灯
    for (int i = 0; i < NUM_LEDS; i++) {
        // colors[]で一つずつ色を変更
        leds[i] = colors[i % 8];
    }
    FastLED.show();

    // ツールバーを初期化
    toolbar.begin();
    toolbar.setButtonLabel(BTN_A, "DETECT");
    toolbar.setButtonLabel(BTN_B, "CALIB");
    toolbar.setButtonLabel(BTN_C, "LED");

    if (!M5.Imu.begin()) {
        mainTextView.setText("IMU Init Failed!");
        Serial.println("IMU Init Failed!");
        while (1);  // センサーが見つからない場合は停止
    }

    mainTextView.setText("Mode: Face Detection");
    Serial.println("Mode: Face Detection");
    toolbar.draw();
}

void loop() {
    checkModeChange(); // モード変更を確認

    switch (currentMode) {
        case DETECTION:
            detectFace();
            break;
        case CALIBRATION:
            calibrateFace();
            break;
        case LED_CONTROL:
            controlLEDs();
            break;
        default:
            break;
    }

    delay(500);
    mainTextView.draw();
    subTextView.draw();
}
