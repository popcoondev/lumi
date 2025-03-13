#ifndef OCTAGON_RING_VIEW_H
#define OCTAGON_RING_VIEW_H

#include <Arduino.h>
#include <M5Unified.h>
#include "../system/FaceDetector.h"
#include "../ui/components/Button.h"

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
    float rotationAngle;  // 回転角度
    bool isMirrored;  // 鏡写しモード
    uint16_t highlightColor;   // ハイライト色（後方互換性のため）
    uint16_t focusColor;       // フォーカス色（後方互換性のため）
    bool highlightedFaces[NUM_FACES];  // 各面のハイライト状態（後方互換性のため）
    bool focusedFaces[NUM_FACES];      // 各面のフォーカス状態（後方互換性のため）
    uint16_t tempFaceColors[NUM_FACES]; // 一時的な色（後方互換性のため）
    bool hasTempColor[NUM_FACES];       // 一時的な色フラグ（後方互換性のため）
    
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
    
    // ボタンオブジェクト
    TrapezoidButton faceButtons[NUM_FACES];  // 各面のボタン
    CenterButton centerButton;               // 中央ボタン

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
    
    // 特定の面のフォーカス状態を設定
    void setFaceFocused(int faceID, bool focused);
    
    // 特定の面のフォーカス状態を取得
    bool isFaceFocused(int faceID) const;
    
    // フォーカス状態の面の数を取得
    int getFocusedFacesCount() const;
    
    // フォーカス色の設定
    void setFocusColor(uint16_t color);
    
    // フォーカス色の取得
    uint16_t getFocusColor() const;
    
    // すべての面のフォーカスを解除
    void clearAllFocus();
    
    // フォーカスされた面のLED状態を取得
    int getFocusedFacesLedState() const;
    
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
    
    // 中央ボタンの情報テキスト設定
    void setCenterInfo(const String& text, uint16_t color);
    
    // 中央ボタンへのアクセサ
    CenterButton& getCenterButton() { return centerButton; }
    
    // 面ボタンの取得
    TrapezoidButton& getFaceButton(int faceId) { 
        if (faceId >= 0 && faceId < NUM_FACES) {
            return faceButtons[faceId];
        }
        return faceButtons[0]; // 範囲外の場合は最初の面を返す
    }
};

#endif // OCTAGON_RING_VIEW_H
