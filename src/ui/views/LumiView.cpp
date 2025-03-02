#include "ui/views/LumiView.h"
#include "core/Constants.h"

#define CORNER_BUTTON_WIDTH 70
#define CORNER_BUTTON_HEIGHT 40

// Sliderクラスの実装
Slider::Slider(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height), 
      value(50), isDragging(false), 
      knobWidth(8), knobHeight(8),
      barColor(BLACK), knobColor(WHITE)
{
}

void Slider::draw() {
    int barPadding = 10;
    // バー背景
    M5.Lcd.fillRect(x, y, width, height, barColor);

    // バーの可動域を示すライン
    M5.Lcd.drawLine(x + width / 2, y + barPadding, x + width / 2, y + height - barPadding, TFT_WHITE);
    
    // つまみの位置を計算
    int knobY = y + (height - knobHeight) * (100 - value) / 100;
    
    // つまみを描画
    M5.Lcd.fillRoundRect(x + (width - knobWidth) / 2, knobY, knobWidth, knobHeight, 2, knobColor);

    // Viewの左上にタイトルを表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(x+2, y+2);
    M5.Lcd.print(title);

    // 値の表示
    // M5.Lcd.setTextSize(1);
    // M5.Lcd.setTextColor(TFT_WHITE);
    // M5.Lcd.setTextDatum(MC_DATUM);
    // M5.Lcd.drawNumber(value, x + width / 2, knobY + knobHeight / 2);
}

void Slider::setValue(int val) {
    value = constrain(val, 0, 100);
}

int Slider::getValue() {
    return value;
}

bool Slider::handleTouch(int touchX, int touchY, bool isPressed) {
    // スライダー領域内のタッチかチェック
    bool isTouchInSlider = (touchX >= x && touchX < x + width && 
                           touchY >= y && touchY < y + height);
    
    if (isPressed) {
        if (isTouchInSlider || isDragging) {
            isDragging = true;
            
            // Y位置から値を計算（上が100、下が0）
            int newValue = 100 - constrain((touchY - y) * 100 / height, 0, 100);
            setValue(newValue);
            return true;
        }
    } else {
        isDragging = false;
    }
    
    return false;
}

