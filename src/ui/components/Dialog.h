#ifndef DIALOG_H
#define DIALOG_H

#include <Arduino.h>
#include <M5Unified.h>
#include "Button.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// ダイアログのボタン種類
enum DialogType { DIALOG_OK, DIALOG_OK_CANCEL };
enum DialogResult { DIALOG_NONE, DIALOG_OK_PRESSED, DIALOG_CANCEL_PRESSED };

class Dialog {
private:
    String title;
    String message;
    DialogType type;
    bool isVisible;
    DialogResult result;
    
    // ダイアログの位置とサイズ
    // ダイアログは画面中央に配置、サイズと位置は固定、ボタンの位置は固定
    // サイズ（300x200)
    // (0,200)から(40,200)までをボタン領域とする
    const int DIALOG_WIDTH = 300;
    const int DIALOG_HEIGHT = 200;
    const int DIALOG_X = (SCREEN_WIDTH - DIALOG_WIDTH) / 2;
    const int DIALOG_Y = (SCREEN_HEIGHT - DIALOG_HEIGHT) / 2;

    Button okButton = Button(DIALOG_X + 10, DIALOG_Y + 150, 120, 40, "OK");
    Button cancelButton = Button(DIALOG_X + 160, DIALOG_Y + 150, 120, 40, "Cancel");
public:
    Dialog(); 
    void showDialog(const String& title, const String& message, DialogType type);
    DialogResult getResult();
    void hideDialog();
    void draw();
};

#endif
