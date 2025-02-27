#include "FaceDetector.h"
#include "Constants.h"

// CRGB型をJSONから読み取るためのヘルパー関数
uint32_t getCRGBColorFromJson(JsonObject& faceObj, const char* key, uint32_t defaultColor = 0xFFFFFF) {
    if (!faceObj[key].isNull()) {
        // JSONにcolorが数値として保存されている場合
        return faceObj[key].as<uint32_t>();
    }
    return defaultColor;
}

// IMUSensorの実装
IMUSensor::IMUSensor() {
    accX = 0.0f;
    accY = 0.0f;
    accZ = 0.0f;
    prevAccX = 0.0f;
    prevAccY = 0.0f;
    prevAccZ = 0.0f;
    lastStableTime = 0;
}

void IMUSensor::begin() {
    // M5 IMUの初期化は、M5.beginで自動的に行われる
}

void IMUSensor::update() {
    M5.Imu.getAccel(&accX, &accY, &accZ);
}

bool IMUSensor::isStable(float threshold, int duration) {
    if (abs(accX - prevAccX) < threshold &&
        abs(accY - prevAccY) < threshold &&
        abs(accZ - prevAccZ) < threshold) {
        
        if (lastStableTime == 0) {
            lastStableTime = millis();
        } else if (millis() - lastStableTime > duration) {
            return true;
        }
    } else {
        lastStableTime = 0;
    }
    
    prevAccX = accX;
    prevAccY = accY;
    prevAccZ = accZ;
    
    return false;
}

void IMUSensor::getNormalizedVector(float& x, float& y, float& z) {
    float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
    x = accX / mag;
    y = accY / mag;
    z = accZ / mag;
}

// FaceDetectorの実装
FaceDetector::FaceDetector(int maxFaces) {
    this->maxFaces = maxFaces;
    faceList = new FaceData[maxFaces];
    calibratedFaces = 0;
    imuSensor = nullptr;
}

FaceDetector::~FaceDetector() {
    delete[] faceList;
}

void FaceDetector::begin(IMUSensor* imuSensor) {
    this->imuSensor = imuSensor;
    
    // SDカードの初期化
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD card initialization failed!");
    }
}

int FaceDetector::detectFace() {
    if (imuSensor == nullptr) return -1;
    
    float x, y, z;
    imuSensor->getNormalizedVector(x, y, z);
    
    return getNearestFace(x, y, z);
}

int FaceDetector::getNearestFace(float x, float y, float z) {
    int nearestFace = -1;
    float maxSimilarity = -1.0f;
    
    for (int i = 0; i < calibratedFaces; i++) {
        // 正規化された法線ベクトル
        float Nx = faceList[i].x;
        float Ny = faceList[i].y;
        float Nz = faceList[i].z;
        
        // 内積を計算（cosθに相当）
        float dotProduct = x * Nx + y * Ny + z * Nz;
        
        // 内積が最大（最も類似した面）を採用
        if (dotProduct > maxSimilarity) {
            maxSimilarity = dotProduct;
            nearestFace = faceList[i].id;
        }
    }
    
    // cosθ（dotProduct）がしきい値以上の場合のみ採用
    if (maxSimilarity > 0.95) {  // 0.95は誤差の許容範囲
        return nearestFace;
    } else {
        return -1;
    }
}

bool FaceDetector::calibrateNewFace() {
    if (imuSensor == nullptr) return false;
    if (calibratedFaces >= maxFaces) return false;
    
    float x, y, z;
    imuSensor->getNormalizedVector(x, y, z);
    
    // 既存の面とほぼ同じ向きの面がないか確認
    if (getNearestFace(x, y, z) != -1) {
        return false; // 既に似た向きの面が登録されている
    }
    
    // 新しい面を追加
    faceList[calibratedFaces].id = calibratedFaces;
    faceList[calibratedFaces].x = x;
    faceList[calibratedFaces].y = y;
    faceList[calibratedFaces].z = z;
    
    // LED設定（各面は2つのLEDを使用）
    faceList[calibratedFaces].ledAddress[0] = calibratedFaces * 2;
    faceList[calibratedFaces].ledAddress[1] = calibratedFaces * 2 + 1;
    faceList[calibratedFaces].ledAddress[2] = -1; // 未使用
    faceList[calibratedFaces].numLEDs = 2;
    
    // 初期設定
    faceList[calibratedFaces].ledBrightness = 255;
    faceList[calibratedFaces].ledColor = CRGB::White;
    faceList[calibratedFaces].ledState = 0;
    faceList[calibratedFaces].isActive = true;
    
    calibratedFaces++;
    return true;
}

