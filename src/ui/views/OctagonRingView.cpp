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
    , highlightColor(WHITE)     // デフォルトのハイライト色は白
    , rotationAngle(M_PI/8.0f)  // デフォルトでPI/8（22.5度）回転
    , isMirrored(true) // デフォルトで鏡写しに設定
    , faceDetector(nullptr) // FaceDetectorは初期化時にnull
    , tempFaceColors()
    , hasTempColor() 
{
    // ハイライト面の初期化（すべて非ハイライト）
    for (int i = 0; i < NUM_FACES; i++) {
        highlightedFaces[i] = false;
    }

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

    for (int i = 0; i < NUM_FACES; i++) {
        tempFaceColors[i] = TFT_BLACK;
        hasTempColor[i] = false;
    }
}

void OctagonRingView::drawFace(int faceId) {
    if (faceId < 0 || faceId >= NUM_FACES) return;
    
    int v0 = faces[faceId][0];
    int v1 = faces[faceId][1];
    int v2 = faces[faceId][2];
    int v3 = faces[faceId][3];
    
    // 面の色を判定
    uint16_t color = backgroundColor;
    
    // 一時的な色があればそれを使用
    if (hasTempColor[faceId]) {
        color = tempFaceColors[faceId];
    }
    // 通常のハイライト状態
    else if (highlightedFaces[faceId]) {
        color = highlightColor;
    }
    // FaceDetectorによる色
    else if (faceDetector != nullptr && faceDetector->getCalibratedFacesCount() > 0) {
        FaceData* faceList = faceDetector->getFaceList();
        if (faceId < faceDetector->getCalibratedFacesCount() && faceList[faceId].ledState == 1) {
            CRGB ledColor = faceList[faceId].ledColor;
            color = M5.Lcd.color565(ledColor.r, ledColor.g, ledColor.b);
        }
    }
    
    // 台形を描画（三角形2つに分けて描画）
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
    
    // エッジを描画（面ごとに関連するエッジのみ）
    M5.Lcd.drawLine(
        projected[v0][0], projected[v0][1],
        projected[v1][0], projected[v1][1],
        TFT_WHITE
    );
    M5.Lcd.drawLine(
        projected[v1][0], projected[v1][1],
        projected[v2][0], projected[v2][1],
        TFT_WHITE
    );
    M5.Lcd.drawLine(
        projected[v2][0], projected[v2][1],
        projected[v3][0], projected[v3][1],
        TFT_WHITE
    );
    M5.Lcd.drawLine(
        projected[v3][0], projected[v3][1],
        projected[v0][0], projected[v0][1],
        TFT_WHITE
    );
}

// 中央部分のみ再描画
void OctagonRingView::drawCenter() {
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;
    float innerRadius = min(viewWidth, viewHeight) * 0.2f;
    
    // 中央円を背景色で描画
    M5.Lcd.fillCircle(centerX, centerY, innerRadius, backgroundColor);
    
    // 中央円の輪郭を描画
    M5.Lcd.drawCircle(centerX, centerY, innerRadius, TFT_WHITE);
}

// 一時的な色を設定
void OctagonRingView::setFaceTempColor(int faceId, uint16_t color) {
    if (faceId >= 0 && faceId < NUM_FACES) {
        tempFaceColors[faceId] = color;
        hasTempColor[faceId] = true;
    }
}

// 一時的な色を解除
void OctagonRingView::resetFaceTempColor(int faceId) {
    if (faceId >= 0 && faceId < NUM_FACES) {
        hasTempColor[faceId] = false;
    }
}

// 面の現在の色を取得
uint16_t OctagonRingView::getFaceColor(int faceId) const {
    if (faceId < 0 || faceId >= NUM_FACES) return backgroundColor;
    
    // 一時的な色があればそれを使用
    if (hasTempColor[faceId]) {
        return tempFaceColors[faceId];
    }
    // 通常のハイライト状態
    else if (highlightedFaces[faceId]) {
        return highlightColor;
    }
    // FaceDetectorによる色
    else if (faceDetector != nullptr && faceDetector->getCalibratedFacesCount() > 0) {
        FaceData* faceList = faceDetector->getFaceList();
        if (faceId < faceDetector->getCalibratedFacesCount() && faceList[faceId].ledState == 1) {
            CRGB ledColor = faceList[faceId].ledColor;
            return M5.Lcd.color565(ledColor.r, ledColor.g, ledColor.b);
        }
    }
    
    return backgroundColor;
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

// ハイライト面の設定（複数ハイライト対応版）
void OctagonRingView::setHighlightedFace(int faceID) {
    // 全てのハイライトをクリア
    for (int i = 0; i < NUM_FACES; i++) {
        highlightedFaces[i] = false;
    }
    
    // 有効な面IDの場合のみハイライト設定
    if (faceID >= 0 && faceID < NUM_FACES) {
        highlightedFaces[faceID] = true;
    }
    // -1の場合は全てのハイライトをクリア（既に上でクリア済み）
}

// 特定の面のハイライト状態を設定
void OctagonRingView::setFaceHighlighted(int faceID, bool highlighted) {
    if (faceID >= 0 && faceID < NUM_FACES) {
        highlightedFaces[faceID] = highlighted;
    }
}

// 特定の面のハイライト状態を取得
bool OctagonRingView::isFaceHighlighted(int faceID) const {
    if (faceID >= 0 && faceID < NUM_FACES) {
        return highlightedFaces[faceID];
    }
    return false;
}

// 後方互換性のため、最初にハイライトされている面を返す
int OctagonRingView::getHighlightedFace() {
    for (int i = 0; i < NUM_FACES; i++) {
        if (highlightedFaces[i]) {
            return i;
        }
    }
    return -1; // ハイライトされている面がない場合
}

void OctagonRingView::setHighlightColor(uint16_t color) {
    highlightColor = color;
}

uint16_t OctagonRingView::getHighlightColor() {
    return highlightColor;
}

// 鏡写しモードの設定
void OctagonRingView::setMirrored(bool mirror) {
    isMirrored = mirror;
}

// 中心点を軸に回転（角度はラジアンで加算）
void OctagonRingView::rotate(float dAngle) {
    rotationAngle += dAngle;
}

// FaceDetectorの設定
void OctagonRingView::setFaceDetector(FaceDetector* detector) {
    faceDetector = detector;
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
        
        // 鏡写し処理（X座標を反転）
        if (isMirrored) {
            x = -x;
        }
        
        x *= scale;
        y *= scale;
        projected[i][0] = centerX + int(x);
        // y軸反転（画面座標）
        projected[i][1] = centerY - int(y);
    }

    // 台形（各面）描画
    for(int i = 0; i < NUM_FACES; i++){
        drawFace(i);
    }

    // 中央部分の描画
    // drawCenter();
}

