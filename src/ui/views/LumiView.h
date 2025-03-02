#ifndef LUMI_VIEW_H
#define LUMI_VIEW_H

#include <Arduino.h>
#include <M5Unified.h>
#include <functional>
#include "ui/views/OctagonRingView.h"
#include "ui/components/Button.h"

struct TouchedUI {
    int id;       // UI要素のID
    int data;     // 追加データ（面番号など）
    
    TouchedUI() : id(0), data(-1) {}
    TouchedUI(int id, int data = -1) : id(id), data(data) {}
};


// LumiViewで利用するスライダークラス
class Slider {
private:
    int id;             // スライダーのID
    int x, y, width, height;
    int value;          // 0-100の値
    int knobWidth;      // スライダーのつまみの幅
    int knobHeight;     // スライダーのつまみの高さ
    bool isDragging;    // ドラッグ中フラグ
    uint16_t barColor;  // バーの色
    uint16_t knobColor; // つまみの色
    String title;       // スライダーのタイトル

public:
    Slider(int x, int y, int width, int height);
    void draw();
    void setValue(int val);
    int getValue();
    bool handleTouch(int touchX, int touchY, bool isPressed);
    // ドラッグ中かどうかを外部から確認できるようにする
    bool isBeingDragged() const { return isDragging; }
    void setTitle(String title) { this->title = title; }
    String getTitle() { return title; }
    void setId(int id) { this->id = id; }
    int getId() const { return id; }
    bool containsPoint(int x, int y) const;

};

class LumiView {
private:
    OctagonRingView octagon;
    Button settingsButton;
    Button resetButton;
    Button bottomLeftButton;
    Button bottomRightButton;
    Slider brightnessSlider; // FastLEDでの輝度スライダー
    Slider valueBrightnessSlider; // 明度スライダー
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

    TouchedUI activeTouchedUI;  // 現在タッチ中のUI
    bool checkButtonTouch(Button& button, int touchX, int touchY, bool isPressed, bool wasPressed, bool wasReleased);
    bool checkSliderTouch(Slider& slider, int touchX, int touchY, bool isPressed, bool wasPressed, bool wasReleased);
    uint16_t backgroundColor;   // 背景色の保存用

    // UIコンポーネント用の単純なID定数
    enum {
        ID_NONE = 0,
        ID_BUTTON_RESET,
        ID_BUTTON_SETTINGS,
        ID_BUTTON_BOTTOM_LEFT,
        ID_BUTTON_BOTTOM_RIGHT,
        ID_SLIDER_BRIGHTNESS,
        ID_SLIDER_VALUE_BRIGHTNESS,
        ID_SLIDER_HUE,
        ID_SLIDER_SATURATION,
        ID_OCTAGON_CENTER,
        ID_OCTAGON_FACE_BASE = 100  // 面IDは100+faceIndexとなる
    };

public:
    LumiView();
    void begin();
    void draw();
    
    // タッチイベント処理
    void handleTouch();
    
    // ハイライト面の取得・設定
    int getHighlightedFace() { return octagon.getHighlightedFace(); }
    void setHighlightedFace(int faceId) { octagon.setHighlightedFace(faceId); }
    
    // フォーカス関連のメソッド
    void setFaceFocused(int faceId, bool focused) { octagon.setFaceFocused(faceId, focused); }
    bool isFaceFocused(int faceId) { return octagon.isFaceFocused(faceId); }
    int getFocusedFacesCount() { return octagon.getFocusedFacesCount(); }
    void clearAllFocus() { octagon.clearAllFocus(); }
    int getFocusedFacesLedState() { return octagon.getFocusedFacesLedState(); }
    
    // センターボタン情報表示
    void updateCenterButtonInfo();
    void drawCenterButtonInfo(const String& text, uint16_t color);
    
    // 円形プログレスバー
    enum ProgressMode {
        PROGRESS_MODE_NONE,
        PROGRESS_MODE_BRIGHTNESS,
        PROGRESS_MODE_HUE,
        PROGRESS_MODE_SATURATION,
        PROGRESS_MODE_PATTERN
    };
    
    void drawCircularProgress(int value, ProgressMode mode);
    void updateCircularProgressAnimation();
    
    ProgressMode currentProgressMode;
    int progressValue;
    unsigned long lastProgressUpdateTime;
    int progressAnimationFrame;
    
    // 各種イベントコールバック用関数ポインタ
    std::function<void(int)> onFaceTapped;
    std::function<void()> onCenterTapped;
    std::function<void()> onTopRightButtonTapped;
    std::function<void()> onTopLeftButtonTapped;
    std::function<void()> onBottomLeftButtonTapped;
    std::function<void()> onBottomRightButtonTapped;
    std::function<void(int)> onBrightnessChanged;
    std::function<void(int)> onHueChanged;
    std::function<void(int)> onSaturationChanged;
    std::function<void(int)> onValueBrightnessChanged;
    
    // ドラッグ選択用の変数
    bool isDragging;
    int dragStartFace;
    int lastDraggedFace;
    
    // パフォーマンス最適化用の変数
    bool needsProgressUpdate;
    
    // フレンドクラス宣言（OctaControllerからoctagonにアクセスできるようにする）
    friend class OctaController;

    bool isAnySliderDragging() const {
        return brightnessSlider.isBeingDragged() 
        || valueBrightnessSlider.isBeingDragged()
        || hueSlider.isBeingDragged() 
        || saturationSlider.isBeingDragged();
    }

    void drawSliders();
    void drawBrightnessSlider();
    void drawValueBrightnessSlider();
    void drawHueSlider();
    void drawSaturationSlider();

    Slider& getBrightnessSlider() { return brightnessSlider; }
    Slider& getHueSlider() { return hueSlider; }
    Slider& getSaturationSlider() { return saturationSlider; }
    Slider& getValueBrightnessSlider() { return valueBrightnessSlider; }

};

#endif // LUMI_VIEW_H
