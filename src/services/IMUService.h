#ifndef IMU_SERVICE_H
#define IMU_SERVICE_H

#include "../framework/Service.h"
#include "../framework/Event.h"
#include "../system/FaceDetector.h"
#include <mutex>

namespace framework {

// IMUイベントタイプ
enum class IMUEventType {
    DATA_CHANGED,     // IMUデータが変更された
    ORIENTATION_CHANGED, // 向きが変更された
    STABILITY_CHANGED    // 安定性状態が変更された
};

// IMUイベント
class IMUEvent : public Event {
public:
    IMUEvent(IMUEventType imuEventType, float x = 0.0f, float y = 0.0f, float z = 0.0f, bool isStable = false);
    
    IMUEventType getIMUEventType() const { return m_imuEventType; }
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getZ() const { return m_z; }
    bool isStable() const { return m_isStable; }
    
    std::string toString() const override;

private:
    IMUEventType m_imuEventType;
    float m_x, m_y, m_z;
    bool m_isStable;
};

class IMUService : public Service {
public:
    // シングルトンインスタンス取得
    static IMUService& getInstance();
    
    // シングルトンインスタンス破棄
    static void destroyInstance();
    
    // Service基底クラスのオーバーライド
    bool initialize() override;
    void shutdown() override;
    const char* getServiceTypeName() const override { return "IMUService"; }
    
    // IMUセンサー機能
    void update();
    bool isStable(float threshold = 0.2f, int duration = 1000);
    void resetStableTime();
    void getNormalizedVector(float& x, float& y, float& z);
    float getAccX();
    float getAccY();
    float getAccZ();
    
    // ポーリング間隔設定
    void setUpdateInterval(unsigned long intervalMs);
    unsigned long getUpdateInterval() const { return m_updateIntervalMs; }
    
    // 安定性検出設定
    void setStabilityThreshold(float threshold);
    float getStabilityThreshold() const { return m_stabilityThreshold; }
    
    void setStabilityDuration(int durationMs);
    int getStabilityDuration() const { return m_stabilityDurationMs; }
    
private:
    // シングルトンパターン実装
    IMUService();
    ~IMUService();
    IMUService(const IMUService&) = delete;
    IMUService& operator=(const IMUService&) = delete;
    
    static IMUService* s_instance;
    static std::mutex s_mutex;
    
    IMUSensor* m_imuSensor;
    
    // ポーリング関連
    TaskHandle_t m_updateTaskHandle;
    bool m_isRunning;
    unsigned long m_updateIntervalMs;
    
    // 安定性検出設定
    float m_stabilityThreshold;
    int m_stabilityDurationMs;
    bool m_lastStableState;
    
    // 前回の値
    float m_lastX, m_lastY, m_lastZ;
    
    // ポーリングタスク
    static void updateTaskWrapper(void* parameter);
    void updateTask();
    
    // イベント送信ヘルパーメソッド
    void sendIMUEvent(IMUEventType type, float x, float y, float z, bool isStable);
};

} // namespace framework

#endif // IMU_SERVICE_H
