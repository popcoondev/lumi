// M5StackChanFace.h
#ifndef M5STACKCHANFACE_H
#define M5STACKCHANFACE_H

#include <M5Unified.h>  // M5Stackライブラリ

class M5StackChanFace {
  private:
    // 描画領域の左上座標と幅・高さ
    int _x, _y, _w, _h;
    
    // まばたき制御用
    bool _blinking;              // 現在まばたき中か？
    unsigned long _blinkStart;   // まばたき開始時刻
    unsigned long _nextBlink;    // 次のまばたき開始時刻
    const unsigned long _blinkDuration = 200; // まばたきの持続時間（ms）
    
    // 首（顔全体）の傾き制御用
    float _tiltAngle;               // 傾き角度（ラジアン）
    unsigned long _nextTiltChange;  // 次の傾き更新時刻

    // 乱数シードの初期化（setup()で1度呼び出すことも可）
    void initRandom() {
      randomSeed(analogRead(0));
    }

  public:
    // コンストラクタ：描画領域の左上(x,y)と幅(w)、高さ(h)を指定
    M5StackChanFace(int x, int y, int w, int h)
      : _x(x), _y(y), _w(w), _h(h), _blinking(false), _tiltAngle(0.0)
    {
      _nextBlink = millis() + random(2000, 5000);
      _nextTiltChange = millis() + random(3000, 10000);
    }
    
    // update()を毎ループ呼び出すことで、まばたきと傾きの状態を更新
    void update() {
      unsigned long now = millis();
      
      // まばたき制御
      if (!_blinking && now >= _nextBlink) {
        _blinking = true;
        _blinkStart = now;
      }
      if (_blinking && (now - _blinkStart >= _blinkDuration)) {
        _blinking = false;
        _nextBlink = now + random(2000, 5000);  // 次のまばたきまでの間隔をランダムに設定
      }
      
      // 傾き制御：一定時間ごとに傾き角度をランダム更新
      if (now >= _nextTiltChange) {
        // -10度～+10度（ラジアンに変換）の間で設定
        _tiltAngle = radians(random(-10, 11));
        _nextTiltChange = now + random(3000, 10000);
      }
    }
    
    // draw()で顔を描画
    void draw() {
      // 更新の際は背景を塗りつぶす
      M5.Lcd.fillRect(_x, _y, _w, _h, BLACK);

      // （オプション）顔の領域を枠線で表示する場合
      // M5.Lcd.drawRect(_x, _y, _w, _h, WHITE);
      
      // 顔の中心点
      int cx = _x + _w / 2;
      int cy = _y + _h / 2;
      
      // 目と口の相対オフセット（領域サイズに応じた割合）
      float eyeOffsetX = _w / 6.0;   // 両目の左右のずれ
      float eyeOffsetY = _h / 6.0;   // 目の上下位置
      float mouthOffsetY = _h / 6.0; // 口の上下位置
      
      // 通常状態での各部品の相対位置（中心を原点とした座標）
      float leftEyeRelX  = -eyeOffsetX;
      float leftEyeRelY  = -eyeOffsetY;
      float rightEyeRelX = eyeOffsetX;
      float rightEyeRelY = -eyeOffsetY;
      float mouthRelX    = 0;
      float mouthRelY    = mouthOffsetY;
      
      // 傾き角度を考慮して各部品の相対座標を回転
      float cosA = cos(_tiltAngle);
      float sinA = sin(_tiltAngle);
      
      int leftEyeX = cx + (int)(leftEyeRelX * cosA - leftEyeRelY * sinA);
      int leftEyeY = cy + (int)(leftEyeRelX * sinA + leftEyeRelY * cosA);
      
      int rightEyeX = cx + (int)(rightEyeRelX * cosA - rightEyeRelY * sinA);
      int rightEyeY = cy + (int)(rightEyeRelX * sinA + rightEyeRelY * cosA);
      
      int mouthX = cx + (int)(mouthRelX * cosA - mouthRelY * sinA);
      int mouthY = cy + (int)(mouthRelX * sinA + mouthRelY * cosA);
      
      // 目の描画：まばたき中なら短い水平線、通常は小さな塗りつぶし円
      if (_blinking) {
        M5.Lcd.drawLine(leftEyeX - 2, leftEyeY, leftEyeX + 2, leftEyeY, WHITE);
        M5.Lcd.drawLine(rightEyeX - 2, rightEyeY, rightEyeX + 2, rightEyeY, WHITE);
      } else {
        M5.Lcd.fillCircle(leftEyeX, leftEyeY, 2, WHITE);
        M5.Lcd.fillCircle(rightEyeX, rightEyeY, 2, WHITE);
      }
      
      // 口は横長の矩形
      // 幅は左目と右目の間隔に合わせ、高さは目の上下位置に合わせる
      int mouthWidth = (rightEyeX - leftEyeX) / 2;
      M5.Lcd.fillRect(mouthX - mouthWidth / 2, mouthY - 10, mouthWidth, 2, WHITE);
    }
};

#endif  // M5STACKCHANFACE_H
