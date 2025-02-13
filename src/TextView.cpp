#include "TextView.h"

TextView::TextView()
    : posX(0), posY(0), width(320), height(240), fontSize(2), needsUpdate(false), backgroundColor(BLACK), color(WHITE) {}

void TextView::begin() {
    M5.Lcd.setTextSize(fontSize);

}

void TextView::setText(String text) {
    if (this->currentText != text) {
        this->currentText = text;
        wrapText(); // 自動改行
        this->needsUpdate = true;
    }
    draw();
}

void TextView::setPosition(int x, int y, int w, int h) {
    this->posX = x;
    this->posY = y;
    this->width = w;
    this->height = h;
    this->needsUpdate = true;
}

void TextView::setFontSize(int size) {
    this->fontSize = size;
    this->needsUpdate = true;
}

void TextView::setColor(uint16_t color) {
    this->color = color;
    this->needsUpdate = true;
}

void TextView::setBackgroundColor(uint16_t color) {
    this->backgroundColor = color;
    this->needsUpdate = true;
}

void TextView::clear() {
    M5.Lcd.fillRect(posX, posY, width, height, backgroundColor);
    this->currentText = "";
    this->wrappedText.clear();
    this->needsUpdate = false;
}

void TextView::draw() {
    if (needsUpdate) {
        M5.Lcd.fillRect(posX, posY, width, height, backgroundColor);
        M5.Lcd.setCursor(posX, posY);
        M5.Lcd.setTextSize(fontSize);

        // 改行済みテキストを順に描画
        int lineSpacing = fontSize * 8;
        int textY = posY;
        for (const auto& line : wrappedText) {
            M5.Lcd.setCursor(posX, textY);
            M5.Lcd.setTextColor(color, backgroundColor);
            M5.Lcd.printf("%s", line.c_str());
            textY += lineSpacing;
        }

        this->needsUpdate = false;
    }
}

// **テキストを指定幅で自動改行**
void TextView::wrapText() {
    this->wrappedText.clear();
    int charWidth = 6 * fontSize; // 1文字の幅（フォントサイズ依存）
    int maxChars = width / charWidth; // 1行の最大文字数

    String currentLine = "";
    String word = "";
    for (int i = 0; i < currentText.length(); i++) {
        char c = currentText[i];

        if (c == ' ' || c == '\n') {
            if (currentLine.length() + word.length() > maxChars) {
                wrappedText.push_back(currentLine);
                currentLine = word + " ";
            } else {
                currentLine += word + " ";
            }
            word = "";
        } else {
            word += c;
        }
    }
    if (!word.isEmpty()) {
        if (currentLine.length() + word.length() > maxChars) {
            wrappedText.push_back(currentLine);
            wrappedText.push_back(word);
        } else {
            wrappedText.push_back(currentLine + word);
        }
    }
}