bool FaceDetector::loadFaces() {
    if (!SD.exists("/faces.json")) {
        Serial.println("No face data found on SD card");
        return false;
    }
    
    File file = SD.open("/faces.json", FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    
    JsonDocument jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, file);
    if (error) {
        Serial.println("Failed to parse JSON");
        file.close();
        return false;
    }
    
    JsonArray faceArray = jsonDoc["faces"].as<JsonArray>();
    calibratedFaces = 0;
    
    for (JsonObject faceObj : faceArray) {
        if (calibratedFaces >= maxFaces) break;
        
        faceList[calibratedFaces].id = faceObj["id"];
        faceList[calibratedFaces].x = faceObj["x"];
        faceList[calibratedFaces].y = faceObj["y"];
        faceList[calibratedFaces].z = faceObj["z"];
        faceList[calibratedFaces].ledAddress[0] = faceObj["led1"];
        faceList[calibratedFaces].ledAddress[1] = faceObj["led2"];
        faceList[calibratedFaces].numLEDs = faceObj["numLEDs"];
        faceList[calibratedFaces].ledBrightness = faceObj["brightness"];
        faceList[calibratedFaces].ledColor = getCRGBColorFromJson(faceObj, "color", 0xFFFFFF);
        faceList[calibratedFaces].ledState = faceObj["state"];
        faceList[calibratedFaces].isActive = faceObj["isActive"];
        
        calibratedFaces++;
    }
    
    file.close();
    Serial.println("Loaded " + String(calibratedFaces) + " faces from SD card");
    return true;
}

bool FaceDetector::saveFaces() {
    JsonDocument jsonDoc;
    JsonArray faceArray = jsonDoc["faces"].to<JsonArray>();
    
    for (int i = 0; i < calibratedFaces; i++) {
        JsonObject faceObj = faceArray.add<JsonObject>();
        faceObj["id"] = faceList[i].id;
        faceObj["x"] = faceList[i].x;
        faceObj["y"] = faceList[i].y;
        faceObj["z"] = faceList[i].z;
        faceObj["led1"] = faceList[i].ledAddress[0];
        faceObj["led2"] = faceList[i].ledAddress[1];
        faceObj["numLEDs"] = faceList[i].numLEDs;
        faceObj["brightness"] = faceList[i].ledBrightness;
        faceObj["color"] = (uint32_t)faceList[i].ledColor;
        faceObj["state"] = faceList[i].ledState;
        faceObj["isActive"] = faceList[i].isActive;
    }
    
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD card initialization failed!");
        return false;
    }
    
    // 既存のファイルを削除
    if (SD.exists("/faces.json")) {
        SD.remove("/faces.json");
    }
    
    File file = SD.open("/faces.json", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    
    if (serializeJson(jsonDoc, file) == 0) {
        Serial.println("Failed to write JSON");
        file.close();
        return false;
    }
    
    file.close();
    Serial.println("Saved " + String(calibratedFaces) + " faces to SD card");
    return true;
}

void FaceDetector::resetFaces() {
    calibratedFaces = 0;
    memset(faceList, 0, sizeof(FaceData) * maxFaces);
    
    // SDカードのデータを削除
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    if (SD.exists("/faces.json")) {
        SD.remove("/faces.json");
        Serial.println("Deleted faces.json from SD card");
    } else {
        Serial.println("No face data found on SD card");
    }
}