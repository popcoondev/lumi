#ifndef BUTTON_H
#define BUTTON_H

#include <M5Unified.h>
#include "../system/FaceDetector.h"

#define BUTTON_TYPE_OUTLINE 0
#define BUTTON_TYPE_TEXT 1
#define BUTTON_TYPE_POLYGON 2

#define MAX_VERTICES 16  // 多角形ボタンの最大頂点数

class Button {
public:
    Button(int x, int y, int w, int h, const char* label);
    virtual void begin();
    virtual void draw();
    bool isPressed();
    void setLabel(const char* newLabel);
    void setColor(uint16_t normal, uint16_t pressed);
    void setFontSize(uint8_t size);
    void setType(uint8_t type) { this->type = type; }
    void setId(int id) { this->id = id; }
    int getId() const { return id; }
    virtual bool containsPoint(int x, int y) const;
    void setPressed(bool pressed);
    
protected:
    int id;
    int posX, posY, width, height;
    String label;
    uint16_t normalColor, pressedColor;
    unsigned long lastPressTime;
    bool wasPressed;
    uint8_t fontSize;
    uint8_t type;
};

// 多角形ボタンの基本クラス
class PolygonButton : public Button {
protected:
    // 頂点座標の配列
    int vertices[MAX_VERTICES][2];
    int vertexCount;
    
public:
    PolygonButton(const char* label = "");
    virtual void draw() override;
    virtual bool containsPoint(int x, int y) const override;
    
    // 頂点を設定するメソッド
    void setVertices(int vertices[][2], int count);
    
    // 点が多角形内に含まれるか判定する補助関数
    bool isPointInPolygon(int px, int py) const;
};

// 八角形の各面用の台形ボタン
class TrapezoidButton : public PolygonButton {
private:
    bool highlighted;
    bool focused;
    uint16_t highlightColor;
    uint16_t focusColor;
    uint16_t tempColor;
    bool hasTempColor;
    
    // FaceDetectorへの参照（色の取得用）
    FaceDetector* faceDetector;
    int faceId;  // 対応する面ID
    
public:
    TrapezoidButton(const char* label = "");
    
    // 台形の頂点を計算して設定
    void setTrapezoidVertices(int v0x, int v0y, int v1x, int v1y, int v2x, int v2y, int v3x, int v3y);
    
    // 状態設定メソッド
    void setHighlighted(bool highlighted);
    bool isHighlighted() const { return highlighted; }
    void setFocused(bool focused);
    bool isFocused() const { return focused; }
    void setHighlightColor(uint16_t color) { highlightColor = color; }
    uint16_t getHighlightColor() const { return highlightColor; }
    void setFocusColor(uint16_t color) { focusColor = color; }
    uint16_t getFocusColor() const { return focusColor; }
    void setTempColor(uint16_t color);
    void resetTempColor();
    void setFaceDetector(FaceDetector* detector) { faceDetector = detector; }
    void setFaceId(int id) { faceId = id; }
    int getFaceId() const { return faceId; }
    
    // 現在の色を取得
    uint16_t getCurrentColor() const;
    
    // 描画メソッドのオーバーライド
    virtual void draw() override;
};

// 中央ボタンクラス
class CenterButton : public Button {
private:
    String infoText;
    uint16_t infoColor;
    int radius;
    
public:
    CenterButton(const char* label = "");
    
    // 情報テキスト設定
    void setInfo(const String& text, uint16_t color);
    String getInfo() const { return infoText; }
    uint16_t getInfoColor() const { return infoColor; }
    void setRadius(int r) { radius = r; }
    int getRadius() const { return radius; }
    
    // 位置設定
    void setPosition(int x, int y, int w, int h);
    
    // 描画メソッドのオーバーライド
    virtual void draw() override;
    
    // 点が円内にあるか判定
    virtual bool containsPoint(int x, int y) const override;
};

#endif // BUTTON_H
