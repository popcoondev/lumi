#ifndef MIC_MANAGER_H
#define MIC_MANAGER_H
#include <Arduino.h>
#include <array>
#include <functional>

// FFTコールバックの型定義（8帯域の振幅レベルを引数に渡す）
typedef std::function<void(const std::array<double, 8>&, double)> FFTCallback;

class MicManager {
public:
    MicManager();
    ~MicManager();

    // タスクを開始（タスクが既に動作中の場合は再利用）
    void startTask(FFTCallback callback);

    // タスクを停止
    void stopTask();

private:
    // FreeRTOSタスク関数（staticメンバ関数）
    static void taskFunction(void* param);

    // タスクハンドル
    TaskHandle_t taskHandle;

    // タスク動作中かどうかのフラグ
    bool taskRunning;

    // FFT結果更新用のコールバック関数
    FFTCallback fftCallback;
};

#endif // MIC_MANAGER_H
