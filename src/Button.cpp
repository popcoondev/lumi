#include "Button.h"

Button::Button(int x, int y, int w, int h, const char* label)
    : posX(x), posY(y), width(w), height(h), label(label),
      normalColor(DARKGREY), pressedColor(LIGHTGREY), lastPressTime(0), wasPressed(false) {}

void Button::begin() {
    draw();
}

void Button::draw() {
    M5.Lcd.fillRoundRect(posX, posY, width, height, 5, normalColor);
    M5.Lcd.drawRoundRect(posX, posY, width, height, 5, WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE, normalColor);
    M5.Lcd.setCursor(posX + 10, posY + 8);
    M5.Lcd.print(label);
}

void Button::setLabel(const char* newLabel) {
    label = newLabel;
    draw();
}

void Button::setColor(uint16_t normal, uint16_t pressed) {
    normalColor = normal;
    pressedColor = pressed;
    draw();
}

bool Button::isPressed() {
    M5.update();
    if (M5.Touch.getCount() > 0) {
        auto touch = M5.Touch.getDetail();
        if (touch.x >= posX && touch.x <= posX + width &&
            touch.y >= posY && touch.y <= posY + height) {

            // チャタリング防止（200ms 以内の連続タップを無視）
            if (millis() - lastPressTime < 200) {
                return false;
            }
            lastPressTime = millis();
            wasPressed = true;
            M5.Lcd.fillRoundRect(posX+1, posY+1, width-1, height-1, 5, pressedColor);
            M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-1, 5, WHITE);
            M5.Lcd.setTextColor(normalColor, pressedColor);
            return false;  // 押した直後は無効
        }
    } 

    // リリース後に "押された" 状態を確定
    if (wasPressed) {
        wasPressed = false;
        draw();
        return true;
    }

    return false;
}
