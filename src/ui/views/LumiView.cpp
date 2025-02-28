#include "ui/views/LumiView.h"
#include "core/Constants.h"

// Sliderクラスの実装
Slider::Slider(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height), 
      value(50), isDragging(false), 
      knobWidth(width - 10), knobHeight(30),
      barColor(TFT_DARKGREY), knobColor(TFT_LIGHTGREY)
{
}

void Slider::draw() {
    // バー背景
    M5.Lcd.fillRect(x, y, width, height, barColor);
    
    // つまみの位置を計算
    int knobY = y + (height - knobHeight) * (100 - value) / 100;
    
    // つまみを描画
    M5.Lcd.fillRoundRect(x + (width - knobWidth) / 2, knobY, knobWidth, knobHeight, 5, knobColor);
    
    // 値の表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawNumber(value, x + width / 2, knobY + knobHeight / 2);
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
    : settingsButton(320 - 40, 0, 40, 40, "⚙"),
      topLeftButton(0, 0, 40, 40, "?"),
      bottomLeftButton(0, 240 - 40, 40, 40, "?"),
      bottomRightButton(320 - 40, 240 - 40, 40, 40, "?"),
      brightnessSlider(0, 40, 40, 160),
      colorSlider(320 - 40, 40, 40, 160),
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
    octagon.rotate(0.3926991); // π/8ラジアン（22.5度）
    
    // ボタンのスタイル設定
    settingsButton.setColor(TFT_DARKGREY, TFT_LIGHTGREY);
    topLeftButton.setColor(TFT_DARKGREY, TFT_LIGHTGREY);
    bottomLeftButton.setColor(TFT_DARKGREY, TFT_LIGHTGREY);
    bottomRightButton.setColor(TFT_DARKGREY, TFT_LIGHTGREY);
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
    colorSlider.draw();
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
    
    // スライダーのタッチ処理
    if (brightnessSlider.handleTouch(touchX, touchY, isPressed)) {
        if (onBrightnessChanged) onBrightnessChanged(brightnessSlider.getValue());
        return;
    }
    
    if (colorSlider.handleTouch(touchX, touchY, isPressed)) {
        if (onColorChanged) onColorChanged(colorSlider.getValue());
        return;
    }
}

int LumiView::getTappedFace(int x, int y) {
    // オクタゴンのタップ判定は簡易的な実装
    // 実際には表示座標から逆変換して判定する必要あり
    
    // 中心からの相対座標
    int relX = x - octagonCenter.centerX;
    int relY = y - octagonCenter.centerY;
    
    // 中心から一定距離内なら中心タップとみなす
    float dist = sqrt(relX * relX + relY * relY);
    if (dist < octagonCenter.radius) {
        return -1; // 中心タップは別処理
    }
    
    // 中心からの角度を計算
    float angle = atan2(relY, relX);
    if (angle < 0) angle += 2 * M_PI;
    
    // 角度からどの面かを判定（8分割）
    int faceId = (int)((angle / (2 * M_PI) * 8) + 0.5) % 8;
    
    return faceId;
}

bool LumiView::isCenterTapped(int x, int y) {
    int relX = x - octagonCenter.centerX;
    int relY = y - octagonCenter.centerY;
    float dist = sqrt(relX * relX + relY * relY);
    return (dist < octagonCenter.radius);
}