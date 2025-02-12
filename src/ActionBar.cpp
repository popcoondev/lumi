#include "ActionBar.h"
#include "Button.h"

ActionBar::ActionBar() 
    : title(""), status(""), backButtonWidth(ACTIONBAR_BACK_BUTTON_WIDTH), barHeight(ACTIONBAR_HEIGHT), textSize(ACTIONBAR_TEXT_SIZE), backPressed(false) {}

void ActionBar::begin() {
    M5.Lcd.fillRect(0, 0, M5.Lcd.width(), barHeight, NAVY);  // 背景色
    backButton.begin();
    draw();
}

void ActionBar::setTitle(const char* newTitle) {
    title = newTitle;
    draw();
}

void ActionBar::setStatus(const char* newStatus) {
    status = newStatus;
    draw();
}

void ActionBar::draw() {
    // 背景再描画
    M5.Lcd.fillRect(0, 0, M5.Lcd.width(), barHeight, NAVY);
    
    // 戻るボタン
    backButton.setColor(RED, MAROON);
    backButton.draw();

    // タイトル
    M5.Lcd.setCursor(backButtonWidth + 10, 8);
    M5.Lcd.setTextSize(textSize);
    M5.Lcd.setTextColor(WHITE, NAVY);
    M5.Lcd.print(title);

    // ステータス
    M5.Lcd.setCursor(M5.Lcd.width() - 100, 8);
    M5.Lcd.setTextSize(textSize);
    M5.Lcd.setTextColor(WHITE, NAVY);
    M5.Lcd.print(status);
}

bool ActionBar::isBackPressed() {
    backPressed = backButton.isPressed();
    return backPressed;
}
