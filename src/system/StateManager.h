#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>

// メインステート
enum State { 
    STATE_NONE, 
    STATE_DETECTION, 
    STATE_CALIBRATION, 
    STATE_LED_CONTROL 
};

// サブステート
enum SubState {
    // Detection サブステート
    STATE_DETECTION_INIT, 
    STATE_DETECTION_DETECT_FACE,
    
    // Calibration サブステート
    STATE_CALIBRATION_INIT, 
    STATE_CALIBRATION_WAIT_STABLE, 
    STATE_CALIBRATION_DETECT_FACE, 
    STATE_CALIBRATION_CONFIRM_NEW_FACE, 
    STATE_CALIBRATION_COMPLETE,
    
    // LED Control サブステート
    STATE_LED_CONTROL_INIT, 
    STATE_LED_CONTROL_PAUSED, 
    STATE_LED_CONTROL_PLAYING, 
    STATE_LED_CONTROL_STOP
};

// 状態情報
class StateInfo {
public:
    State mainState;
    SubState subState;
    String mainText;
    String subText;
    String buttonALabel;
    String buttonBLabel;
    String buttonCLabel;
    String title;
    String status;
};

class StateManager {
private:
    State currentState;
    SubState currentSubState;
    StateInfo stateInfo;

public:
    StateManager();
    void begin();
    void changeState(State newState);
    void changeSubState(SubState newSubState);
    void processCurrentState();
    void updateStateInfo(StateInfo newInfo);
    StateInfo getCurrentStateInfo();
};

#endif // STATE_MANAGER_H