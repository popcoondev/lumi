#include "LEDService.h"
#include "../framework/EventBus.h"
#include "../core/Constants.h"

namespace framework {

// 静的メンバ初期化
LEDService* LEDService::s_instance = nullptr;
std::mutex LEDService::s_mutex;

// LEDEvent実装
LEDEvent::LEDEvent(LEDEventType ledEventType, int faceId, CRGB color, int patternIndex)
    : Event(EventType::CUSTOM), m_ledEventType(ledEventType), m_faceId(faceId), 
      m_color(color), m_patternIndex(patternIndex) {}

std::string LEDEvent::toString() const {
    std::string typeStr;
    switch (m_ledEventType) {
        case LEDEventType::FACE_COLOR_CHANGED: typeStr = "FACE_COLOR_CHANGED"; break;
        case LEDEventType::PATTERN_STARTED: typeStr = "PATTERN_STARTED"; break;
        case LEDEventType::PATTERN_STOPPED: typeStr = "PATTERN_STOPPED"; break;
        case LEDEventType::BRIGHTNESS_CHANGED: typeStr = "BRIGHTNESS_CHANGED"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    
    return "LEDEvent{type=" + typeStr + 
           ", faceId=" + std::to_string(m_faceId) + 
           ", color=(" + std::to_string(m_color.r) + "," + 
                         std::to_string(m_color.g) + "," + 
                         std::to_string(m_color.b) + ")" +
           ", patternIndex=" + std::to_string(m_patternIndex) + 
           ", " + Event::toString() + "}";
}

// シングルトンインスタンス取得
LEDService& LEDService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = new LEDService();
    }
    return *s_instance;
}

// シングルトンインスタンス破棄
void LEDService::destroyInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

// コンストラクタ
LEDService::LEDService() 
    : Service(), m_isProcessing(false), m_processingTaskHandle(nullptr) {
    // 初期化はinitialize()で行う
}

// デストラクタ
LEDService::~LEDService() {
    shutdown();
}

// 初期化
bool LEDService::initialize() {
    if (!Service::initialize()) {
        return false;
    }
    
    // 処理タスクの作成
    m_isProcessing = true;
    xTaskCreatePinnedToCore(
        processingTaskWrapper,
        "LEDServiceTask",
        4096,
        this,
        1,
        &m_processingTaskHandle,
        1
    );
    
    return true;
}

// シャットダウン
void LEDService::shutdown() {
    if (isInitialized()) {
        // 処理タスクの停止
        m_isProcessing = false;
        if (m_processingTaskHandle != nullptr) {
            vTaskDelete(m_processingTaskHandle);
            m_processingTaskHandle = nullptr;
        }
        
        // キューのクリア
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            while (!m_operationQueue.empty()) {
                m_operationQueue.pop();
            }
        }
        
        Service::shutdown();
    }
}

// 処理タスクラッパー
void LEDService::processingTaskWrapper(void* parameter) {
    LEDService* service = static_cast<LEDService*>(parameter);
    service->processOperationQueue();
    vTaskDelete(NULL);
}

