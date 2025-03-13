#include "Button.h"
#include <cmath>

Button::Button(int x, int y, int w, int h, const char* label)
    : posX(x), posY(y), width(w), height(h), label(label),
      normalColor(DARKGREY), pressedColor(LIGHTGREY), lastPressTime(0), wasPressed(false), fontSize(2), type(BUTTON_TYPE_OUTLINE), id(0) {}

void Button::begin() {
    draw();
}

void Button::draw() {
    if (type == BUTTON_TYPE_OUTLINE) {
        if (!wasPressed) {
            M5.Lcd.fillRoundRect(posX, posY, width, height, 5, normalColor);
            M5.Lcd.drawRoundRect(posX, posY, width, height, 5, LIGHTGREY);
            M5.Lcd.setTextColor(WHITE, normalColor);
        }
        else {
            M5.Lcd.fillRoundRect(posX+1, posY+1, width-1, height-1, 5, pressedColor);
            M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-1, 5, LIGHTGREY);
            M5.Lcd.setTextColor(WHITE, pressedColor);
        }
    } else if (type == BUTTON_TYPE_TEXT) {
        if (!wasPressed) {
            M5.Lcd.fillRoundRect(posX+1, posY+1, width-1, height-1, 5, BLACK);
            M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-1, 5, BLACK);
            M5.Lcd.setTextColor(WHITE, BLACK);
        }
        else {
            M5.Lcd.fillRoundRect(posX+1, posY+1, width-1, height-1, 5, BLACK);
            M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-1, 5, WHITE);
            M5.Lcd.setTextColor(WHITE, BLACK);
        }
    }

    M5.Lcd.setTextSize(fontSize);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString(label, posX + width / 2, posY + height / 2);
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

void Button::setFontSize(uint8_t size) {
    fontSize = size;
    draw();
}

bool Button::isPressed() {
    // M5.update();
    // if (M5.Touch.getCount() > 0) {
    //     auto touch = M5.Touch.getDetail();
    //     if (touch.x >= posX && touch.x <= posX + width &&
    //         touch.y >= posY && touch.y <= posY + height) {

    //         // チャタリング防止（100ms 以内の連続タップを無視）
    //         if (millis() - lastPressTime < 100) {
    //             return false;
    //         }
    //         lastPressTime = millis();
    //         wasPressed = true;
    //         if(type == BUTTON_TYPE_OUTLINE) {
    //             M5.Lcd.fillRoundRect(posX+1, posY+1, width-1, height-1, 5, pressedColor);
    //             M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-1, 5, WHITE);
    //             M5.Lcd.setTextColor(normalColor, pressedColor);
    //         }
    //         else {
    //             // type == BUTTON_TYPE_TEXTの場合は、テキストの色を反転
    //             M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-6, 5, WHITE);
    //             M5.Lcd.setTextColor(pressedColor, normalColor);
                
    //         }
    //         return false;  // 押した直後は無効
    //     }
    // } 

    // // リリース後に "押された" 状態を確定
    // if (wasPressed) {
    //     wasPressed = false;
    //     draw();
    //     return true;
    // }

    return wasPressed;
}

bool Button::containsPoint(int x, int y) const {
    return (x >= posX && x < posX + width && y >= posY && y < posY + height);
}

void Button::setPressed(bool pressed) {
    wasPressed = pressed;
    // // 見た目の更新処理は必要に応じて
    // if(type == BUTTON_TYPE_OUTLINE) {
    //     M5.Lcd.fillRoundRect(posX+1, posY+1, width-1, height-1, 5, pressedColor);
    //     M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-1, 5, WHITE);
    //     M5.Lcd.setTextColor(normalColor, pressedColor);
    // }
    // else {
    //     // type == BUTTON_TYPE_TEXTの場合は、テキストの色を反転
    //     M5.Lcd.drawRoundRect(posX+1, posY+1, width-1, height-6, 5, WHITE);
    //     M5.Lcd.setTextColor(pressedColor, normalColor);
        
    // }
}

