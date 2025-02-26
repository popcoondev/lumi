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
    , rotationAngle(0.0f)
{
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

// 内部で頂点座標を計算（モデル座標系：中心が原点）
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

// 表示領域設定
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

// 中心点を軸に回転（角度はラジアンで加算）
void OctagonRingView::rotate(float dAngle) {
    rotationAngle += dAngle;
}

// 描画
void OctagonRingView::draw() {
    // 背景クリア
    M5.Lcd.fillRect(viewX, viewY, viewWidth, viewHeight, backgroundColor);

    // まず、モデル座標系の各頂点に対して「回転」を適用する
    // ※各頂点は vertices[i] にあるので、回転後の座標を仮の配列に保存
    float rotated[NUM_VERTICES][2];
    for(int i = 0; i < NUM_VERTICES; i++){
        float x = vertices[i][0];
        float y = vertices[i][1];
        rotated[i][0] = x * cosf(rotationAngle) - y * sinf(rotationAngle);
        rotated[i][1] = x * sinf(rotationAngle) + y * cosf(rotationAngle);
    }

    // 次に、rotated 配列のバウンディングボックスを求める
    float minX = rotated[0][0], maxX = rotated[0][0];
    float minY = rotated[0][1], maxY = rotated[0][1];
    for(int i = 1; i < NUM_VERTICES; i++){
        if(rotated[i][0] < minX) minX = rotated[i][0];
        if(rotated[i][0] > maxX) maxX = rotated[i][0];
        if(rotated[i][1] < minY) minY = rotated[i][1];
        if(rotated[i][1] > maxY) maxY = rotated[i][1];
    }
    float wModel = maxX - minX;
    float hModel = maxY - minY;

    // 表示領域にフィットさせるスケール（余白を考慮して0.9倍）
    float scale = 1.0f;
    if(wModel > 0 && hModel > 0) {
        float scaleX = float(viewWidth) / wModel;
        float scaleY = float(viewHeight) / hModel;
        scale = std::min(scaleX, scaleY) * 0.9f;
    }

    // 表示領域中心
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;

    // 各回転済み頂点を、スケール・シフトして projected 配列に設定
    for(int i = 0; i < NUM_VERTICES; i++){
        float x = rotated[i][0] - (minX + wModel * 0.5f);
        float y = rotated[i][1] - (minY + hModel * 0.5f);
        x *= scale;
        y *= scale;
        projected[i][0] = centerX + int(x);
        // y軸反転（画面座標）
        projected[i][1] = centerY - int(y);
    }

    // 台形（各面）描画
    for(int i = 0; i < NUM_FACES; i++){
        int v0 = faces[i][0];
        int v1 = faces[i][1];
        int v2 = faces[i][2];
        int v3 = faces[i][3];

        // デフォルト色（例：赤系）
        uint16_t color = M5.Lcd.color565(200, 80, 80);
        if(i == highlightedFace) {
            color = M5.Lcd.color565(0, 255, 0);
        }

        // 台形を三角形2枚に分割して塗りつぶす
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
        int a = edges[i][0];
        int b = edges[i][1];
        M5.Lcd.drawLine(
            projected[a][0], projected[a][1],
            projected[b][0], projected[b][1],
            TFT_WHITE
        );
    }
}
