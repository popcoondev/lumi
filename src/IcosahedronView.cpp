#include "IcosahedronView.h"
#include <cmath>
#include <cstring>  // for memcpy
#include <algorithm> // for std::sort

// コンストラクタ：頂点、エッジ、面リストの初期化
IcosahedronView::IcosahedronView() 
    : angleX(0), angleY(0), viewX(0), viewY(0), viewWidth(320), viewHeight(240), vertices(), edges(), faces(), depth(), backgrountColor(BLACK), highlightedFace(-1)

{
    const float PHI = (1.0f + sqrt(5.0f)) / 2.0f;

    // 12個の頂点（Python版と同じ順番）
    float v[12][3] = {
        {-1,  PHI,  0}, { 1,  PHI,  0}, {-1, -PHI,  0}, { 1, -PHI,  0},
        { 0, -1,  PHI}, { 0,  1,  PHI}, { 0, -1, -PHI}, { 0,  1, -PHI},
        { PHI,  0, -1}, { PHI,  0,  1}, {-PHI,  0, -1}, {-PHI,  0,  1}
    };
    memcpy(vertices, v, sizeof(v));

    // 30本のエッジ（重複なし、各エッジは [小,大] の順）
    int e[30][2] = {
        {0, 11}, {5, 11}, {0, 5},
        {0, 1},  {1, 5},  {1, 7},
        {0, 7},  {0, 10}, {7, 10},
        {10,11}, {1, 9},  {5, 9},
        {4, 11}, {4, 5},  {2, 10},
        {2, 11}, {6, 7},  {6, 10},
        {1, 8},  {7, 8},  {3, 9},
        {3, 4},  {4, 9},  {2, 4},
        {2, 3},  {2, 6},  {3, 6},
        {3, 8},  {6, 8},  {8, 9}
    };
    memcpy(edges, e, sizeof(e));

    // 20個の面（各面は頂点インデックス3つの三角形）
    int f[20][3] = {
        {0, 11, 5},
        {0, 5, 1},
        {0, 1, 7},
        {0, 7, 10},
        {0, 10, 11},
        {1, 5, 9},
        {5, 11, 4},
        {11, 10, 2},
        {10, 7, 6},
        {7, 1, 8},
        {3, 9, 4},
        {3, 4, 2},
        {3, 2, 6},
        {3, 6, 8},
        {3, 8, 9},
        {4, 9, 5},
        {2, 4, 11},
        {6, 2, 10},
        {8, 6, 7},
        {9, 8, 1}
    };
    memcpy(faces, f, sizeof(f));
}

// 描画領域の位置とサイズを設定
void IcosahedronView::setViewPosition(int x, int y, int w, int h) {
    viewX = x;
    viewY = y;
    viewWidth = w;
    viewHeight = h;
}

// 3D→2D 投影変換
void IcosahedronView::project3D(float x, float y, float z, int &px, int &py) {
    // 描画領域の中心座標を計算
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;
    // z軸のシフトで遠近感を調整（z + 3.0f でゼロ除算を回避）
    float factor = 1.5f / (z + 3.0f);
    // ※ここでは仮の SCALE 定数を利用（後で描画全体のスケール調整を行います）
    px = centerX + static_cast<int>(x * SCALE * factor);
    py = centerY - static_cast<int>(y * SCALE * factor);
}

// 3D回転：X軸回り、続いてY軸回りに回転（角度はラジアン）
void IcosahedronView::rotate3D(float &x, float &y, float &z, float angleX, float angleY) {
    // X軸周りの回転
    float tempY = y * cos(angleX) - z * sin(angleX);
    float tempZ = y * sin(angleX) + z * cos(angleX);
    y = tempY; 
    z = tempZ;

    // Y軸周りの回転
    float tempX = x * cos(angleY) + z * sin(angleY);
    tempZ = -x * sin(angleY) + z * cos(angleY);
    x = tempX; 
    z = tempZ;
}

void IcosahedronView::setBackgroundColor(uint16_t color) {
    backgrountColor = color;
}

