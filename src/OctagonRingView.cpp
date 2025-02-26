#include "OctagonRingView.h"
#include <cmath>
#include <algorithm>

// コンストラクタ
OctagonRingView::OctagonRingView()
    : outerRadius(1.0f)
    , innerRadius(0.5f)
    , viewX(0)
    , viewY(0)
    , viewWidth(100)
    , viewHeight(100)
    , backgroundColor(BLACK)
    , highlightedFace(-1)
{
    // 頂点座標の初期化
    updateGeometry();

    // 面(台形)の定義：外側i, 外側(i+1), 内側(i+1), 内側i
    for(int i = 0; i < NUM_FACES; i++) {
        int iNext = (i + 1) % NUM_OUTER;
        faces[i][0] = i;                  // outer i
        faces[i][1] = iNext;             // outer i+1
        faces[i][2] = NUM_OUTER + iNext; // inner i+1
        faces[i][3] = NUM_OUTER + i;     // inner i
    }

    // エッジ(24本)の定義
    // 1) 外側リング8本
    // 2) 内側リング8本
    // 3) 外→内のラジアル8本
    int idx = 0;
    // 外側リング
    for(int i = 0; i < NUM_OUTER; i++){
        int iNext = (i + 1) % NUM_OUTER;
        edges[idx][0] = i;
        edges[idx][1] = iNext;
        idx++;
    }
    // 内側リング
    for(int i = 0; i < NUM_INNER; i++){
        int iNext = (i + 1) % NUM_INNER;
        edges[idx][0] = NUM_OUTER + i;
        edges[idx][1] = NUM_OUTER + iNext;
        idx++;
    }
    // ラジアル
    for(int i = 0; i < NUM_OUTER; i++){
        edges[idx][0] = i;
        edges[idx][1] = NUM_OUTER + i;
        idx++;
    }
}

// 外側・内側の頂点座標を計算
void OctagonRingView::updateGeometry() {
    // 外側8頂点
    for(int i = 0; i < NUM_OUTER; i++){
        float theta = 2.0f * M_PI * float(i) / float(NUM_OUTER);
        vertices[i][0] = outerRadius * cosf(theta);
        vertices[i][1] = outerRadius * sinf(theta);
    }
    // 内側8頂点
    for(int i = 0; i < NUM_INNER; i++){
        float theta = 2.0f * M_PI * float(i) / float(NUM_INNER);
        vertices[NUM_OUTER + i][0] = innerRadius * cosf(theta);
        vertices[NUM_OUTER + i][1] = innerRadius * sinf(theta);
    }
}

void OctagonRingView::setViewPosition(int x, int y, int w, int h) {
    viewX = x;
    viewY = y;
    viewWidth = w;
    viewHeight = h;
}

void OctagonRingView::setBackgroundColor(uint16_t color) {
    backgroundColor = color;
}

void OctagonRingView::setHighlightedFace(int faceID) {
    highlightedFace = faceID;
}

void OctagonRingView::draw() {
    // 背景クリア
    M5.Lcd.fillRect(viewX, viewY, viewWidth, viewHeight, backgroundColor);

    // 頂点のバウンディングボックスを求める
    float minX = vertices[0][0];
    float maxX = vertices[0][0];
    float minY = vertices[0][1];
    float maxY = vertices[0][1];
    for(int i = 1; i < NUM_VERTICES; i++){
        float x = vertices[i][0];
        float y = vertices[i][1];
        if(x < minX) minX = x;
        if(x > maxX) maxX = x;
        if(y < minY) minY = y;
        if(y > maxY) maxY = y;
    }

    // 幅・高さ
    float w = maxX - minX;
    float h = maxY - minY;

    // 表示領域にフィットさせるスケールを計算 (余白を少し持たせるため0.9倍)
    float scale = 1.0f;
    if(w > 0 && h > 0) {
        float scaleX = float(viewWidth)  / w;
        float scaleY = float(viewHeight) / h;
        scale = std::min(scaleX, scaleY) * 0.9f;
    }

    // 表示領域の中心座標
    float centerX = viewX + viewWidth  * 0.5f;
    float centerY = viewY + viewHeight * 0.5f;

    // 頂点を投影（2Dなのでシフト＆スケーリングのみ）
    for(int i = 0; i < NUM_VERTICES; i++){
        float x = vertices[i][0] - (minX + w * 0.5f);
        float y = vertices[i][1] - (minY + h * 0.5f);
        x *= scale;
        y *= scale;
        projected[i][0] = int(centerX + x);
        // y軸は画面座標で下向きなので符号を反転
        projected[i][1] = int(centerY - y);
    }

    // 台形を描画 (4頂点を fillTriangle x2 で塗りつぶす)
    for(int i = 0; i < NUM_FACES; i++){
        int v0 = faces[i][0];
        int v1 = faces[i][1];
        int v2 = faces[i][2];
        int v3 = faces[i][3];

        // デフォルト色（赤系）
        uint16_t color = M5.Lcd.color565(200, 80, 80);

        // ハイライト面なら色を変更
        if(i == highlightedFace){
            color = M5.Lcd.color565(0, 255, 0);
        }

        // trapezoid = (v0, v1, v2, v3)
        // fillTriangle: (v0,v1,v2) & (v0,v2,v3)
        M5.Lcd.fillTriangle(
            projected[v0][0], projected[v0][1],
            projected[v1][0], projected[v1][1],
            projected[v2][0], projected[v2][1],
            color
        );
        M5.Lcd.fillTriangle(
            projected[v0][0], projected[v0][1],
            projected[v2][0], projected[v2][1],
            projected[v3][0], projected[v3][1],
            color
        );
    }

    // エッジ描画
    for(int i = 0; i < NUM_EDGES; i++){
        int vA = edges[i][0];
        int vB = edges[i][1];
        M5.Lcd.drawLine(
            projected[vA][0], projected[vA][1],
            projected[vB][0], projected[vB][1],
            TFT_WHITE
        );
    }
}
