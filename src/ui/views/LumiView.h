#ifndef LUMI_VIEW_H
#define LUMI_VIEW_H

#include <Arduino.h>
#include <M5Unified.h>
#include "ui/views/OctagonRingView.h"
#include "ui/components/Button.h"

// LumiViewで利用するスライダークラス
class Slider {
private:
    int x, y, width, height;
    int value;          // 0-100の値
    int knobWidth;      // スライダーのつまみの幅
    int knobHeight;     // スライダーのつまみの高さ
    bool isDragging;    // ドラッグ中フラグ
    uint16_t barColor;  // バーの色
    uint16_t knobColor; // つまみの色

public:
    Slider(int x, int y, int width, int height);
    void draw();
    void setValue(int val);
    int getValue();
    bool handleTouch(int touchX, int touchY, bool isPressed);
};

class LumiView {
private:
    OctagonRingView octagon;
    Button settingsButton;
    Button topLeftButton;
    Button bottomLeftButton;
    Button bottomRightButton;
    Slider brightnessSlider; // 左側のスライダー
    Slider colorSlider;      // 右側のスライダー
    
    // タッチ検出用の領域
    struct {
        int centerX, centerY;
        int radius;
    } octagonCenter;
    
    bool isTouchActive;
    int lastTouchX, lastTouchY;
    
    // オクタゴンの面がタップされたか判定
    int getTappedFace(int x, int y);
    
    // オクタゴンの中心がタップされたか判定
    bool isCenterTapped(int x, int y);

public:
    LumiView();
    void begin();
    void draw();
    
    // タッチイベント処理
    void handleTouch();
    
    // 各種イベントコールバック
    void onFaceTapped(int faceId);
    void onCenterTapped();
    void onSettingsButtonTapped();
    void onTopLeftButtonTapped();
    void onBottomLeftButtonTapped();
    void onBottomRightButtonTapped();
    void onBrightnessChanged(int value);
    void onColorChanged(int value);
};

#endif // LUMI_VIEW_H