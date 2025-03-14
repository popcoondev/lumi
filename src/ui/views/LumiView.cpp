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
    M5.Lcd.setTextColor(TFT_WHITE);
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

int Slider::getValue() const {
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
      bottomLeftButton(0, 240 - CORNER_BUTTON_HEIGHT, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Reset"),
      settingsButton(320 - CORNER_BUTTON_WIDTH, 0, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Settings"),
      bottomRightButton(320 - CORNER_BUTTON_WIDTH, 240 - CORNER_BUTTON_HEIGHT, CORNER_BUTTON_WIDTH, CORNER_BUTTON_HEIGHT, "Pattern"),
      brightnessSlider(0, 40, 40, 80),
      valueBrightnessSlider(0, 120, 40, 80),
      hueSlider(320 - 40, 40, 40, 80),
      saturationSlider(320 - 40, 120, 40, 80),
      isTouchActive(false),
      lastTouchX(0),
      lastTouchY(0),
      isDragging(false),
      dragStartFace(-1),
      lastDraggedFace(-1),
      currentMode(MODE_TAP),
      selectedPatternIndex(0),
      isPatternPlaying(false),
      backgroundColor(BLACK)
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
    octagon.setBackgroundColor(BLACK);

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

// モード設定
void LumiView::setOperationMode(OperationMode mode) {
    currentMode = mode;
    
    // モードに応じたボタンラベルの設定
    if (mode == MODE_TAP) {
        bottomRightButton.setLabel("Pattern");
    } else if (mode == MODE_PATTERN) {
        bottomRightButton.setLabel("Listen");
    } else if (mode == MODE_LISTEN) {
        bottomRightButton.setLabel("Tap");
    }
    
    clearAllFocus();
    
    // ボタンを再描画
    bottomRightButton.draw();
}

// パターン選択の更新
void LumiView::updatePatternSelection(int moveCount) {    
    // パターン数（LEDManagerから取得する想定）
    const int patternCount = ledPatterns;

    // 選択中のパターンを更新
    selectedPatternIndex = (selectedPatternIndex + moveCount + patternCount) % patternCount;
}

void LumiView::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // オクタゴンを描画（モードに関わらず常に描画）
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

// センターボタン情報表示の更新
void LumiView::updateCenterButtonInfo() {
    String text;
    uint16_t color;
    
    if (currentMode == MODE_TAP) {
        // タップモードの場合
        int ledState = getFocusedFacesLedState();
        int focusCount = getFocusedFacesCount();
        
        switch (ledState) {
            case -1: // フォーカスなし
                text = "";
                color = WHITE;
                break;
            case 0: // すべて消灯
                text = "ON " + String(focusCount);
                color = TFT_GREEN;
                break;
            case 2: // すべて点灯
                text = "OFF " + String(focusCount);
                color = TFT_RED;
                break;
            case 1: // 一部点灯
                // フォーカスされた面の中で点灯している面の数を取得
                int onCount = 0;
                for (int i = 0; i < NUM_FACES; i++) {
                    if (octagon.isFaceFocused(i) && octagon.isFaceHighlighted(i)) {
                        onCount++;
                    }
                }
                text = "TOGGLE " + String(onCount) + "/" + String(focusCount);
                color = TFT_YELLOW;
                break;
        }
    } else if (currentMode == MODE_PATTERN) {
        // パターンモードの場合
        String patternName = "Pattern " + String(selectedPatternIndex + 1);
        if (isPatternPlaying) {
            text = "STOP " + patternName;
            color = TFT_RED;
        } else {
            text = "PLAY " + patternName;
            color = TFT_GREEN;
        }
    } else if (currentMode == MODE_LISTEN) {
        // リッスンモードの場合
        text = "MIC";
        color = TFT_CYAN;
    }
        
    // テキスト情報を表示
    drawCenterButtonInfo(text, color);
}

// センターボタン情報の描画
void LumiView::drawCenterButtonInfo(const String& text, uint16_t color) {
    int centerX = octagon.viewX + octagon.viewWidth / 2;
    int centerY = octagon.viewY + octagon.viewHeight / 2;

    // 中央円を背景色で描画
    float innerRadius = min(octagon.viewWidth, octagon.viewHeight) * 0.2f;
    M5.Lcd.fillCircle(centerX, centerY, innerRadius, BLACK);
    
    // テキスト表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString(text, centerX, centerY);
}

// オクタゴンの中心がタップされたか判定
bool LumiView::isCenterTapped(int x, int y) {
    int centerX = octagon.viewX + octagon.viewWidth / 2;
    int centerY = octagon.viewY + octagon.viewHeight / 2;
    float innerRadius = min(octagon.viewWidth, octagon.viewHeight) * 0.2f;
    
    // 中心からの距離を計算
    float dx = x - centerX;
    float dy = y - centerY;
    float distSq = dx * dx + dy * dy;
    
    // 中心円内かどうかを判定
    return distSq < innerRadius * innerRadius;
}

// オクタゴンの面がタップされたか判定
int LumiView::getTappedFace(int x, int y) {
    return octagon.getFaceAtPoint(x, y);
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
    
    // モードに応じた処理
    if (currentMode == MODE_TAP) {
        // タップモードの場合
        
        // オクタゴンの面タップ判定
        if (wasPressed) {
            int faceId = getTappedFace(touchX, touchY);
            if (faceId >= 0) {
                activeTouchedUI = TouchedUI(ID_OCTAGON_FACE_BASE + faceId, faceId);
                // タップ視覚フィードバック - 一時的に色を変更
                uint16_t originalColor = octagon.getFaceColor(faceId);
                octagon.setFaceTempColor(faceId, TFT_DARKGREY);
                octagon.drawFace(faceId);  // 指定した面だけを再描画
                
                // ドラッグ選択の開始
                isDragging = true;
                dragStartFace = faceId;
                lastDraggedFace = faceId;
                
                // フォーカス状態をトグル
                octagon.setFaceFocused(faceId, !octagon.isFaceFocused(faceId));
                octagon.drawFace(faceId);  // フォーカス状態を反映して再描画
                
                // センターボタン情報を更新
                updateCenterButtonInfo();
            }
        }

        // タッチ継続中の処理
        if (isPressed && !wasPressed && !wasReleased) {
            // オクタゴン中心のドラッグ離脱検出
            if (activeTouchedUI.id == ID_OCTAGON_CENTER && !isCenterTapped(touchX, touchY)) {
                // センター領域から外れた - 視覚フィードバックを元に戻す
                M5.Lcd.fillCircle(octagon.viewX + octagon.viewWidth / 2, 
                                octagon.viewY + octagon.viewHeight / 2, 
                                min(octagon.viewWidth, octagon.viewHeight) * 0.1f,
                                backgroundColor);
                octagon.drawCenter();  // センター部分のみ再描画
                // updateCenterButtonInfo(); // 情報表示を更新
            }
            
            // 面のドラッグ選択処理
            if (isDragging) {
                int currentFaceId = getTappedFace(touchX, touchY);
                if (currentMode == MODE_TAP) {
                    if (currentFaceId >= 0 && currentFaceId != lastDraggedFace) {
                        // 新しい面にドラッグされた
                        lastDraggedFace = currentFaceId;
                        
                        // 新しい面のフォーカス状態を設定（最初にタップした面と同じ状態に）
                        bool shouldFocus = octagon.isFaceFocused(dragStartFace);
                        octagon.setFaceFocused(currentFaceId, shouldFocus);
                        octagon.drawFace(currentFaceId);  // 再描画
                        
                        // センターボタン情報を更新
                        updateCenterButtonInfo();
                    }
                }
            }
        }
    }
    else {
        // パターンモードの場合
        if (wasPressed) {
            if (isCenterTapped(touchX, touchY)) {
                // センター再描画で視覚フィードバックを元に戻す
                // octagon.drawCenter();
                // updateCenterButtonInfo(); // 情報表示を更新
                
                // センターがタップされた場合はパターン再生トグル
                isPatternPlaying = !isPatternPlaying;
                updateCenterButtonInfo();
            }

            int faceId = getTappedFace(touchX, touchY);
            if (faceId == 3) {
                updatePatternSelection(-1);
                // drawPatternWheel();
            } else if (faceId == 7) {
                updatePatternSelection(1);
                // drawPatternWheel();
            }
        }
    }
    
    // タッチ終了時の処理
    if (wasReleased) {
        if (activeTouchedUI.id == ID_OCTAGON_CENTER) {
            // センター再描画で視覚フィードバックを元に戻す
            octagon.drawCenter();
            updateCenterButtonInfo(); // 情報表示を更新
            
            // 同じ領域上でリリースされた場合のみアクション実行
            if (isCenterTapped(touchX, touchY) && onCenterTapped) {
                onCenterTapped();
            }
        }
        else if (activeTouchedUI.id >= ID_OCTAGON_FACE_BASE) {
            int faceId = activeTouchedUI.data;
            
            // 一時的な色を元に戻す
            octagon.resetFaceTempColor(faceId);
            octagon.drawFace(faceId);
            
            // ドラッグ選択の終了
            isDragging = false;
            dragStartFace = -1;
            lastDraggedFace = -1;
            
            // 同じ面上でリリースされた場合のみアクション実行（タップ時にすでにフォーカス処理済み）
            int releasedFaceId = getTappedFace(touchX, touchY);
            if (releasedFaceId == faceId && onFaceTapped) {
                // onFaceTapped(faceId); // フォーカス処理に変更したため不要
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
            // button.draw();
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
    
    // タッチ開始時
    if (wasPressed && isTouchInSlider) {
        activeTouchedUI = TouchedUI(slider.getId());
        slider.handleTouch(touchX, touchY, true);
        
        // スライダー値変更のコールバック
        int id = slider.getId();
        int value = slider.getValue();
        
        if (id == ID_SLIDER_BRIGHTNESS && onBrightnessChanged) {
            onBrightnessChanged(value);
        }
        else if (id == ID_SLIDER_HUE && onHueChanged) {
            onHueChanged(value);
        }
        else if (id == ID_SLIDER_SATURATION && onSaturationChanged) {
            onSaturationChanged(value);
        }
        else if (id == ID_SLIDER_VALUE_BRIGHTNESS && onValueBrightnessChanged) {
            onValueBrightnessChanged(value);
        }
        
        return true;
    }
    
    // ドラッグ中の処理
    if (isPressed && !wasPressed && !wasReleased && activeTouchedUI.id == slider.getId()) {
        if (slider.handleTouch(touchX, touchY, true)) {
            // スライダー値変更のコールバック
            int id = slider.getId();
            int value = slider.getValue();
            
            if (id == ID_SLIDER_BRIGHTNESS && onBrightnessChanged) {
                onBrightnessChanged(value);
            }
            else if (id == ID_SLIDER_HUE && onHueChanged) {
                onHueChanged(value);
            }
            else if (id == ID_SLIDER_SATURATION && onSaturationChanged) {
                onSaturationChanged(value);
            }
            else if (id == ID_SLIDER_VALUE_BRIGHTNESS && onValueBrightnessChanged) {
                onValueBrightnessChanged(value);
            }
        }
        return true;
    }
    
    // タッチ終了時
    if (wasReleased && activeTouchedUI.id == slider.getId()) {
        slider.handleTouch(touchX, touchY, false);
        activeTouchedUI = TouchedUI();
        return true;
    }
    
    return false;
}

// スライダーの描画
void LumiView::drawSliders() {
    brightnessSlider.draw();
    valueBrightnessSlider.draw();
    hueSlider.draw();
    saturationSlider.draw();
}

// 明るさスライダーの描画
void LumiView::drawBrightnessSlider() {
    brightnessSlider.draw();
}

// 明度スライダーの描画
void LumiView::drawValueBrightnessSlider() {
    valueBrightnessSlider.draw();
}

// 色相スライダーの描画
void LumiView::drawHueSlider() {
    hueSlider.draw();
}

// 彩度スライダーの描画
void LumiView::drawSaturationSlider() {
    saturationSlider.draw();
}
