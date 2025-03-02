#include "ui/views/LumiView.h"
#include "core/Constants.h"

#define CORNER_BUTTON_WIDTH 70
#define CORNER_BUTTON_HEIGHT 40

// Sliderクラスの実装
Slider::Slider(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height), 
      value(50), isDragging(false), 
      knobWidth(8), knobHeight(8),
      barColor(BLACK), knobColor(WHITE),
      title("Slider"), id(0)
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

bool Slider::containsPoint(int x, int y) const {
    return (x >= this->x && x < this->x + width && y >= this->y && y < this->y + height);
}

// LumiViewクラスの実装
LumiView::LumiView()
    : resetButton(0, 0, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Reset"),
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

    // idを設定
    settingsButton.setId(ID_BUTTON_SETTINGS);
    resetButton.setId(ID_BUTTON_RESET);
    bottomLeftButton.setId(ID_BUTTON_BOTTOM_LEFT);
    bottomRightButton.setId(ID_BUTTON_BOTTOM_RIGHT);
    brightnessSlider.setId(ID_SLIDER_BRIGHTNESS);
    valueBrightnessSlider.setId(ID_SLIDER_VALUE_BRIGHTNESS);
    hueSlider.setId(ID_SLIDER_HUE);
    saturationSlider.setId(ID_SLIDER_SATURATION);


    // ボタンのスタイル設定
    settingsButton.setColor(BLACK, TFT_LIGHTGREY);
    settingsButton.setFontSize(1.4);
    settingsButton.setType(BUTTON_TYPE_TEXT);
    
    resetButton.setColor(BLACK, TFT_LIGHTGREY);
    resetButton.setFontSize(1.4);
    resetButton.setType(BUTTON_TYPE_TEXT);

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
    resetButton.draw();
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
    bool isPressed = touch.isPressed();
    bool wasPressed = touch.wasPressed();
    bool wasReleased = touch.wasReleased();

    // タッチ座標取得
    int touchX = touch.x;
    int touchY = touch.y;
    
    // タッチ状態の更新
    isTouchActive = isPressed;
    if (isPressed) {
        lastTouchX = touchX;
        lastTouchY = touchY;
    }
    
    // ボタンのタッチ処理
    if (checkButtonTouch(settingsButton, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkButtonTouch(resetButton, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkButtonTouch(bottomLeftButton, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkButtonTouch(bottomRightButton, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    
    // スライダーのタッチ処理
    if (checkSliderTouch(brightnessSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkSliderTouch(hueSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkSliderTouch(saturationSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    if (checkSliderTouch(valueBrightnessSlider, touchX, touchY, isPressed, wasPressed, wasReleased)) return;
    
    // オクタゴンの中心タップ判定
    if (wasPressed && isCenterTapped(touchX, touchY)) {
        activeTouchedUI = TouchedUI(ID_OCTAGON_CENTER);
    }
    
    // オクタゴンの面タップ判定
    if (wasPressed) {
        int faceId = getTappedFace(touchX, touchY);
        if (faceId >= 0) {
            activeTouchedUI = TouchedUI(ID_OCTAGON_FACE_BASE + faceId, faceId);
        }
    }
    
    // タッチ終了時の処理
    if (wasReleased) {
        if (activeTouchedUI.id == ID_OCTAGON_CENTER) {
            if (isCenterTapped(touchX, touchY) && onCenterTapped) {
                onCenterTapped();
            }
        }
        else if (activeTouchedUI.id >= ID_OCTAGON_FACE_BASE) {
            // 同じ面上でリリースされたか確認
            int faceId = getTappedFace(touchX, touchY);
            if (faceId == activeTouchedUI.data && onFaceTapped) {
                onFaceTapped(faceId);
            }
        }
        
        // タッチ情報をリセット
        activeTouchedUI = TouchedUI();

    }
}

// ボタンタッチ処理のヘルパーメソッド
bool LumiView::checkButtonTouch(Button& button, int touchX, int touchY, bool isPressed, bool wasPressed, bool wasReleased) {
    // タッチ開始時
    if (wasPressed) {
        if (button.containsPoint(touchX, touchY)) {
            activeTouchedUI = TouchedUI(button.getId());
            button.setPressed(true);
            button.draw();
            return true;
        }
    }
    
    // ボタンがアクティブな状態でタッチ終了時
    if (wasReleased && activeTouchedUI.id == button.getId()) {
        button.setPressed(false);
        button.draw();
        
        // ボタン上でリリースされた場合のみコールバック実行
        if (button.containsPoint(touchX, touchY)) {
            int id = button.getId();
            if (id == ID_BUTTON_SETTINGS && onTopRightButtonTapped) {
                onTopRightButtonTapped();
            }
            else if (id == ID_BUTTON_RESET && onTopLeftButtonTapped) {
                onTopLeftButtonTapped();
            }
            else if (id == ID_BUTTON_BOTTOM_LEFT && onBottomLeftButtonTapped) {
                onBottomLeftButtonTapped();
            }
            else if (id == ID_BUTTON_BOTTOM_RIGHT && onBottomRightButtonTapped) {
                onBottomRightButtonTapped();
            }
        }
        
        activeTouchedUI = TouchedUI();
        return true;
    }
    
    return false;
}

// スライダータッチ処理のヘルパーメソッド
bool LumiView::checkSliderTouch(Slider& slider, int touchX, int touchY, bool isPressed, bool wasPressed, bool wasReleased) {
    // スライダー領域内のタッチかチェック
    bool isTouchInSlider = slider.containsPoint(touchX, touchY);
    
    // タッチ開始時または既にこのスライダーをドラッグ中
    if ((wasPressed && isTouchInSlider) || 
        (isPressed && activeTouchedUI.id == slider.getId())) {
        
        // アクティブなUIをこのスライダーに設定
        if (wasPressed) {
            activeTouchedUI = TouchedUI(slider.getId());
        }
        
        // 値を更新
        int oldValue = slider.getValue();
        slider.handleTouch(touchX, touchY, isPressed);
        int newValue = slider.getValue();
        
        // 値が変わった場合のみ再描画とコールバック実行
        if (oldValue != newValue) {
            slider.draw();
            
            int id = slider.getId();
            if (id == ID_SLIDER_BRIGHTNESS && onBrightnessChanged) {
                onBrightnessChanged(newValue);
            }
            else if (id == ID_SLIDER_HUE && onHueChanged) {
                onHueChanged(newValue);
            }
            else if (id == ID_SLIDER_SATURATION && onSaturationChanged) {
                onSaturationChanged(newValue);
            }
            else if (id == ID_SLIDER_VALUE_BRIGHTNESS && onValueBrightnessChanged) {
                onValueBrightnessChanged(newValue);
            }
        }
        
        return true;
    }
    
    // ドラッグ終了時
    if (wasReleased && activeTouchedUI.id == slider.getId()) {
        activeTouchedUI = TouchedUI();
        return true;
    }
    
    return false;
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
