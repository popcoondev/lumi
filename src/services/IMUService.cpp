#include "IMUService.h"
#include "../framework/EventBus.h"

namespace framework {

// 静的メンバ初期化
IMUService* IMUService::s_instance = nullptr;
std::mutex IMUService::s_mutex;

// IMUEvent実装
IMUEvent::IMUEvent(IMUEventType imuEventType, float x, float y, float z, bool isStable)
    : Event(EventType::CUSTOM), m_imuEventType(imuEventType), 
      m_x(x), m_y(y), m_z(z), m_isStable(isStable) {}

std::string IMUEvent::toString() const {
    std::string typeStr;
    switch (m_imuEventType) {
        case IMUEventType::DATA_CHANGED: typeStr = "DATA_CHANGED"; break;
        case IMUEventType::ORIENTATION_CHANGED: typeStr = "ORIENTATION_CHANGED"; break;
        case IMUEventType::STABILITY_CHANGED: typeStr = "STABILITY_CHANGED"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    
    return "IMUEvent{type=" + typeStr + 
           ", x=" + std::to_string(m_x) + 
           ", y=" + std::to_string(m_y) + 
           ", z=" + std::to_string(m_z) + 
           ", stable=" + (m_isStable ? "true" : "false") + 
           ", " + Event::toString() + "}";
}

// シングルトンインスタンス取得
IMUService& IMUService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = new IMUService();
    }
    return *s_instance;
}

// シングルトンインスタンス破棄
void IMUService::destroyInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

// コンストラクタ
IMUService::IMUService() 
    : Service(), m_imuSensor(nullptr), m_isRunning(false), 
      m_updateTaskHandle(nullptr), m_updateIntervalMs(50),
      m_stabilityThreshold(0.2f), m_stabilityDurationMs(1000),
      m_lastStableState(false), m_lastX(0.0f), m_lastY(0.0f), m_lastZ(0.0f) {
    // 初期化はinitialize()で行う
}

// デストラクタ
IMUService::~IMUService() {
    shutdown();
    if (m_imuSensor) {
        delete m_imuSensor;
        m_imuSensor = nullptr;
    }
}

// 初期化
bool IMUService::initialize() {
    if (!Service::initialize()) {
        return false;
    }
    
    // IMUセンサーの作成と初期化
    m_imuSensor = new IMUSensor();
    m_imuSensor->begin();
    
    // 更新タスクの作成
    m_isRunning = true;
    xTaskCreatePinnedToCore(
        updateTaskWrapper,
        "IMUServiceTask",
        4096,
        this,
        1,
        &m_updateTaskHandle,
        1
    );
    
    return true;
}

// シャットダウン
void IMUService::shutdown() {
    if (isInitialized()) {
        // 更新タスクの停止
        m_isRunning = false;
        if (m_updateTaskHandle != nullptr) {
            vTaskDelete(m_updateTaskHandle);
            m_updateTaskHandle = nullptr;
        }
        
        Service::shutdown();
    }
}

// 更新タスクラッパー
void IMUService::updateTaskWrapper(void* parameter) {
    IMUService* service = static_cast<IMUService*>(parameter);
    service->updateTask();
    vTaskDelete(NULL);
}

// 更新タスク
void IMUService::updateTask() {
    while (m_isRunning) {
        update();
        vTaskDelay(pdMS_TO_TICKS(m_updateIntervalMs));
    }
}

// IMUセンサー更新
void IMUService::update() {
    if (!m_imuSensor) return;
    
    // センサーデータ更新
    m_imuSensor->update();
    
    // 現在の値を取得
    float x = m_imuSensor->getAccX();
    float y = m_imuSensor->getAccY();
    float z = m_imuSensor->getAccZ();
    
    // 安定性チェック
    bool isStable = m_imuSensor->isStable(m_stabilityThreshold, m_stabilityDurationMs);
    
    // 値が変化した場合にイベント送信
    bool dataChanged = (x != m_lastX || y != m_lastY || z != m_lastZ);
    if (dataChanged) {
        sendIMUEvent(IMUEventType::DATA_CHANGED, x, y, z, isStable);
        
        // 向きの変化が大きい場合は向き変更イベントも送信
        float diffMagnitude = sqrt(pow(x - m_lastX, 2) + pow(y - m_lastY, 2) + pow(z - m_lastZ, 2));
        if (diffMagnitude > m_stabilityThreshold * 2) {
            sendIMUEvent(IMUEventType::ORIENTATION_CHANGED, x, y, z, isStable);
        }
    }
    
    // 安定性状態が変化した場合にイベント送信
    if (isStable != m_lastStableState) {
        sendIMUEvent(IMUEventType::STABILITY_CHANGED, x, y, z, isStable);
        m_lastStableState = isStable;
    }
    
    // 前回の値を更新
    m_lastX = x;
    m_lastY = y;
    m_lastZ = z;
}

// IMUセンサー機能のラッパー
bool IMUService::isStable(float threshold, int duration) {
    if (!m_imuSensor) return false;
    return m_imuSensor->isStable(threshold, duration);
}

void IMUService::resetStableTime() {
    if (!m_imuSensor) return;
    m_imuSensor->resetStableTime();
}

void IMUService::getNormalizedVector(float& x, float& y, float& z) {
    if (!m_imuSensor) {
        x = y = z = 0.0f;
        return;
    }
    m_imuSensor->getNormalizedVector(x, y, z);
}

float IMUService::getAccX() {
    if (!m_imuSensor) return 0.0f;
    return m_imuSensor->getAccX();
}

float IMUService::getAccY() {
    if (!m_imuSensor) return 0.0f;
    return m_imuSensor->getAccY();
}

float IMUService::getAccZ() {
    if (!m_imuSensor) return 0.0f;
    return m_imuSensor->getAccZ();
}

// 設定メソッド
void IMUService::setUpdateInterval(unsigned long intervalMs) {
    m_updateIntervalMs = intervalMs;
}

void IMUService::setStabilityThreshold(float threshold) {
    m_stabilityThreshold = threshold;
}

void IMUService::setStabilityDuration(int durationMs) {
    m_stabilityDurationMs = durationMs;
}

// イベント送信ヘルパーメソッド
void IMUService::sendIMUEvent(IMUEventType type, float x, float y, float z, bool isStable) {
    IMUEvent event(type, x, y, z, isStable);
    EventBus::getInstance().postEvent(event);
}

} // namespace framework
