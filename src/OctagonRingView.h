#pragma once
#include <M5Unified.h>

class OctagonRingView {
public:
    OctagonRingView();

    // 表示領域の設定
    void setViewPosition(int x, int y, int w, int h);

    // 背景色の設定
    void setBackgroundColor(uint16_t color);

    // 特定の面をハイライトする (0～7)
    void setHighlightedFace(int faceID);

    // ハイライトされている面IDを取得
    int getHighlightedFace();

    // 描画
    void draw();

    // 中心点を軸に回転（回転角度をラジアン単位で加算）
    void rotate(float dAngle);

private:
    // 外側8頂点 + 内側8頂点 = 合計16頂点
    static const int NUM_OUTER   = 8;
    static const int NUM_INNER   = 8;
    static const int NUM_VERTICES = 16;

    // 面(台形)は8枚 (外側i,外側i+1,内側i+1,内側i) で構成
    static const int NUM_FACES = 8;

    // エッジは、外側リング8本 + 内側リング8本 + ラジアル(外-内)8本 = 24
    static const int NUM_EDGES = 24;

    // 外側半径・内側半径
    float outerRadius;
    float innerRadius;

    // 2D座標 (x, y)（モデル座標）
    float vertices[NUM_VERTICES][2];

    // 面：台形を4頂点で定義
    // faces[i] = { outer_i, outer_(i+1), inner_(i+1), inner_i }
    int faces[NUM_FACES][4];

    // エッジ配列 (頂点インデックス2つ)
    int edges[NUM_EDGES][2];

    // 描画領域
    int viewX;
    int viewY;
    int viewWidth;
    int viewHeight;

    // 背景色
    uint16_t backgroundColor;

    // 投影後のピクセル座標を保持
    int projected[NUM_VERTICES][2];

    // ハイライト対象の面ID (未指定なら -1)
    int highlightedFace;

    // モデル全体の回転角度（ラジアン）
    float rotationAngle;

    // 頂点座標を更新 (outerRadius, innerRadius) から計算
    void updateGeometry();
};
