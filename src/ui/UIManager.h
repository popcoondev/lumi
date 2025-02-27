#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <M5Unified.h>
#include "ActionBar.h"
#include "TextView.h"
#include "Toolbar.h"
#include "Dialog.h"
#include "M5StackChanFace.h"
#include "OctagonRingView.h"
#include "StateManager.h"
#include "ButtonEvent.h"
#include "Constants.h"

class UIManager {
private:
    ActionBar actionBar;
    TextView mainTextView;
    TextView subTextView;
    Toolbar toolbar;
    Dialog dialog;
    OctagonRingView octagon;
    
    bool isViewUpdate;      // 強制更新フラグ
    bool isOctagonDirty;    // オクタゴン表示の更新フラグ
    StateInfo lastStateInfo; // 前回の状態情報

public:
    UIManager();
    ~UIManager();
    void begin();
    void updateUI(StateInfo stateInfo);
    ButtonEvent getButtonEvents();
    DialogResult showDialog(String title, String message, DialogType type);
    void highlightFace(int faceId);
    // 画面を強制的に再描画するメソッド
    void forcefullyRedraw();
    
    // 特定の要素だけを再描画するメソッド
    void redrawMainText() { mainTextView.draw(); }
    void redrawSubText() { subTextView.draw(); }
    void redrawActionBar() { actionBar.draw(); }
    void redrawToolbar() { toolbar.draw(); }
    void redrawOctagon() { octagon.draw(); }
};

#endif // UI_MANAGER_H