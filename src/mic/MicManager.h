#ifndef MIC_MANAGER_H
#define MIC_MANAGER_H

#include <Arduino.h>

// コールバック関数の型定義（音量レベルを引数に渡す）
typedef std::function<void(int)> MicCallback;

class MicManager {
public:
    MicManager();
    ~MicManager();

    // タスクを開始（タスクが既に動作中の場合は再利用）
    void startTask(MicCallback callback);

    // タスクを停止
    void stopTask();

private:
    // FreeRTOSタスク関数（staticメンバ関数）
    static void taskFunction(void* param);

    // タスクハンドル
    TaskHandle_t taskHandle;

    // タスク動作中かどうかのフラグ
    bool taskRunning;

    // 音量レベル更新用のコールバック関数
    std::function<void(int)> micCallback;
};

#endif // MIC_MANAGER_H
