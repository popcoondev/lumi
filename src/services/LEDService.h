#ifndef LED_SERVICE_H
#define LED_SERVICE_H

#include "../framework/Service.h"
#include "../led/LEDManager.h"
#include "../framework/Event.h"
#include <FastLED.h>
#include <queue>
#include <mutex>

namespace framework {

// LED操作の種類を定義
enum class LEDOperationType {
    FACE_COLOR,      // 特定の面の色を変更
    ALL_FACES_COLOR, // すべての面の色を変更
    PATTERN_START,   // パターン開始
    PATTERN_STOP,    // パターン停止
    BRIGHTNESS,      // 明るさ変更
    RESET            // すべてリセット
};

// LED操作を表す構造体
struct LEDOperation {
    LEDOperationType type;
    int faceId;
    CRGB color;
    int patternIndex;
    uint8_t brightness;
    int delayMs;
    
    // コンストラクタ
    LEDOperation(
        LEDOperationType type = LEDOperationType::RESET,
        int faceId = -1,
        CRGB color = CRGB::Black,
        int patternIndex = -1,
        uint8_t brightness = 255,
        int delayMs = 0
    ) : type(type), faceId(faceId), color(color), 
        patternIndex(patternIndex), brightness(brightness),
        delayMs(delayMs) {}
};

// LED状態変更イベント用のイベントタイプ拡張
enum class LEDEventType {
    FACE_COLOR_CHANGED,  // 面の色が変更された
    PATTERN_STARTED,     // パターン再生開始
    PATTERN_STOPPED,     // パターン再生停止
    BRIGHTNESS_CHANGED   // 明るさ変更
};

// LED状態変更イベント
class LEDEvent : public Event {
public:
    LEDEvent(LEDEventType ledEventType, int faceId = -1, CRGB color = CRGB::Black, int patternIndex = -1);
    
    LEDEventType getLEDEventType() const { return m_ledEventType; }
    int getFaceId() const { return m_faceId; }
    CRGB getColor() const { return m_color; }
    int getPatternIndex() const { return m_patternIndex; }
    
    std::string toString() const override;

private:
    LEDEventType m_ledEventType;
    int m_faceId;
    CRGB m_color;
    int m_patternIndex;
};

class LEDService : public Service {
public:
    // シングルトンインスタンス取得
    static LEDService& getInstance();
    
    // シングルトンインスタンス破棄
    static void destroyInstance();
    
    // Service基底クラスのオーバーライド
    bool initialize() override;
    void shutdown() override;
    const char* getServiceTypeName() const override { return "LEDService"; }
    
    // LEDManager機能のラッパー（操作をキューに追加）
    void begin(int pin, int numLeds, int ledOffset);
    void runPattern(int patternIndex);
    void stopPattern();
    void lightFace(int faceId, CRGB color);
    CRGB getFaceColor(int faceId);
    void resetAllLeds();
    int getPatternCount();
    void nextPattern();
    void prevPattern();
    String getPatternName(int index);
    String getCurrentPatternName();
    int getCurrentPatternIndex();
    void setBrightness(uint8_t brightness);
    bool isPatternRunning();
    
    // 追加機能
    void lightAllFaces(CRGB color);
    void lightFacesSequential(CRGB color, int delayMs = 50);
    
private:
    // シングルトンパターン実装
    LEDService();
    ~LEDService();
    LEDService(const LEDService&) = delete;
    LEDService& operator=(const LEDService&) = delete;
    
    static LEDService* s_instance;
    static std::mutex s_mutex;
    
    LEDManager m_ledManager;
    
    // 操作キュー関連
    std::queue<LEDOperation> m_operationQueue;
    std::mutex m_queueMutex;
    TaskHandle_t m_processingTaskHandle;
    bool m_isProcessing;
    
    // 操作キュー処理タスク
    static void processingTaskWrapper(void* parameter);
    void processOperationQueue();
    
    // 操作をキューに追加
    void enqueueOperation(const LEDOperation& operation);
    
    // イベント送信ヘルパーメソッド
    void sendLEDEvent(LEDEventType type, int faceId = -1, CRGB color = CRGB::Black, int patternIndex = -1);
};

} // namespace framework

#endif // LED_SERVICE_H