// PolygonButtonの実装
PolygonButton::PolygonButton(const char* label)
    : Button(0, 0, 0, 0, label), vertexCount(0) {
    type = BUTTON_TYPE_POLYGON;
}

void PolygonButton::setVertices(int verts[][2], int count) {
    vertexCount = min(count, MAX_VERTICES);
    for (int i = 0; i < vertexCount; i++) {
        vertices[i][0] = verts[i][0];
        vertices[i][1] = verts[i][1];
    }
    
    // バウンディングボックスを計算して基底クラスのposX, posY, width, heightを設定
    if (vertexCount > 0) {
        int minX = vertices[0][0], maxX = vertices[0][0];
        int minY = vertices[0][1], maxY = vertices[0][1];
        
        for (int i = 1; i < vertexCount; i++) {
            if (vertices[i][0] < minX) minX = vertices[i][0];
            if (vertices[i][0] > maxX) maxX = vertices[i][0];
            if (vertices[i][1] < minY) minY = vertices[i][1];
            if (vertices[i][1] > maxY) maxY = vertices[i][1];
        }
        
        posX = minX;
        posY = minY;
        width = maxX - minX;
        height = maxY - minY;
    }
}

void PolygonButton::draw() {
    if (vertexCount < 3) return; // 少なくとも3頂点が必要
    
    // 多角形を描画（三角形に分割して描画）
    uint16_t fillColor = wasPressed ? pressedColor : normalColor;
    
    // 最初の頂点を基準に三角形分割
    for (int i = 1; i < vertexCount - 1; i++) {
        M5.Lcd.fillTriangle(
            vertices[0][0], vertices[0][1],
            vertices[i][0], vertices[i][1],
            vertices[i+1][0], vertices[i+1][1],
            fillColor
        );
    }
    
    // 輪郭線を描画
    uint16_t edgeColor = LIGHTGREY;
    for (int i = 0; i < vertexCount; i++) {
        int next = (i + 1) % vertexCount;
        M5.Lcd.drawLine(
            vertices[i][0], vertices[i][1],
            vertices[next][0], vertices[next][1],
            edgeColor
        );
    }
    
    // ラベルを描画（多角形の中心に）
    if (label.length() > 0) {
        int centerX = 0, centerY = 0;
        for (int i = 0; i < vertexCount; i++) {
            centerX += vertices[i][0];
            centerY += vertices[i][1];
        }
        centerX /= vertexCount;
        centerY /= vertexCount;
        
        M5.Lcd.setTextSize(fontSize);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextColor(WHITE, fillColor);
        M5.Lcd.drawString(label, centerX, centerY);
    }
}

bool PolygonButton::containsPoint(int x, int y) const {
    return isPointInPolygon(x, y);
}

bool PolygonButton::isPointInPolygon(int px, int py) const {
    if (vertexCount < 3) return false;
    
    bool inside = false;
    for (int i = 0, j = vertexCount - 1; i < vertexCount; j = i++) {
        if (((vertices[i][1] > py) != (vertices[j][1] > py)) &&
            (px < (vertices[j][0] - vertices[i][0]) * (py - vertices[i][1]) / (vertices[j][1] - vertices[i][1]) + vertices[i][0])) {
            inside = !inside;
        }
    }
    return inside;
}

// TrapezoidButtonの実装
TrapezoidButton::TrapezoidButton(const char* label)
    : PolygonButton(label), highlighted(false), focused(false),
      highlightColor(WHITE), focusColor(TFT_YELLOW),
      tempColor(0), hasTempColor(false),
      faceDetector(nullptr), faceId(-1) {
}

void TrapezoidButton::setTrapezoidVertices(int v0x, int v0y, int v1x, int v1y, int v2x, int v2y, int v3x, int v3y) {
    int verts[4][2] = {
        {v0x, v0y}, {v1x, v1y}, {v2x, v2y}, {v3x, v3y}
    };
    setVertices(verts, 4);
}

void TrapezoidButton::setHighlighted(bool highlight) {
    highlighted = highlight;
}

void TrapezoidButton::setFocused(bool focus) {
    focused = focus;
}

