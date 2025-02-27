#ifndef ICOSAHEDRON_VIEW_H
#define ICOSAHEDRON_VIEW_H

#include <M5CoreS3.h>

class IcosahedronView {
private:
    static constexpr float PHI = 1.618; // 黄金比
    static constexpr int CENTER_X = 160;
    static constexpr int CENTER_Y = 120;
    static constexpr int SCALE = 50;

    int viewX = 0;
    int viewY = 0;
    int viewWidth = 320;
    int viewHeight = 240;

    float vertices[12][3];
    int edges[30][2];
    int faces[20][3];
    float depth[12];

    float angleX, angleY;  // 回転角度
    int projected[12][2];  // 2D投影座標
    int backgrountColor;
    int highlightedFace;

    void project3D(float x, float y, float z, int &px, int &py);
    void rotate3D(float &x, float &y, float &z, float angleX, float angleY);

public:
    IcosahedronView(); // コンストラクタ
    void setViewPosition(int x, int y, int w, int h);
    void draw();
    void rotate(float dAngleX, float dAngleY);
    void setBackgroundColor(uint16_t color);
    void setHighlightedFace(int faceID);

};

#endif // ICOSAHEDRON_VIEW_H
