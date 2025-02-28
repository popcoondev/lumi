#ifndef LUMI_VIEW_H
#define LUMI_VIEW_H

#include <Arduino.h>
#include <M5Unified.h>
#include <functional>
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
    // ドラッグ中かどうかを外部から確認できるようにする
    bool isBeingDragged() const { return isDragging; }
};

class LumiView {
private:
    OctagonRingView octagon;
    Button settingsButton;
    Button topLeftButton;
    Button bottomLeftButton;
    Button bottomRightButton;
    Slider brightnessSlider; // 明度スライダー
    Slider hueSlider;        // 色相スライダー
    Slider saturationSlider;     // 彩度スライダー
    
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
    
    // ハイライト面の取得・設定
    int getHighlightedFace() { return octagon.getHighlightedFace(); }
    void setHighlightedFace(int faceId) { octagon.setHighlightedFace(faceId); }
    
    // 各種イベントコールバック用関数ポインタ
    std::function<void(int)> onFaceTapped;
    std::function<void()> onCenterTapped;
    std::function<void()> onSettingsButtonTapped;
    std::function<void()> onTopLeftButtonTapped;
    std::function<void()> onBottomLeftButtonTapped;
    std::function<void()> onBottomRightButtonTapped;
    std::function<void(int)> onBrightnessChanged;
    std::function<void(int)> onHueChanged;
    std::function<void(int)> onSaturationChanged;
    
    // フレンドクラス宣言（OctaControllerからoctagonにアクセスできるようにする）
    friend class OctaController;

    bool isAnySliderDragging() const {
        return brightnessSlider.isBeingDragged() || hueSlider.isBeingDragged() || saturationSlider.isBeingDragged();
    }

    void drawSliders();
    void drawBrightnessSlider();
    void drawColorSlider();

    Slider& getBrightnessSlider() { return brightnessSlider; }
    Slider& getColorSlider() { return hueSlider; }

};

#endif // LUMI_VIEW_H
