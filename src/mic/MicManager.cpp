#include "MicManager.h"
#include <M5Unified.h>

MicManager::MicManager() : taskHandle(nullptr), taskRunning(false), micCallback(nullptr) {
}

MicManager::~MicManager() {
    stopTask();
}

void MicManager::startTask(MicCallback callback) {
    micCallback = callback;
    // 既にタスクが動作中であれば何もしない（必要に応じてresumeの実装も検討）
    if (!taskRunning) {
        // タスクを特定のコアにピン留め（例：Core 1）して作成
        xTaskCreatePinnedToCore(
            taskFunction,       // タスク関数
            "MicTask",          // タスク名
            4096,               // スタックサイズ（必要に応じて調整）
            this,               // 引数：thisポインタを渡す
            1,                  // 優先度（適宜調整）
            &taskHandle,        // タスクハンドル
            1                   // ピン留めするコア番号（例：Core 1）
        );
        taskRunning = true;
    }
}

void MicManager::stopTask() {
    if (taskRunning && taskHandle != nullptr) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
        taskRunning = false;
    }
}

void MicManager::taskFunction(void* param) {
    MicManager* instance = static_cast<MicManager*>(param);
    const TickType_t delayTicks = pdMS_TO_TICKS(100); // 100ms間隔

    while (true) {
        // マイクからのサンプル読み取り処理
        int16_t sample[128];
        size_t count = M5.Mic.record(sample, 128, 16000);  // サンプル数とサンプリングレートは環境に合わせて

        // 絶対値の平均を計算（ノイズレベルの概算）
        long sum = 0;
        for (size_t i = 0; i < count; i++) {
            sum += abs(sample[i]);
        }
        int avg = (count > 0) ? (sum / count) : 0;

        // 平均値を0～100の範囲にマッピング
        int soundLevel = map(avg, 0, 2000, 0, 100);
        soundLevel = constrain(soundLevel, 0, 100);

        // コールバック関数が設定されている場合、音量レベルを通知
        if (instance->micCallback != nullptr) {
            instance->micCallback(soundLevel);
        }

        // 100ms待機
        vTaskDelay(delayTicks);
    }
}
