#include "Dialog.h"
#include "TextView.h"

Dialog::Dialog() : isVisible(false), result(DIALOG_NONE) {}

void Dialog::showDialog(const String& titleText, const String& messageText, DialogType dialogType) {
    title = titleText;
    message = messageText;
    type = dialogType;
    isVisible = true;
    result = DIALOG_NONE;
    draw();
}

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
    TextView titleView;
    titleView.setPosition(DIALOG_X + 10, DIALOG_Y + 10, DIALOG_WIDTH - 20, 30);
    titleView.setFontSize(2);
    titleView.setBackgroundColor(WHITE);
    titleView.setColor(BLACK);
    titleView.setText(title);
    titleView.draw();

    // メッセージ
    TextView messageView;
    messageView.setPosition(DIALOG_X + 10, DIALOG_Y + 40, DIALOG_WIDTH - 20, 100);
    messageView.setFontSize(1);
    messageView.setBackgroundColor(WHITE);
    messageView.setColor(BLACK);
    messageView.setText(message);
    messageView.draw();

    // ボタン描画
    if (type == DIALOG_OK) {
        okButton.draw();
    }
    else if (type == DIALOG_OK_CANCEL) {
        okButton.draw();
        cancelButton.draw();
    }
}