void TrapezoidButton::setTempColor(uint16_t color) {
    tempColor = color;
    hasTempColor = true;
}

void TrapezoidButton::resetTempColor() {
    hasTempColor = false;
}

uint16_t TrapezoidButton::getCurrentColor() const {
    // 色の優先順位: 一時的な色 > フォーカス色 > ハイライト色 > FaceDetectorの色 > 通常色
    if (hasTempColor) {
        return tempColor;
    }
    else if (focused) {
        return focusColor;
    }
    else if (highlighted) {
        return highlightColor;
    }
    else if (faceDetector != nullptr && faceDetector->getCalibratedFacesCount() > 0 && faceId >= 0) {
        FaceData* faceList = faceDetector->getFaceList();
        if (faceId < faceDetector->getCalibratedFacesCount() && faceList[faceId].ledState == 1) {
            CRGB ledColor = faceList[faceId].ledColor;
            return M5.Lcd.color565(ledColor.r, ledColor.g, ledColor.b);
        }
    }
    
    return normalColor;
}

void TrapezoidButton::draw() {
    if (vertexCount != 4) return; // 台形は4頂点
    
    // 現在の色を取得
    uint16_t fillColor = getCurrentColor();
    
    // 台形を描画（三角形2つに分けて描画）
    M5.Lcd.fillTriangle(
        vertices[0][0], vertices[0][1],
        vertices[1][0], vertices[1][1],
        vertices[2][0], vertices[2][1],
        fillColor
    );
    M5.Lcd.fillTriangle(
        vertices[0][0], vertices[0][1],
        vertices[2][0], vertices[2][1],
        vertices[3][0], vertices[3][1],
        fillColor
    );
    
    // エッジの色を決定
    uint16_t edgeColor = TFT_WHITE;
    
    // 輪郭線を描画
    M5.Lcd.drawLine(
        vertices[0][0], vertices[0][1],
        vertices[1][0], vertices[1][1],
        edgeColor
    );
    M5.Lcd.drawLine(
        vertices[1][0], vertices[1][1],
        vertices[2][0], vertices[2][1],
        edgeColor
    );
    M5.Lcd.drawLine(
        vertices[2][0], vertices[2][1],
        vertices[3][0], vertices[3][1],
        edgeColor
    );
    M5.Lcd.drawLine(
        vertices[3][0], vertices[3][1],
        vertices[0][0], vertices[0][1],
        edgeColor
    );
    
    // ラベルを描画（必要に応じて）
    if (label.length() > 0) {
        int centerX = (vertices[0][0] + vertices[1][0] + vertices[2][0] + vertices[3][0]) / 4;
        int centerY = (vertices[0][1] + vertices[1][1] + vertices[2][1] + vertices[3][1]) / 4;
        
        M5.Lcd.setTextSize(fontSize);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextColor(WHITE, fillColor);
        M5.Lcd.drawString(label, centerX, centerY);
    }
}

// CenterButtonの実装
CenterButton::CenterButton(const char* label)
    : Button(0, 0, 0, 0, label), infoText(""), infoColor(WHITE), radius(30) {
}

void CenterButton::setInfo(const String& text, uint16_t color) {
    infoText = text;
    infoColor = color;
}

void CenterButton::setPosition(int x, int y, int w, int h) {
    posX = x;
    posY = y;
    width = w;
    height = h;
}

void CenterButton::draw() {
    // 中央円を描画
    M5.Lcd.fillCircle(posX + width/2, posY + height/2, radius, normalColor);
    
    // 中央円の輪郭を描画（オプション）
    // M5.Lcd.drawCircle(posX + width/2, posY + height/2, radius, TFT_WHITE);
    
    // 情報テキストを表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(infoColor, normalColor);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.drawString(infoText, posX + width/2, posY + height/2);
}

bool CenterButton::containsPoint(int x, int y) const {
    int centerX = posX + width/2;
    int centerY = posY + height/2;
    int dx = x - centerX;
    int dy = y - centerY;
    return (dx*dx + dy*dy) <= (radius*radius);
}
