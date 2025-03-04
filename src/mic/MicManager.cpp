#include "MicManager.h"
#include <M5Unified.h>
#include <arduinoFFT.h>
#include <array>

// FFT設定
const uint16_t FFT_SAMPLES = 128;
const uint8_t FFT_HAMMING = 1;
const double SAMPLING_FREQUENCY = 16000.0;
ArduinoFFT<double> FFT;

// 各帯域の周波数範囲（例：Lumi の各面に対応する帯域）
struct FrequencyBand {
  double lowFreq;
  double highFreq;
};

FrequencyBand bands[8] = {
  {16, 64},     // 面1：超低音（バスドラム）
  {65, 125},    // 面2：低音（ベース）
  {126, 250},   // 面3：中低音（ギター、男性声）
  {251, 500},   // 面4：中音（ピアノ）
  {501, 1000},  // 面5：中高音（ボーカル）
  {1001, 2000}, // 面6：高音（ハイハット、女性声）
  {2001, 4000}, // 面7：超高音（シンバル）
  {4001, 8000}  // 面8：金属音、ノイズ
};

// BPM検出用の自己相関関数
static double detectBPM(const std::vector<double>& env, double frameRate) {
    size_t n = env.size();
    std::vector<double> autocorr(n, 0.0);
    for (size_t lag = 0; lag < n; lag++) {
        double sum = 0.0;
        for (size_t i = 0; i < n - lag; i++) {
            sum += env[i] * env[i + lag];
        }
        autocorr[lag] = sum;
    }
    // BPM の一般的な範囲は 60〜180 BPM → 周期 1.0〜0.33秒
    int minLag = static_cast<int>(std::round(frameRate * 0.33));
    int maxLag = static_cast<int>(std::round(frameRate * 1.0));
    int bestLag = minLag;
    double bestVal = 0.0;
    for (int lag = minLag; lag <= maxLag && lag < (int)n; lag++) {
        if (autocorr[lag] > bestVal) {
            bestVal = autocorr[lag];
            bestLag = lag;
        }
    }
    double beatPeriodSec = bestLag / frameRate;
    double bpm = 60.0 / beatPeriodSec;

    // debug print
    Serial.print("Autocorr: ");
    for (int i = 0; i < 10; i++) {
        Serial.print(autocorr[i]);
        Serial.print(" ");
    }
    Serial.println();
    Serial.print("Best lag: ");
    Serial.println(bestLag);
    Serial.print("BPM: ");
    Serial.println(bpm);
    
    return bpm;
}

MicManager::MicManager() : taskHandle(nullptr), taskRunning(false), fftCallback(nullptr) {
    FFT = ArduinoFFT<double>(nullptr, nullptr, FFT_SAMPLES, SAMPLING_FREQUENCY);
    M5.Mic.begin();
}

MicManager::~MicManager() {
    stopTask();
    M5.Mic.end();
}

void MicManager::startTask(FFTCallback callback) {
    fftCallback = callback;
    if (!taskRunning) {
        xTaskCreatePinnedToCore(
            taskFunction,       // タスク関数
            "MicTask",          // タスク名
            4096,               // スタックサイズ（必要に応じて調整）
            this,               // 引数として this ポインタを渡す
            1,                  // 優先度
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
    const TickType_t delayTicks = pdMS_TO_TICKS(100); // 各処理後の待機時間

    // FFT 用の作業領域
    int16_t sampleBuffer[FFT_SAMPLES];
    double vReal[FFT_SAMPLES];
    double vImag[FFT_SAMPLES];

    // エンベロープ蓄積用（各ブロックの平均絶対値）
    static std::vector<double> envelopeData;
    envelopeData.reserve(300);  // 十分な容量を確保

    // ブロックごとの時間（秒）
    const double blockDuration = static_cast<double>(FFT_SAMPLES) / SAMPLING_FREQUENCY;  // 例: 128/16000 = 0.008 sec
    const double envelopeFrameRate = 1.0 / blockDuration; // 約125 fps

    while (true) {
        Serial.println("Mic task running...");
        
        // 128サンプルを蓄積する
        int samplesCollected = 0;
        while (samplesCollected < FFT_SAMPLES) {
            size_t count = M5.Mic.record(&sampleBuffer[samplesCollected], FFT_SAMPLES - samplesCollected, SAMPLING_FREQUENCY);
            if (count > 0) {
                samplesCollected += count;
            }
            vTaskDelay(pdMS_TO_TICKS(1));  // 少し待機
        }
        
        // エンベロープ値の計算（平均絶対値）
        double blockEnvelope = 0.0;
        for (int i = 0; i < FFT_SAMPLES; i++) {
            blockEnvelope += std::abs(sampleBuffer[i]);
        }
        blockEnvelope /= FFT_SAMPLES;
        envelopeData.push_back(blockEnvelope);

        // ある程度データが貯まったら BPM 検出（例: 250ブロック分 ≒2秒分）
        double bpm = 0.0;
        const size_t envelopeThreshold = 250;
        Serial.println("Envelope data size: " + String(envelopeData.size()));
        if (envelopeData.size() >= envelopeThreshold) {
            bpm = detectBPM(envelopeData, envelopeFrameRate);
            Serial.println("Detected BPM: ");
            Serial.println(bpm);
            envelopeData.pop_back();  // 最古のデータを削除
        }

        Serial.println("FFT processing...");

        // サンプルを double 配列に変換、虚部は 0 に設定
        for (int i = 0; i < FFT_SAMPLES; i++) {
            vReal[i] = static_cast<double>(sampleBuffer[i]);
            vImag[i] = 0.0;
        }

        // ローカルに FFT インスタンスを作成
        ArduinoFFT<double> localFFT(vReal, vImag, FFT_SAMPLES, SAMPLING_FREQUENCY);

        // ウィンドウ関数適用（Hamming）、FFT計算、振幅変換
        localFFT.windowing(vReal, FFT_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD, vReal, true);
        localFFT.compute(vReal, vImag, FFT_SAMPLES, FFT_FORWARD);
        localFFT.complexToMagnitude(vReal, vImag, FFT_SAMPLES);

        // 各帯域の平均振幅を算出
        double bandLevels[8] = {0.0};
        double binFreq = SAMPLING_FREQUENCY / static_cast<double>(FFT_SAMPLES);
        for (int band = 0; band < 8; band++) {
            double sum = 0.0;
            int countBins = 0;
            // DC成分 (i=0) を除外して解析
            for (int i = 1; i < FFT_SAMPLES / 2; i++) {
                double freq = i * binFreq;
                if (freq >= bands[band].lowFreq && freq <= bands[band].highFreq) {
                    sum += vReal[i];
                    countBins++;
                }
            }
            bandLevels[band] = (countBins > 0) ? sum / countBins : 0.0;
        }

        // 結果を std::array に変換して、bandLevels と bpm をコールバックで返却
        if (instance->fftCallback) {
            std::array<double, 8> levels;
            for (int i = 0; i < 8; i++) {
                levels[i] = bandLevels[i];
            }
            instance->fftCallback(levels, bpm);
        }

        vTaskDelay(delayTicks);
    }
}