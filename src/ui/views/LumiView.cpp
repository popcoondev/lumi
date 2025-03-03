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
      currentProgressMode(PROGRESS_MODE_NONE),
      progressValue(0),
      lastProgressUpdateTime(0),
      progressAnimationFrame(0),
      needsProgressUpdate(false),
      currentMode(MODE_TAP),
      isWheelActive(false),
      wheelRotation(0),
      selectedPatternIndex(0),
      isPatternPlaying(false)
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

// 円形プログレスバーの描画
void LumiView::drawCircularProgress(int value, ProgressMode mode) {
    int centerX = octagon.viewX + octagon.viewWidth / 2;
    int centerY = octagon.viewY + octagon.viewHeight / 2;
    float innerRadius = min(octagon.viewWidth, octagon.viewHeight) * 0.2f;
    float outerRadius = innerRadius + 2; // プログレスバーの幅
    
    // モードに応じた色を設定
    uint16_t progressColor;
    switch (mode) {
        case PROGRESS_MODE_BRIGHTNESS:
            progressColor = TFT_YELLOW;
            break;
        case PROGRESS_MODE_HUE:
            // 色相に応じた色
            // progressColor = M5.Lcd.color565(
            //     map(value, 0, 100, 0, 255), 
            //     map((value + 33) % 100, 0, 100, 0, 255), 
            //     map((value + 66) % 100, 0, 100, 0, 255)
            // );
            break;
        case PROGRESS_MODE_SATURATION:
            // 現在の色相と彩度に基づく色
            // progressColor = M5.Lcd.color565(
            //     map(value, 0, 100, 128, 255), 
            //     map(value, 0, 100, 128, 0), 
            //     map(value, 0, 100, 128, 255)
            // );
            break;
        case PROGRESS_MODE_PATTERN:
            progressColor = TFT_CYAN;
            break;
        default:
            progressColor = TFT_WHITE;
            break;
    }
    
    // 中央円を背景色で描画
    // M5.Lcd.fillCircle(centerX, centerY, innerRadius, BLACK);
    
    if (mode == PROGRESS_MODE_PATTERN) {
        // パターンモードの場合は回転アニメーション
        float startAngle = progressAnimationFrame * 15.0f; // 15度ずつ回転
        
        // 4つの弧を描画
        for (int i = 0; i < 4; i++) {
            float arcStart = startAngle + (i * 90);
            float arcEnd = arcStart + 30; // 30度の弧
            
            // 弧を描画（外側から内側に向かって）
            for (float r = outerRadius; r > innerRadius; r -= 0.5) {
                M5.Lcd.drawArc(centerX, centerY, r, r, arcStart, arcEnd, progressColor);
            }
        }
    } else {
        // プログレスモードの場合は進捗を表示
        float endAngle = map(value, 0, 100, 0, 360);
        
        // プログレス弧を描画（外側から内側に向かって）
        for (float r = outerRadius; r > innerRadius; r -= 0.5) {
            M5.Lcd.drawArc(centerX, centerY, r, r, 0, endAngle, progressColor);
        }
    }
    
    // 現在のモードと値を保存
    currentProgressMode = mode;
    progressValue = value;
    needsProgressUpdate = false;
}

// 円形プログレスアニメーションの更新
void LumiView::updateCircularProgressAnimation() {
    unsigned long currentTime = millis();
    
    // パターンモードの場合は100msごとにアニメーションを更新
    if (currentProgressMode == PROGRESS_MODE_PATTERN && 
        currentTime - lastProgressUpdateTime > 100) {
        
        progressAnimationFrame = (progressAnimationFrame + 1) % 24; // 24フレームでループ
        drawCircularProgress(progressValue, currentProgressMode);
        lastProgressUpdateTime = currentTime;
    }
    
    // プログレスモードで更新が必要な場合
    if (needsProgressUpdate && currentProgressMode != PROGRESS_MODE_PATTERN) {
        drawCircularProgress(progressValue, currentProgressMode);
    }
}

