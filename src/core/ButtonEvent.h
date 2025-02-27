#ifndef BUTTON_EVENT_H
#define BUTTON_EVENT_H

// ボタンイベント構造体
struct ButtonEvent {
    bool buttonA;
    bool buttonB;
    bool buttonC;
    bool isBackPressed;
    
    ButtonEvent() : buttonA(false), buttonB(false), buttonC(false), isBackPressed(false) {}
};

#endif // BUTTON_EVENT_H