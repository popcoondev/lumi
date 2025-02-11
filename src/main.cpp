// プログラムの概要
// 正20面体の中心に配置されるM5Stack CoreS3用のプログラム
// 機能1: センサーで検出した加速度から、どの面が上になっているかを判定する
//  判定した面をLCDに表示する
// 機能2: 機能1の面を判定するための事前キャリブレーション
// 　機能開始してから特定の面を上にして、ボタンを押すとその面をキャリブレーションする
//  最大で20面までキャリブレーションし、LCDおよびシリアルモニタにデータ表示する
// 機能3: 面にそれぞれ配置されたLEDを点灯させる
// 　機能1で判定した面に対応するLEDを点灯させる


#include <Arduino.h>
#include <M5Unified.h>

#define STABLE_THRESHOLD 0.2 // しきい値 (m/s^2)
#define STABLE_DURATION 1000 // 判定時間 (ms)


// IMU（MPU6886）データ
float accX, accY, accZ;

// 直前の判定
int lastFace = -1;
unsigned long stableStartTime = 0;

struct FaceData {
    int id;        // 面番号
    float x, y, z; // 重力加速度の理想値
};

// 正20面体の20面の加速度基準値 (仮の値)
FaceData faceList[20] = {

};

// 最も近い面を探す
int getNearestFace(float x, float y, float z) {
    int closestFace = -1;
    float minDistance = 9999;

    for (int i = 0; i < 20; i++) {
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

void setup() {
    M5.begin();
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Detecting Face...");
    
    if (!M5.Imu.begin()) {
        M5.Lcd.println("IMU Init Failed!");
        while (1);  // センサーが見つからない場合は停止
    }
}

void loop() {
    M5.update();
    M5.Imu.getAccel(&accX, &accY, &accZ);

    // 正規化して -1.0 ~ 1.0 の範囲に
    float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
    float normX = accX / mag;
    float normY = accY / mag;
    float normZ = accZ / mag;

    // どの面に近いか
    int detectedFace = getNearestFace(normX, normY, normZ);

    // しきい値内で安定しているか判定
    if (detectedFace != lastFace) {
        stableStartTime = millis();  // 新しい面が検出されたら時間リセット
    }

    if (millis() - stableStartTime > STABLE_DURATION) {
        // 判定時間内で変わっていなければ確定
        if (detectedFace != lastFace) {
            lastFace = detectedFace;
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(10, 10);
            M5.Lcd.printf("Face Up: %d", detectedFace);
        }
    }

    delay(100);  // 更新頻度を調整
}
