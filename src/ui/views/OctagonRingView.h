#ifndef OCTAGON_RING_VIEW_H
#define OCTAGON_RING_VIEW_H

#include <Arduino.h>
#include <M5Unified.h>

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
    int highlightedFace;  // ハイライト表示する面ID (-1=なし)
    float rotationAngle;  // 回転角度
    bool isMirrored;  // 鏡写しモード

    // ジオメトリ更新関数
    void updateGeometry();

public:
    // コンストラクタ
    OctagonRingView();

    // 表示領域の設定
    void setViewPosition(int x, int y, int width, int height);

    // 背景色の設定
    void setBackgroundColor(uint16_t color);

    // ハイライト面の設定
    void setHighlightedFace(int faceID);
    
    // ハイライト面の取得
    int getHighlightedFace();
    
    // 鏡写しモードの設定
    void setMirrored(bool mirror);

    // 回転角度の設定（ラジアン単位で加算）
    void rotate(float deltaAngle);

    // 描画
    void draw();
};

#endif // OCTAGON_RING_VIEW_H