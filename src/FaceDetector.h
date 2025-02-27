#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#include <Arduino.h>
#include <M5Unified.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "Constants.h"

// IMUセンサークラス
class IMUSensor {
private:
    float accX, accY, accZ;
    float prevAccX, prevAccY, prevAccZ;
    unsigned long lastStableTime;

public:
    IMUSensor();
    void begin();
    void update();
    bool isStable(float threshold, int duration);
    void resetStableTime() { lastStableTime = 0; }
    void getNormalizedVector(float& x, float& y, float& z);
    float getAccX() { return accX; }
    float getAccY() { return accY; }
    float getAccZ() { return accZ; }
};

// 面のデータ構造
struct FaceData {
    int id;               // 面のID
    float x, y, z;        // センサー値（重力加速度の方向ベクトル）
    int ledAddress[3];    // 面に対応するLEDテープのアドレス（最大3つ）
    int numLEDs;          // 使用するLEDの数
    int ledBrightness;    // LEDの明るさ (0~255)
    CRGB ledColor;        // LEDの色
    int ledState;         // LEDの状態（ON=1, OFF=0）
    bool isActive;        // この面がアクティブか
};

class FaceDetector {
private:
    FaceData* faceList;
    int maxFaces;
    int calibratedFaces;
    IMUSensor* imuSensor;

public:
    FaceDetector(int maxFaces);
    ~FaceDetector();
    void begin(IMUSensor* imuSensor);
    int detectFace();
    bool calibrateNewFace();
    bool loadFaces();
    bool saveFaces();
    void resetFaces();
    int getNearestFace(float x, float y, float z);
    FaceData* getFaceList() { return faceList; }
    int getCalibratedFacesCount() { return calibratedFaces; }
};

#endif // FACE_DETECTOR_H