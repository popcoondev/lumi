#include "StateManager.h"

StateManager::StateManager() {
    currentState = STATE_NONE;
    currentSubState = STATE_DETECTION_INIT;  // 適当な初期値
}

void StateManager::begin() {
    // 初期状態の設定
    stateInfo.mainState = STATE_NONE;
    stateInfo.subState = STATE_DETECTION_INIT;
    stateInfo.mainText = "System Ready";
    stateInfo.subText = "";
    stateInfo.buttonALabel = "Detect";
    stateInfo.buttonBLabel = "Calib";
    stateInfo.buttonCLabel = "LED";
    stateInfo.title = "Main Menu";
    stateInfo.status = "Ready";
}

void StateManager::changeState(State newState) {
    currentState = newState;
    
    // 新しい状態に応じてサブステートを初期化
    switch (newState) {
        case STATE_DETECTION:
            currentSubState = STATE_DETECTION_INIT;
            stateInfo.title = "Face Detection";
            stateInfo.mainText = "Initializing detection...";
            stateInfo.buttonALabel = "";
            stateInfo.buttonBLabel = "";
            stateInfo.buttonCLabel = "";
            break;
            
        case STATE_CALIBRATION:
            currentSubState = STATE_CALIBRATION_INIT;
            stateInfo.title = "Face Calibration";
            stateInfo.mainText = "Initializing calibration...";
            stateInfo.buttonALabel = "RESET";
            stateInfo.buttonBLabel = "SAVE";
            stateInfo.buttonCLabel = "LOAD";
            break;
            
        case STATE_LED_CONTROL:
            currentSubState = STATE_LED_CONTROL_INIT;
            stateInfo.title = "LED Control";
            stateInfo.mainText = "Initializing LED control...";
            stateInfo.buttonALabel = "play";
            stateInfo.buttonBLabel = "prev";
            stateInfo.buttonCLabel = "next";
            break;
            
        case STATE_NONE:
        default:
            currentSubState = STATE_DETECTION_INIT; // デフォルト
            stateInfo.title = "Main Menu";
            stateInfo.status = "Ready";
            stateInfo.mainText = "System Ready";
            stateInfo.subText = "";
            stateInfo.buttonALabel = "Detect";
            stateInfo.buttonBLabel = "Calib";
            stateInfo.buttonCLabel = "LED";
            break;
    }
    
    // 状態情報を更新
    stateInfo.mainState = currentState;
    stateInfo.subState = currentSubState;
}

void StateManager::changeSubState(SubState newSubState) {
    currentSubState = newSubState;
    stateInfo.subState = newSubState;
    
    // サブステートに応じた設定（必要に応じて実装）
    switch (currentState) {
        case STATE_LED_CONTROL:
            switch (newSubState) {
                case STATE_LED_CONTROL_PAUSED:
                    stateInfo.buttonALabel = "play";
                    break;
                case STATE_LED_CONTROL_PLAYING:
                    stateInfo.buttonALabel = "pause";
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void StateManager::processCurrentState() {
    // 現在の状態に合わせた処理
    // 主な処理はOctaControllerで行うため、ここでは最小限の処理のみ
    switch (currentState) {
        case STATE_DETECTION:
            // 検出状態の処理
            break;
            
        case STATE_CALIBRATION:
            // キャリブレーション状態の処理
            break;
            
        case STATE_LED_CONTROL:
            // LED制御状態の処理
            break;
            
        case STATE_NONE:
        default:
            // メインメニュー状態の処理
            break;
    }
}

void StateManager::updateStateInfo(StateInfo newInfo) {
    stateInfo = newInfo;
}

StateInfo StateManager::getCurrentStateInfo() {
    return stateInfo;
}