// 描画：全頂点を回転・投影し、面とエッジを描画（面は奥行き順にソート）
void IcosahedronView::draw() {
    // 描画領域を背景色でクリア
    M5.Lcd.fillRect(viewX, viewY, viewWidth, viewHeight, backgrountColor);

    // 各頂点を回転・投影し、 projected 配列に保存
    // ※同時に z 値も depth[] に保存（面の奥行きソート用）
    for (int i = 0; i < 12; i++) {
        float x = vertices[i][0];
        float y = vertices[i][1];
        float z = vertices[i][2];

        rotate3D(x, y, z, angleX, angleY);
        project3D(x, y, z, projected[i][0], projected[i][1]);
        depth[i] = z;
    }

    // ----- ここから、描画領域にフィットするように再スケーリング -----
    // 現在の projected 配列のバウンディングボックスを求める
    int minX = projected[0][0], maxX = projected[0][0];
    int minY = projected[0][1], maxY = projected[0][1];
    for (int i = 1; i < 12; i++) {
        if (projected[i][0] < minX) minX = projected[i][0];
        if (projected[i][0] > maxX) maxX = projected[i][0];
        if (projected[i][1] < minY) minY = projected[i][1];
        if (projected[i][1] > maxY) maxY = projected[i][1];
    }
    int modelWidth = maxX - minX;
    int modelHeight = maxY - minY;
    // 描画領域の中心
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;
    // 横・縦のスケール係数を計算し、余裕を持たせるために 90% 程度にする
    float scaleFactor = 1.0f;
    if (modelWidth > 0 && modelHeight > 0) {
        float sX = static_cast<float>(viewWidth) / modelWidth;
        float sY = static_cast<float>(viewHeight) / modelHeight;
        scaleFactor = std::min(sX, sY) * 0.9f;
    }
    // 各 projected 座標を中心基準に再スケーリング
    for (int i = 0; i < 12; i++) {
        projected[i][0] = centerX + static_cast<int>((projected[i][0] - centerX) * scaleFactor);
        projected[i][1] = centerY + static_cast<int>((projected[i][1] - centerY) * scaleFactor);
    }
    // ----- 再スケーリング終了 -----

    // 面を奥行き順にソート
    int sortedFaces[20];
    for (int i = 0; i < 20; i++) sortedFaces[i] = i;
    std::sort(sortedFaces, sortedFaces + 20, [&](int a, int b) {
        float za = (depth[faces[a][0]] + depth[faces[a][1]] + depth[faces[a][2]]) / 3.0f;
        float zb = (depth[faces[b][0]] + depth[faces[b][1]] + depth[faces[b][2]]) / 3.0f;
        return za > zb;  // 手前の面から描画（透明度などの関係であれば後描画に変更も検討）
    });

    // 面の描画（各面は簡易ライティング付きで塗りつぶし）
    for (int i = 0; i < 20; i++) {
        int f = sortedFaces[i];
        int v0 = faces[f][0];
        int v1 = faces[f][1];
        int v2 = faces[f][2];

        // 法線計算（簡易ライティング用）
        float ax = vertices[v1][0] - vertices[v0][0];
        float ay = vertices[v1][1] - vertices[v0][1];
        float az = vertices[v1][2] - vertices[v0][2];

        float bx = vertices[v2][0] - vertices[v0][0];
        float by = vertices[v2][1] - vertices[v0][1];
        float bz = vertices[v2][2] - vertices[v0][2];

        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        // 上方からの光と仮定して明るさを計算
        float light = (ny + 1.0f) / 2.0f;  // 0～1に正規化
        uint16_t color = BLACK; //M5.Lcd.color565(255 * light, 100 * light, 100 * light);

        // ハイライト対象の場合は、色を上書き（例：緑色）
        if (f == highlightedFace) {
            color = M5.Lcd.color565(0, 255, 0);
        }

        M5.Lcd.fillTriangle(
            projected[v0][0], projected[v0][1],
            projected[v1][0], projected[v1][1],
            projected[v2][0], projected[v2][1],
            color
        );
    }


    // エッジ（輪郭）の描画
    for (int i = 0; i < 30; i++) {
        int v1 = edges[i][0];
        int v2 = edges[i][1];
        M5.Lcd.drawLine(
            projected[v1][0], projected[v1][1],
            projected[v2][0], projected[v2][1],
            TFT_WHITE
        );
    }
}

// 回転角度の更新
void IcosahedronView::rotate(float dAngleX, float dAngleY) {
    angleX += dAngleX;
    angleY += dAngleY;
}

void IcosahedronView::setHighlightedFace(int faceID) {
    highlightedFace = faceID;
}