// 操作キュー処理
void LEDService::processOperationQueue() {
    while (m_isProcessing) {
        LEDOperation operation;
        bool hasOperation = false;
        
        // キューから操作を取得
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (!m_operationQueue.empty()) {
                operation = m_operationQueue.front();
                m_operationQueue.pop();
                hasOperation = true;
            }
        }
        
        // 操作を実行
        if (hasOperation) {
            switch (operation.type) {
                case LEDOperationType::FACE_COLOR:
                    m_ledManager.lightFace(operation.faceId, operation.color);
                    sendLEDEvent(LEDEventType::FACE_COLOR_CHANGED, operation.faceId, operation.color);
                    break;
                    
                case LEDOperationType::ALL_FACES_COLOR:
                    for (int i = 0; i < MAX_FACES; i++) {
                        m_ledManager.lightFace(i, operation.color);
                        sendLEDEvent(LEDEventType::FACE_COLOR_CHANGED, i, operation.color);
                        if (operation.delayMs > 0) {
                            delay(operation.delayMs);
                        }
                    }
                    break;
                    
                case LEDOperationType::PATTERN_START:
                    m_ledManager.runPattern(operation.patternIndex);
                    sendLEDEvent(LEDEventType::PATTERN_STARTED, -1, CRGB::Black, operation.patternIndex);
                    break;
                    
                case LEDOperationType::PATTERN_STOP:
                    m_ledManager.stopPattern();
                    sendLEDEvent(LEDEventType::PATTERN_STOPPED);
                    break;
                    
                case LEDOperationType::BRIGHTNESS:
                    m_ledManager.setBrightness(operation.brightness);
                    sendLEDEvent(LEDEventType::BRIGHTNESS_CHANGED);
                    break;
                    
                case LEDOperationType::RESET:
                    m_ledManager.resetAllLeds();
                    for (int i = 0; i < MAX_FACES; i++) {
                        sendLEDEvent(LEDEventType::FACE_COLOR_CHANGED, i, CRGB::Black);
                    }
                    break;
            }
        }
        
        // 少し待機
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// 操作をキューに追加
void LEDService::enqueueOperation(const LEDOperation& operation) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_operationQueue.push(operation);
}

// LEDManagerのラッパーメソッド
void LEDService::begin(int pin, int numLeds, int ledOffset) {
    m_ledManager.begin(pin, numLeds, ledOffset);
}

void LEDService::runPattern(int patternIndex) {
    enqueueOperation(LEDOperation(LEDOperationType::PATTERN_START, -1, CRGB::Black, patternIndex));
}

void LEDService::stopPattern() {
    enqueueOperation(LEDOperation(LEDOperationType::PATTERN_STOP));
}

void LEDService::lightFace(int faceId, CRGB color) {
    enqueueOperation(LEDOperation(LEDOperationType::FACE_COLOR, faceId, color));
}

CRGB LEDService::getFaceColor(int faceId) {
    return m_ledManager.getFaceColor(faceId);
}

void LEDService::resetAllLeds() {
    enqueueOperation(LEDOperation(LEDOperationType::RESET));
}

int LEDService::getPatternCount() {
    return m_ledManager.getPatternCount();
}

void LEDService::nextPattern() {
    int nextIndex = (m_ledManager.getCurrentPatternIndex() + 1) % m_ledManager.getPatternCount();
    enqueueOperation(LEDOperation(LEDOperationType::PATTERN_START, -1, CRGB::Black, nextIndex));
}

void LEDService::prevPattern() {
    int prevIndex = (m_ledManager.getCurrentPatternIndex() - 1 + m_ledManager.getPatternCount()) % m_ledManager.getPatternCount();
    enqueueOperation(LEDOperation(LEDOperationType::PATTERN_START, -1, CRGB::Black, prevIndex));
}

String LEDService::getPatternName(int index) {
    return m_ledManager.getPatternName(index);
}

String LEDService::getCurrentPatternName() {
    return m_ledManager.getCurrentPatternName();
}

int LEDService::getCurrentPatternIndex() {
    return m_ledManager.getCurrentPatternIndex();
}

void LEDService::setBrightness(uint8_t brightness) {
    enqueueOperation(LEDOperation(LEDOperationType::BRIGHTNESS, -1, CRGB::Black, -1, brightness));
}

bool LEDService::isPatternRunning() {
    return m_ledManager.isPatternRunning();
}

// 追加機能
void LEDService::lightAllFaces(CRGB color) {
    enqueueOperation(LEDOperation(LEDOperationType::ALL_FACES_COLOR, -1, color, -1, 255, 0));
}

void LEDService::lightFacesSequential(CRGB color, int delayMs) {
    enqueueOperation(LEDOperation(LEDOperationType::ALL_FACES_COLOR, -1, color, -1, 255, delayMs));
}

// イベント送信ヘルパーメソッド
void LEDService::sendLEDEvent(LEDEventType type, int faceId, CRGB color, int patternIndex) {
    LEDEvent event(type, faceId, color, patternIndex);
    EventBus::getInstance().postEvent(event);
}

} // namespace framework
