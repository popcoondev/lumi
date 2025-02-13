#include "Dialog.h"

Dialog::Dialog() : isVisible(false), result(DIALOG_NONE) {}

void Dialog::showDialog(const String& titleText, const String& messageText, DialogType dialogType) {
    title = titleText;
    message = messageText;
    type = dialogType;
    isVisible = true;
    result = DIALOG_NONE;
    draw();
}

// DialogResult Dialog::getResult() {
//     if (okButton.isPressed()) {
//         result = DIALOG_OK_PRESSED;
//         hideDialog();
//     }
//     else if (cancelButton.isPressed()) {
//         result = DIALOG_CANCEL_PRESSED;
//         hideDialog();
//     }
//     else {
//         result = DIALOG_NONE;
//     }
//     return result;
// }
DialogResult Dialog::getResult() {
    if (okButton.isPressed()) {
        Serial.println("OK Button Pressed!");
        hideDialog();
        return DIALOG_OK_PRESSED;
    }
    if (cancelButton.isPressed()) {
        Serial.println("Cancel Button Pressed!");
        hideDialog();
        return DIALOG_CANCEL_PRESSED;
    }
    return DIALOG_NONE;
}

void Dialog::hideDialog() {
    isVisible = false;
    M5.Lcd.fillRect(DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT, BLACK); // 消去
}

void Dialog::draw() {
    if (!isVisible) return;

    // 背景描画
    M5.Lcd.fillRect(DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT, WHITE);
    M5.Lcd.drawRect(DIALOG_X, DIALOG_Y, DIALOG_WIDTH, DIALOG_HEIGHT, DARKGREY);

    // タイトル
    M5.Lcd.setCursor(DIALOG_X + 10, DIALOG_Y + 10);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.print(title);

    // メッセージ
    M5.Lcd.setCursor(DIALOG_X + 10, DIALOG_Y + 40);
    M5.Lcd.setTextSize(1);
    M5.Lcd.print(message);

    // ボタン描画
    if (type == DIALOG_OK) {
        okButton.draw();
    }
    else if (type == DIALOG_OK_CANCEL) {
        okButton.draw();
        cancelButton.draw();
    }
}
