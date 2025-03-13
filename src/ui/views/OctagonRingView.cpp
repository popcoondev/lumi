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
    , rotationAngle(M_PI/8.0f)  // デフォルトでPI/8（22.5度）回転
    , isMirrored(true) // デフォルトで鏡写しに設定
    , highlightColor(WHITE)     // デフォルトのハイライト色は白
    , focusColor(TFT_YELLOW)    // フォーカス色は黄色
    , faceDetector(nullptr) // FaceDetectorは初期化時にnull
{
    // ハイライト面の初期化（すべて非ハイライト）
    for (int i = 0; i < NUM_FACES; i++) {
        highlightedFaces[i] = false;
        focusedFaces[i] = false;
        tempFaceColors[i] = TFT_BLACK;
        hasTempColor[i] = false;
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

    // 各面ボタンの初期化
    for (int i = 0; i < NUM_FACES; i++) {
        faceButtons[i].setFaceId(i);
        faceButtons[i].setHighlightColor(WHITE);
        faceButtons[i].setFocusColor(TFT_YELLOW);
        faceButtons[i].setFaceDetector(faceDetector);
    }
    
    // 中央ボタンの初期化
    centerButton.setColor(BLACK, BLACK);
}

// 中央ボタンの情報テキスト設定
void OctagonRingView::setCenterInfo(const String& text, uint16_t color) {
    centerButton.setInfo(text, color);
}

void OctagonRingView::drawFace(int faceId) {
    if (faceId < 0 || faceId >= NUM_FACES) return;
    
    int v0 = faces[faceId][0];
    int v1 = faces[faceId][1];
    int v2 = faces[faceId][2];
    int v3 = faces[faceId][3];
    
    // 面の色を判定
    uint16_t color = backgroundColor;
    if (focusedFaces[faceId]) {
        color = focusColor;
    }
    // 一時的な色があればそれを使用
    else if (hasTempColor[faceId]) {
        color = tempFaceColors[faceId];
    }
    // 通常のハイライト状態
    // else if (highlightedFaces[faceId]) {
    //     color = highlightColor;
    // }
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
    
    // エッジの色を決定
    uint16_t edgeColor = TFT_WHITE;
    
    // 通常の線幅で描画
    M5.Lcd.drawLine(
        projected[v0][0], projected[v0][1],
        projected[v1][0], projected[v1][1],
        edgeColor
    );
    M5.Lcd.drawLine(
        projected[v1][0], projected[v1][1],
        projected[v2][0], projected[v2][1],
        edgeColor
    );
    M5.Lcd.drawLine(
        projected[v2][0], projected[v2][1],
        projected[v3][0], projected[v3][1],
        edgeColor
    );
    M5.Lcd.drawLine(
        projected[v3][0], projected[v3][1],
        projected[v0][0], projected[v0][1],
        edgeColor
    );
    
    // フォーカス状態の場合、外側に平行線を描画
    // if (focusedFaces[faceId]) {
    //     // 外側の底辺（v0-v1）に平行な線を描画
    //     // 底辺の方向ベクトルを計算
    //     int dx = projected[v1][0] - projected[v0][0];
    //     int dy = projected[v1][1] - projected[v0][1];
        
    //     // 底辺に垂直な方向ベクトル（時計回りに90度回転）
    //     int perpX = -dy;
    //     int perpY = dx;
        
    //     // ベクトルの長さを計算
    //     float length = sqrt(perpX * perpX + perpY * perpY);
    //     if (length > 0) {
    //         // 単位ベクトル化して2ピクセル分の長さにする
    //         float normalizedPerpX = perpX / length * 2;
    //         float normalizedPerpY = perpY / length * 2;
            
    //         // 底辺の外側2ピクセルの位置に線を描画
    //         int startX = projected[v0][0] + normalizedPerpX;
    //         int startY = projected[v0][1] + normalizedPerpY;
    //         int endX = projected[v1][0] + normalizedPerpX;
    //         int endY = projected[v1][1] + normalizedPerpY;
            
    //         // 黄色の線を描画（太さ2ピクセル）
    //         for (int i = 0; i < 2; i++) {
    //             M5.Lcd.drawLine(
    //                 startX, startY + i,
    //                 endX, endY + i,
    //                 focusColor
    //             );
    //         }
    //     }
    // }
}

// 中央部分のみ再描画
void OctagonRingView::drawCenter() {
    int centerX = viewX + viewWidth / 2;
    int centerY = viewY + viewHeight / 2;
    float innerRadius = min(viewWidth, viewHeight) * 0.2f;
    
    // 中央円を背景色で描画
    M5.Lcd.fillCircle(centerX, centerY, innerRadius, BLACK);
    
    // 中央円の輪郭を描画
    // M5.Lcd.drawCircle(centerX, centerY, innerRadius, TFT_WHITE);
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

// 特定の面のフォーカス状態を設定
void OctagonRingView::setFaceFocused(int faceID, bool focused) {
    if (faceID >= 0 && faceID < NUM_FACES) {
        focusedFaces[faceID] = focused;
    }
}

// 特定の面のフォーカス状態を取得
bool OctagonRingView::isFaceFocused(int faceID) const {
    if (faceID >= 0 && faceID < NUM_FACES) {
        return focusedFaces[faceID];
    }
    return false;
}

// フォーカス状態の面の数を取得
int OctagonRingView::getFocusedFacesCount() const {
    int count = 0;
    for (int i = 0; i < NUM_FACES; i++) {
        if (focusedFaces[i]) {
            count++;
        }
    }
    return count;
}

// フォーカス色の設定
void OctagonRingView::setFocusColor(uint16_t color) {
    focusColor = color;
}

// フォーカス色の取得
uint16_t OctagonRingView::getFocusColor() const {
    return focusColor;
}

// すべての面のフォーカスを解除
void OctagonRingView::clearAllFocus() {
    for (int i = 0; i < NUM_FACES; i++) {
        focusedFaces[i] = false;
    }
}

// フォーカスされた面のLED状態を取得
int OctagonRingView::getFocusedFacesLedState() const {
    int onCount = 0;
    int focusCount = 0;
    
    for (int i = 0; i < NUM_FACES; i++) {
        if (focusedFaces[i]) {
            focusCount++;
            
            // LED状態を確認
            if (faceDetector != nullptr && faceDetector->getCalibratedFacesCount() > 0) {
                FaceData* faceList = faceDetector->getFaceList();
                if (i < faceDetector->getCalibratedFacesCount() && faceList[i].ledState == 1) {
                    onCount++;
                }
            } else if (highlightedFaces[i]) {
                // FaceDetectorがない場合はハイライト状態で代用
                onCount++;
            }
        }
    }
    
    // すべて点灯: 2, すべて消灯: 0, 一部点灯: 1
    if (focusCount == 0) return -1; // フォーカスなし
    if (onCount == 0) return 0;     // すべて消灯
    if (onCount == focusCount) return 2; // すべて点灯
    return 1; // 一部点灯
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
        // 後方互換性のため、古い実装も呼び出す
        drawFace(i);
        
        // 新しいボタンクラスの設定
        int v0 = faces[i][0];
        int v1 = faces[i][1];
        int v2 = faces[i][2];
        int v3 = faces[i][3];
        
        // 台形の頂点を設定
        faceButtons[i].setTrapezoidVertices(
            projected[v0][0], projected[v0][1],
            projected[v1][0], projected[v1][1],
            projected[v2][0], projected[v2][1],
            projected[v3][0], projected[v3][1]
        );
        
        // 状態を設定
        faceButtons[i].setHighlighted(highlightedFaces[i]);
        faceButtons[i].setFocused(focusedFaces[i]);
        if (hasTempColor[i]) {
            faceButtons[i].setTempColor(tempFaceColors[i]);
        } else {
            faceButtons[i].resetTempColor();
        }
    }

    // 中央ボタンの設定
    float innerRadius = min(viewWidth, viewHeight) * 0.2f;
    centerButton.setRadius(innerRadius);
    centerButton.setPosition(centerX - innerRadius, centerY - innerRadius, innerRadius * 2, innerRadius * 2);
    
    // 中央部分の描画
    drawCenter();
    centerButton.draw();
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

// タップされた座標が指定面の内部にあるか判定
bool OctagonRingView::isPointInFace(int faceID, int screenX, int screenY) const {
    if (faceID < 0 || faceID >= NUM_FACES) return false;
    
    // 新しいボタンクラスのcontainsPointメソッドを使用
    return faceButtons[faceID].containsPoint(screenX, screenY);
}

int OctagonRingView::getFaceAtPoint(int screenX, int screenY) const {
    Serial.println("getFaceAtPoint x: " + String(screenX) + " y: " + String(screenY));

    // タップ点が表示領域外なら早期リターン
    if (screenX < viewX || screenX >= viewX + viewWidth ||
        screenY < viewY || screenY >= viewY + viewHeight) {
        Serial.println("out of bounds");
        return -1;
    }
    
    // 中心ボタンのチェック
    if (centerButton.containsPoint(screenX, screenY)) {
        Serial.println("center area");
        return -1;
    }
    
    // 各面ボタンをチェック
    for (int i = 0; i < NUM_FACES; i++) {
        if (faceButtons[i].containsPoint(screenX, screenY)) {
            return i;
        }
    }
    
    // どの面にも含まれない場合
    return -1;
}