// LumiViewクラスの実装
LumiView::LumiView()
    : topLeftButton(0, 0, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Reset"),
      bottomLeftButton(0, 240 - CORNER_BUTTON_HEIGHT, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Single"),
      settingsButton(320 - CORNER_BUTTON_WIDTH, 0, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Settings"),
      bottomRightButton(320 - CORNER_BUTTON_WIDTH, 240 - CORNER_BUTTON_HEIGHT, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Patterns"),
      brightnessSlider(0, 40, 40, 80),
      valueBrightnessSlider(0, 120, 40, 80),
      hueSlider(320 - 40, 40, 40, 80),
      saturationSlider(320 - 40, 120, 40, 80),
      isTouchActive(false),
      lastTouchX(0),
      lastTouchY(0)
{
    // オクタゴンの中心タッチ検出用の設定
    octagonCenter.centerX = 160;
    octagonCenter.centerY = 120;
    octagonCenter.radius = 30;
}

void LumiView::begin() {
    // オクタゴンビューの設定
    octagon.setViewPosition(40, 0, 240, 240);
    octagon.setMirrored(true);
    // octagon.rotate(0.4); // π/8ラジアン（22.5度）
    octagon.rotate(PI); // πラジアン（180度）LEDの向きを合わせるため
    
    // ボタンのスタイル設定
    settingsButton.setColor(BLACK, TFT_LIGHTGREY);
    settingsButton.setFontSize(1.4);
    settingsButton.setType(BUTTON_TYPE_TEXT);
    
    topLeftButton.setColor(BLACK, TFT_LIGHTGREY);
    topLeftButton.setFontSize(1.4);
    topLeftButton.setType(BUTTON_TYPE_TEXT);

    bottomLeftButton.setColor(BLACK, TFT_LIGHTGREY);
    bottomLeftButton.setFontSize(1.4);
    bottomLeftButton.setType(BUTTON_TYPE_TEXT);

    bottomRightButton.setColor(BLACK, TFT_LIGHTGREY);
    bottomRightButton.setFontSize(1.4);
    bottomRightButton.setType(BUTTON_TYPE_TEXT);

    brightnessSlider.setTitle("B");
    valueBrightnessSlider.setTitle("V");
    hueSlider.setTitle("H");
    saturationSlider.setTitle("S");
}

void LumiView::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // オクタゴンを描画
    octagon.draw();
    
    // 四隅のボタンを描画
    settingsButton.draw();
    topLeftButton.draw();
    bottomLeftButton.draw();
    bottomRightButton.draw();
    
    // スライダーを描画
    brightnessSlider.draw();
    valueBrightnessSlider.draw();
    hueSlider.draw();
    saturationSlider.draw();
}

void LumiView::handleTouch() {
    // M5.Touch更新はOctaControllerで行われていることを前提
    auto touch = M5.Touch.getDetail();
    bool isPressed = (touch.isPressed() || touch.wasPressed());
    
    // タッチ座標取得
    int touchX = touch.x;
    int touchY = touch.y;
    
    // タッチ状態の更新
    isTouchActive = isPressed;
    if (isPressed) {
        lastTouchX = touchX;
        lastTouchY = touchY;
    }
    
    // ボタンのタッチ判定
    if (touch.wasPressed()) {
        if (settingsButton.isPressed()) {
            if (onSettingsButtonTapped) onSettingsButtonTapped();
            return;
        }
        
        if (topLeftButton.isPressed()) {
            if (onTopLeftButtonTapped) onTopLeftButtonTapped();
            return;
        }
        
        if (bottomLeftButton.isPressed()) {
            if (onBottomLeftButtonTapped) onBottomLeftButtonTapped();
            return;
        }
        
        if (bottomRightButton.isPressed()) {
            if (onBottomRightButtonTapped) onBottomRightButtonTapped();
            return;
        }
        
        // オクタゴンの中心タップ判定
        if (isCenterTapped(touchX, touchY)) {
            if (onCenterTapped) onCenterTapped();
            return;
        }
        
        // オクタゴンの面タップ判定
        int faceId = getTappedFace(touchX, touchY);
        if (faceId >= 0) {
            if (onFaceTapped) onFaceTapped(faceId);
            return;
        }
    }
    
    // スライダーのタッチ処理を最適化
    // 明るさスライダーの処理
    bool brightnessChanged = brightnessSlider.handleTouch(touchX, touchY, isPressed);
    if (brightnessChanged || brightnessSlider.isBeingDragged()) {
        // 操作中または値に変化があった場合のみスライダーを再描画
        brightnessSlider.draw();
        
        // 値に変化があった場合のみコールバック呼び出し
        if (brightnessChanged && onBrightnessChanged) {
            onBrightnessChanged(brightnessSlider.getValue());
        }
        return;
    }
    
    // 明度スライダーの処理
    bool valueBrightnessChanged = valueBrightnessSlider.handleTouch(touchX, touchY, isPressed);
    if (valueBrightnessChanged || valueBrightnessSlider.isBeingDragged()) {
        // 操作中または値に変化があった場合のみスライダーを再描画
        valueBrightnessSlider.draw();
        
        // 値に変化があった場合のみコールバック呼び出し
        if (valueBrightnessChanged && onValueBrightnessChanged) {
            onValueBrightnessChanged(valueBrightnessSlider.getValue());
        }
        return;
    }

    // 色相スライダーの処理
    bool hueChanged = hueSlider.handleTouch(touchX, touchY, isPressed);
    if (hueChanged || hueSlider.isBeingDragged()) {
        // 操作中または値に変化があった場合のみスライダーを再描画
        hueSlider.draw();
        
        // 値に変化があった場合のみコールバック呼び出し
        if (hueChanged && onHueChanged) {
            onHueChanged(hueSlider.getValue());
        }
        return;
    }

    // 彩度スライダーの処理
    bool saturationChanged = saturationSlider.handleTouch(touchX, touchY, isPressed);
    if (saturationChanged || saturationSlider.isBeingDragged()) {
        // 操作中または値に変化があった場合のみスライダーを再描画
        saturationSlider.draw();
        
        // 値に変化があった場合のみコールバック呼び出し
        if (saturationChanged && onSaturationChanged) {
            onSaturationChanged(saturationSlider.getValue());
        }
        return;
    }
}

int LumiView::getTappedFace(int x, int y) {
    Serial.println("getTappedFace x: " + String(x) + " y: " + String(y));
    
    // 中心タップの場合は-1を返す
    if (isCenterTapped(x, y)) {
        return -1;
    }
    
    // タップされた面をOctagonRingViewのメソッドを使って判定
    return octagon.getFaceAtPoint(x, y);
}

bool LumiView::isCenterTapped(int x, int y) {
    Serial.println("isCenterTapped x: " + String(x) + " y: " + String(y));
    
    // 画面中央からの距離を計算（OctagonRingViewの配置を考慮）
    // 注：OctagonRingViewがビュー全体の中央に配置されている前提
    int centerX = octagon.viewX + octagon.viewWidth / 2;
    int centerY = octagon.viewY + octagon.viewHeight / 2;
    
    int relX = x - centerX;
    int relY = y - centerY;
    float distSq = relX * relX + relY * relY;
    
    // 中心円の半径（スクリーンサイズの20%程度）
    float centerRadiusSq = min(octagon.viewWidth, octagon.viewHeight) * 0.2f;
    centerRadiusSq *= centerRadiusSq; // 二乗値
    
    bool result = (distSq < centerRadiusSq);
    Serial.println("isCenterTapped result: " + String(result ? "true" : "false"));
    return result;
}

// スライダーのみを再描画するメソッド
void LumiView::drawSliders() {
    brightnessSlider.draw();
    hueSlider.draw();
    saturationSlider.draw();
}

// 明るさスライダーのみを再描画
void LumiView::drawBrightnessSlider() {
    brightnessSlider.draw();
}

// 色スライダーのみを再描画
void LumiView::drawHueSlider() {
    hueSlider.draw();
}

// 彩度スライダーのみを再描画
void LumiView::drawSaturationSlider() {
    saturationSlider.draw();
}

// 明度スライダーのみを再描画
void LumiView::drawValueBrightnessSlider() {
    valueBrightnessSlider.draw();
}
