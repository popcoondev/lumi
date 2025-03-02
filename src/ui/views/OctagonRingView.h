#ifndef OCTAGON_RING_VIEW_H
#define OCTAGON_RING_VIEW_H

#include <Arduino.h>
#include <M5Unified.h>
#include "../system/FaceDetector.h"

// 八角形リングのジオメトリ定数
#define NUM_OUTER 8  // 外側の頂点数
#define NUM_INNER 8  // 内側の頂点数
#define NUM_VERTICES (NUM_OUTER + NUM_INNER)  // 頂点数合計 = 16
#define NUM_FACES NUM_OUTER  // 面の数 = 8
#define NUM_EDGES (NUM_OUTER + NUM_INNER + NUM_OUTER)  // エッジ数 = 24

class OctagonRingView {
private:
    // 3Dモデルのパラメータ
    float outerRadius;  // 外側リングの半径
    float innerRadius;  // 内側リングの半径

    // 頂点座標（モデル座標系）
    float vertices[NUM_VERTICES][2];

    // 投影後の頂点座標（スクリーン座標系）
    int projected[NUM_VERTICES][2];

    // 面の定義（頂点インデックス4つで台形を表現）
    int faces[NUM_FACES][4];

    // エッジの定義（頂点インデックス2つで線分を表現）
    int edges[NUM_EDGES][2];

    // 表示パラメータ
    int viewX, viewY;  // 表示領域左上座標
    int viewWidth, viewHeight;  // 表示領域サイズ
    uint16_t backgroundColor;  // 背景色
    bool highlightedFaces[NUM_FACES];  // 各面のハイライト状態
    uint16_t highlightColor;   // ハイライト色
    float rotationAngle;  // 回転角度
    bool isMirrored;  // 鏡写しモード
    
    // FaceDetectorへの参照
    FaceDetector* faceDetector;

    // ジオメトリ更新関数
    void updateGeometry();

    // 点が三角形内に含まれるか判定する補助関数
    bool isPointInTriangle(int px, int py, 
                        int x1, int y1, 
                        int x2, int y2, 
                        int x3, int y3) const;

    friend class LumiView;
    uint16_t tempFaceColors[NUM_FACES];   // タッチフィードバック用の一時的な面の色
    bool hasTempColor[NUM_FACES];         // 一時的な色が設定されているかのフラグ

public:
    // コンストラクタ
    OctagonRingView();
    
    // FaceDetectorの設定
    void setFaceDetector(FaceDetector* detector);

    // 表示領域の設定
    void setViewPosition(int x, int y, int width, int height);

    // 背景色の設定
    void setBackgroundColor(uint16_t color);

    // ハイライト面の設定
    void setHighlightedFace(int faceID);
    
    // 特定の面のハイライト状態を設定
    void setFaceHighlighted(int faceID, bool highlighted);
    
    // 特定の面のハイライト状態を取得
    bool isFaceHighlighted(int faceID) const;
    
    // ハイライト面の取得
    int getHighlightedFace();
    
    // ハイライト色の設定
    void setHighlightColor(uint16_t color);
    
    // ハイライト色の取得
    uint16_t getHighlightColor();
    
    // 鏡写しモードの設定
    void setMirrored(bool mirror);

    // 回転角度の設定（ラジアン単位で加算）
    void rotate(float deltaAngle);

    // 描画
    void draw();

    // タップされた座標から面IDを取得
    int getFaceAtPoint(int screenX, int screenY) const;

    // タップされた座標が指定面の内部にあるか判定
    bool isPointInFace(int faceID, int screenX, int screenY) const;

    // 特定の面だけを再描画
    void drawFace(int faceId);
    
    // センター部分のみ再描画
    void drawCenter();
    
    // 面に一時的な色を設定（タッチフィードバック用）
    void setFaceTempColor(int faceId, uint16_t color);
    
    // 一時的な色を解除
    void resetFaceTempColor(int faceId);
    
    // 面の現在の色を取得
    uint16_t getFaceColor(int faceId) const;

};

#endif // OCTAGON_RING_VIEW_H