// モード設定
void LumiView::setOperationMode(OperationMode mode) {
    currentMode = mode;
    
    // モードに応じたボタンラベルの設定
    if (mode == MODE_TAP) {
        bottomRightButton.setLabel("Pattern");
        isWheelActive = false;
    } else {
        bottomRightButton.setLabel("Tap");
        isWheelActive = true;
        
        // パターンモードに切り替えたときはフォーカスをクリア
        clearAllFocus();
    }
    
    // ボタンを再描画
    bottomRightButton.draw();
}

// パターンホイールの描画
void LumiView::drawPatternWheel() {
    if (!isWheelActive) return;
    
    int centerX = octagon.viewX + octagon.viewWidth / 2;
    int centerY = octagon.viewY + octagon.viewHeight / 2;
    float outerRadius = min(octagon.viewWidth, octagon.viewHeight) * 0.45f;
    float markerRadius = outerRadius * 0.85f;
    
    // パターン数（LEDManagerから取得する想定）
    const int patternCount = 8; // 仮の値
    
    // 各パターンの位置にマーカーを描画
    for (int i = 0; i < patternCount; i++) {
        float angle = wheelRotation + (2 * PI * i / patternCount);
        int markerX = centerX + markerRadius * cos(angle);
        int markerY = centerY + markerRadius * sin(angle);
        
        // 選択中のパターンは大きく黄色で表示
        if (i == selectedPatternIndex) {
            M5.Lcd.fillCircle(markerX, markerY, 8, TFT_YELLOW);
        } else {
            M5.Lcd.fillCircle(markerX, markerY, 5, TFT_WHITE);
        }
    }
    
    // 選択中のパターン名を表示
    String patternName = "Pattern " + String(selectedPatternIndex + 1);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString(patternName, centerX, centerY + 30);
}

// パターン選択の更新
void LumiView::updatePatternSelection(float rotationDelta) {
    if (!isWheelActive) return;
    
    // ホイールの回転を更新
    wheelRotation += rotationDelta;
    
    // パターン数（LEDManagerから取得する想定）
    const int patternCount = 8; // 仮の値
    
    // 回転角度からパターンインデックスを計算
    float normalizedRotation = wheelRotation / (2 * PI);
    normalizedRotation = normalizedRotation - floor(normalizedRotation); // 0-1の範囲に正規化
    
    int newPatternIndex = floor(normalizedRotation * patternCount);
    if (newPatternIndex != selectedPatternIndex) {
        selectedPatternIndex = newPatternIndex;
        // パターン変更のコールバックはOctaControllerで処理
    }
}

void LumiView::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    if (currentMode == MODE_TAP || !isWheelActive) {
        // タップモードの場合は通常のオクタゴンを描画
        octagon.draw();
    } else {
        // パターンモードの場合はホイール表示
        drawPatternWheel();
    }
    
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
    
    // 円形プログレスバーを描画（現在のモードに応じて）
    if (currentProgressMode != PROGRESS_MODE_NONE) {
        drawCircularProgress(progressValue, currentProgressMode);
    }
}

// センターボタン情報表示の更新
void LumiView::updateCenterButtonInfo() {
    String text;
    uint16_t color;
    bool showProgress = false;
    
    if (currentMode == MODE_TAP) {
        // タップモードの場合
        int ledState = getFocusedFacesLedState();
        int focusCount = getFocusedFacesCount();
        
        switch (ledState) {
            case -1: // フォーカスなし
                text = "";
                color = WHITE;
                // プログレス表示はOFF
                currentProgressMode = PROGRESS_MODE_NONE;
                break;
            case 0: // すべて消灯
                text = "ON " + String(focusCount);
                color = TFT_GREEN;
                // プログレス表示はOFF
                currentProgressMode = PROGRESS_MODE_NONE;
                break;
            case 2: // すべて点灯
                text = "OFF " + String(focusCount);
                color = TFT_RED;
                // プログレス表示はOFF
                currentProgressMode = PROGRESS_MODE_NONE;
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
                // プログレス表示はOFF
                currentProgressMode = PROGRESS_MODE_NONE;
                break;
        }
    } else {
        // パターンモードの場合
        String patternName = "Pattern " + String(selectedPatternIndex + 1);
        if (isPatternPlaying) {
            text = "STOP " + patternName;
            color = TFT_RED;
            // パターン実行中はアニメーション表示
            showProgress = true;
        } else {
            text = "PLAY " + patternName;
            color = TFT_GREEN;
            // プログレス表示はOFF
            currentProgressMode = PROGRESS_MODE_NONE;
        }
    }
    
    // 中央円を背景色で描画（プログレスを消去）
    int centerX = octagon.viewX + octagon.viewWidth / 2;
    int centerY = octagon.viewY + octagon.viewHeight / 2;
    float innerRadius = min(octagon.viewWidth, octagon.viewHeight) * 0.2f;
    M5.Lcd.fillCircle(centerX, centerY, innerRadius, BLACK);
    
    // 必要な場合のみプログレスを表示
    if (showProgress) {
        drawCircularProgress(0, PROGRESS_MODE_PATTERN);
    }
    
    // テキスト情報を表示
    drawCenterButtonInfo(text, color);
}