bool OctagonRingView::isPointInTriangle(int px, int py, 
                                    int x1, int y1, 
                                    int x2, int y2, 
                                    int x3, int y3) const {
    // 3つの小三角形の面積の合計が元の三角形と等しければ点は三角形の中
    float areaOrig = abs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));
    
    float area1 = abs((x1 - px) * (y2 - py) - (x2 - px) * (y1 - py));
    float area2 = abs((x2 - px) * (y3 - py) - (x3 - px) * (y2 - py));
    float area3 = abs((x3 - px) * (y1 - py) - (x1 - px) * (y3 - py));
    
    // 丸め誤差を考慮して小さな許容誤差を加える
    float areaSumWithEpsilon = area1 + area2 + area3 + 0.1f;
    
    return areaSumWithEpsilon >= areaOrig && area1 > 0 && area2 > 0 && area3 > 0;
}

int OctagonRingView::getFaceAtPoint(int screenX, int screenY) const {
    Serial.println("getFaceAtPoint x: " + String(screenX) + " y: " + String(screenY));

    // タップ点が表示領域外なら早期リターン
    if (screenX < viewX || screenX >= viewX + viewWidth ||
        screenY < viewY || screenY >= viewY + viewHeight) {
        Serial.println("out of bounds");
        return -1;
    }
    
    // 中心点からの距離を計算
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;
    float dx = screenX - centerX;
    float dy = screenY - centerY;
    float distSq = dx * dx + dy * dy;
    
    // 中心近くの場合は-1を返す
    float innerRadius = min(viewWidth, viewHeight) * 0.2f;
    if (distSq < innerRadius * innerRadius) {
        Serial.println("center area");
        return -1;
    }
    
    // タップ点の角度（0〜2π）を計算
    float angle = atan2(dy, dx);
    if (angle < 0) angle += 2 * M_PI;
    
    // デバッグ出力
    Serial.println("original angle: " + String(angle * 180 / M_PI) + " degrees");
    
    // 鏡写しモード（x軸反転）の処理
    if (isMirrored) {
        angle = 2 * M_PI - angle;
        if (angle >= 2 * M_PI) angle -= 2 * M_PI;
    }
    
    // 回転角度の適用
    angle -= rotationAngle;
    if (angle < 0) angle += 2 * M_PI;
    if (angle >= 2 * M_PI) angle -= 2 * M_PI;
    
    // 角度から面を算出（8分割）
    int faceId = (int)(angle * NUM_FACES / (2 * M_PI));
    
    // オフセット調整（右下が面0になるように）
    int offsetFaceId = (faceId + 6) % NUM_FACES;
    
    // 面の順序を連続的にするためのマッピング
    // ログから: 0,7,6,5,4... となっているので、時計回りに 0,1,2,3,4... となるように修正
    int finalFaceId = offsetFaceId;
    
    // 新しいマッピングテーブルを作成
    static const int faceOrderMap[NUM_FACES] = {
        0, // 0 → 0 (そのまま)
        1, // 1 → 1 (そのまま)
        2, // 2 → 2 (そのまま)
        3, // 3 → 3 (そのまま)
        4, // 4 → 4 (そのまま)
        5, // 5 → 5 (そのまま)
        6, // 6 → 6 (そのまま)
        7  // 7 → 7 (そのまま)
    };
    
    // ログから: 0,7,6となっているので、0,1,2に変換
    if (offsetFaceId == 7) finalFaceId = 1;
    else if (offsetFaceId == 6) finalFaceId = 2;
    else if (offsetFaceId == 5) finalFaceId = 3;
    else if (offsetFaceId == 4) finalFaceId = 4;
    else if (offsetFaceId == 3) finalFaceId = 5;
    else if (offsetFaceId == 2) finalFaceId = 6;
    else if (offsetFaceId == 1) finalFaceId = 7;
    // 0はそのまま
    
    // 角度値とマッピング後のデバッグ出力
    Serial.println("adjusted angle: " + String(angle * 180 / M_PI) + 
                  " degrees, raw faceId: " + String(faceId) +
                  ", offset faceId: " + String(offsetFaceId) +
                  ", final faceId: " + String(finalFaceId));
    
    // 外側の境界チェック
    float outerRadius = min(viewWidth, viewHeight) * 0.45f;
    if (distSq > outerRadius * outerRadius) {
        Serial.println("outside outer radius");
        return -1;
    }
    
    return finalFaceId;
}
