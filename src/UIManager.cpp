#include "UIManager.h"

UIManager::UIManager() {
    lastStateInfo = StateInfo(); 
    isViewUpdate = false;
}

UIManager::~UIManager() {
}

void UIManager::begin() {
    // アクションバーの初期化
    actionBar.begin();
    
    // メインテキストビューの初期化
    mainTextView.begin();
    mainTextView.setPosition(0, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3*2, SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT);
    mainTextView.setFontSize(2);
    mainTextView.setColor(WHITE);
    mainTextView.setBackgroundColor(BLACK);
    mainTextView.setText("System Ready");
    
    // サブテキストビューの初期化
    int subViewHeight = (SCREEN_HEIGHT-ACTIONBAR_HEIGHT-TOOLBAR_HEIGHT)/2;
    subTextView.begin();
    subTextView.setPosition(SCREEN_WIDTH/3*2, ACTIONBAR_HEIGHT, SCREEN_WIDTH/3, subViewHeight);
    subTextView.setFontSize(1);
    subTextView.setColor(WHITE);
    subTextView.setBackgroundColor(BLACK);
    
    // オクタゴンビューの初期化
    octagon.setViewPosition(SCREEN_WIDTH/3*2, ACTIONBAR_HEIGHT+subViewHeight, SCREEN_WIDTH/3, subViewHeight);
    octagon.rotate(0.3926991); // π/8ラジアン（22.5度）
    
    // ツールバーの初期化
    toolbar.begin();
    toolbar.setButtonLabel(BTN_A, "Detect");
    toolbar.setButtonLabel(BTN_B, "Calib");
    toolbar.setButtonLabel(BTN_C, "LED");
    
    // 初期描画
    actionBar.draw();
    mainTextView.draw();
    subTextView.draw();
    toolbar.draw();
    octagon.draw();
    
    // 初期状態を保存
    lastStateInfo.title = "Main Menu";
    lastStateInfo.status = "Ready";
    lastStateInfo.mainText = "System Ready";
    lastStateInfo.subText = "";
    lastStateInfo.buttonALabel = "Detect";
    lastStateInfo.buttonBLabel = "Calib";
    lastStateInfo.buttonCLabel = "LED";
}

void UIManager::updateUI(StateInfo stateInfo) {
    bool needsRedraw = false;
    
    // タイトルの変更検出
    if (stateInfo.title != lastStateInfo.title || stateInfo.status != lastStateInfo.status) {
        actionBar.setTitle(stateInfo.title);
        actionBar.setStatus(stateInfo.status);
        needsRedraw = true;
    }
    
    // メインテキストの変更検出
    if (stateInfo.mainText != lastStateInfo.mainText) {
        mainTextView.setText(stateInfo.mainText);
        needsRedraw = true;
    }
    
    // サブテキストの変更検出
    if (stateInfo.subText != lastStateInfo.subText) {
        subTextView.setText(stateInfo.subText);
        needsRedraw = true;
    }
    
    // ボタンラベルの変更検出
    bool buttonChanged = false;
    if (stateInfo.buttonALabel != lastStateInfo.buttonALabel) {
        toolbar.setButtonLabel(BTN_A, stateInfo.buttonALabel);
        buttonChanged = true;
    }
    
    if (stateInfo.buttonBLabel != lastStateInfo.buttonBLabel) {
        toolbar.setButtonLabel(BTN_B, stateInfo.buttonBLabel);
        buttonChanged = true;
    }
    
    if (stateInfo.buttonCLabel != lastStateInfo.buttonCLabel) {
        toolbar.setButtonLabel(BTN_C, stateInfo.buttonCLabel);
        buttonChanged = true;
    }
    
    if (buttonChanged) {
        needsRedraw = true;
    }
    
    // 強制更新フラグがセットされている場合
    if (isViewUpdate) {
        needsRedraw = true;
        isViewUpdate = false;
    }
    
    // 変更があった場合のみ再描画
    if (needsRedraw) {
        // タイトルが変更された場合のみアクションバーを再描画
        if (stateInfo.title != lastStateInfo.title || stateInfo.status != lastStateInfo.status) {
            actionBar.draw();
        }
        
        // メインテキストが変更された場合のみ再描画
        if (stateInfo.mainText != lastStateInfo.mainText) {
            mainTextView.draw();
        }
        
        // サブテキストが変更された場合のみ再描画
        if (stateInfo.subText != lastStateInfo.subText) {
            subTextView.draw();
        }
        
        // ボタンラベルが変更された場合のみツールバーを再描画
        if (buttonChanged) {
            toolbar.draw();
        }
        
        // オクタゴンは必要な場合のみ再描画
        if (isOctagonDirty) {
            octagon.draw();
            isOctagonDirty = false;
        }
    }
    
    // 現在の状態を保存
    lastStateInfo = stateInfo;
}

ButtonEvent UIManager::getButtonEvents() {
    ButtonEvent event;
    event.buttonA = toolbar.getPressedButton(BTN_A);
    event.buttonB = toolbar.getPressedButton(BTN_B);
    event.buttonC = toolbar.getPressedButton(BTN_C);
    event.isBackPressed = actionBar.isBackPressed();
    
    return event;
}

DialogResult UIManager::showDialog(String title, String message, DialogType type) {
    dialog.showDialog(title, message, type);
    
    // ダイアログの結果を待つ
    DialogResult result = DIALOG_NONE;
    while (result == DIALOG_NONE) {
        M5.update(); // ボタン状態を更新
        result = dialog.getResult();
        delay(50);
    }

    // 少し遅延を入れてから再描画する
    delay(50);

    // 画面全体を強制的に再描画
    forcefullyRedraw();
    
    return result;
}

void UIManager::highlightFace(int faceId) {
    if (octagon.getHighlightedFace() != faceId) {
        octagon.setHighlightedFace(faceId);
        isOctagonDirty = true;
    }
}

void UIManager::forcefullyRedraw() {
    // LCDバッファをクリア（オプション、必要な場合のみ）
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // すべてのUI要素を再描画
    actionBar.draw();
    
    // テキストビューの内容を一度クリアして再描画することで強制更新
    String mainText = mainTextView.getText();
    String subText = subTextView.getText();
    
    mainTextView.clear();
    mainTextView.setText(mainText);
    mainTextView.draw();
    
    subTextView.clear();
    subTextView.setText(subText);
    subTextView.draw();
    
    toolbar.draw();
    octagon.draw();
    
    // 更新フラグをリセット
    isViewUpdate = false;
    isOctagonDirty = false;
}