// センターボタン情報の描画
void LumiView::drawCenterButtonInfo(const String& text, uint16_t color) {
    int centerX = octagon.viewX + octagon.viewWidth / 2;
    int centerY = octagon.viewY + octagon.viewHeight / 2;
    
    // テキスト表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(color);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString(text, centerX, centerY);
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
                updateCenterButtonInfo(); // 情報表示を更新
            }
            
            // 面のドラッグ選択処理
            if (isDragging) {
                int currentFaceId = getTappedFace(touchX, touchY);
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
    } else {
        // パターンモードの場合
        
        // ホイール操作の処理
        if (isPressed && !isCenterTapped(touchX, touchY)) {
            // 中心からの角度を計算
            int centerX = octagon.viewX + octagon.viewWidth / 2;
            int centerY = octagon.viewY + octagon.viewHeight / 2;
            
            // 現在の角度
            float currentAngle = atan2(touchY - centerY, touchX - centerX);
            
            // ドラッグ中の場合は角度の変化を計算
            if (isPressed && !wasPressed) {
                // 前回の角度
                float lastAngle = atan2(lastTouchY - centerY, lastTouchX - centerX);
                
                // 角度の変化量
                float angleDelta = currentAngle - lastAngle;
                
                // 角度の変化が大きすぎる場合は補正（-πからπの範囲に収める）
                if (angleDelta > PI) angleDelta -= 2 * PI;
                if (angleDelta < -PI) angleDelta += 2 * PI;
                
                // ホイールの回転を更新
                updatePatternSelection(angleDelta);
                
                // 再描画
                draw();
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
        else if (activeTouchedUI.id >= ID_OCTAGON_FACE_BASE && currentMode == MODE_TAP) {
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
    
    // タッチ開始時または既にこのスライダーをドラッグ中
    if ((wasPressed && isTouchInSlider) || 
        (isPressed && activeTouchedUI.id == slider.getId())) {
        
        // アクティブなUIをこのスライダーに設定
        if (wasPressed) {
            activeTouchedUI = TouchedUI(slider.getId());
            
            // スライダーに応じたプログレスモードを設定
            int id = slider.getId();
            if (id == ID_SLIDER_BRIGHTNESS) {
                currentProgressMode = PROGRESS_MODE_BRIGHTNESS;
            }
            else if (id == ID_SLIDER_HUE) {
                currentProgressMode = PROGRESS_MODE_HUE;
            }
            else if (id == ID_SLIDER_SATURATION) {
                currentProgressMode = PROGRESS_MODE_SATURATION;
            }
            else if (id == ID_SLIDER_VALUE_BRIGHTNESS) {
                currentProgressMode = PROGRESS_MODE_BRIGHTNESS; // 明度も明るさと同じ表示
            }
        }
        
        // 値を更新
        int oldValue = slider.getValue();
        slider.handleTouch(touchX, touchY, isPressed);
        int newValue = slider.getValue();
        
        // 値が変わった場合のみ再描画とコールバック実行
        if (oldValue != newValue) {
            slider.draw();
            
            // プログレスバーを更新
            drawCircularProgress(newValue, currentProgressMode);
            
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
        // スライダー操作終了時にプログレスモードをリセット
        currentProgressMode = PROGRESS_MODE_NONE;
        
        // センターボタン情報を更新（プログレスバーを消去）
        updateCenterButtonInfo();
        
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
    valueBrightnessSlider.draw();
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
