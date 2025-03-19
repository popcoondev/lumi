#include "LEDControlActivity.h"
#include <M5Unified.h>

LEDControlActivity::LEDControlActivity()
    : Activity(0, "LEDControlActivity"),
      m_ledManager(nullptr),
      m_playPauseButton(nullptr),
      m_prevButton(nullptr),
      m_nextButton(nullptr),
      m_homeButton(nullptr),
      m_modeButton(nullptr),
      m_isPlaying(false),
      m_isJsonPattern(false),
      m_currentJsonPatternIndex(0)
{
}

LEDControlActivity::~LEDControlActivity() {
    delete m_playPauseButton;
    delete m_prevButton;
    delete m_nextButton;
    delete m_homeButton;
    delete m_modeButton;
}

bool LEDControlActivity::onCreate() {
    if (!Activity::onCreate()) {
        return false;
    }
    
    // ButtonFragment インスタンスの作成
    m_playPauseButton = new ButtonFragment(ID_BUTTON_PLAY_PAUSE);
    m_prevButton = new ButtonFragment(ID_BUTTON_PREV);
    m_nextButton = new ButtonFragment(ID_BUTTON_NEXT);
    m_homeButton = new ButtonFragment(ID_BUTTON_HOME);
    m_modeButton = new ButtonFragment(ID_BUTTON_MODE);
    
    // ButtonFragment の作成
    m_playPauseButton->onCreate();
    m_prevButton->onCreate();
    m_nextButton->onCreate();
    m_homeButton->onCreate();
    m_modeButton->onCreate();
    
    // ボタンの位置とサイズを設定 - サイズを大きくして視認性を向上
    m_playPauseButton->setDisplayArea(120, 100, 80, 40);
    m_prevButton->setDisplayArea(20, 100, 80, 40);
    m_nextButton->setDisplayArea(220, 100, 80, 40);
    m_homeButton->setDisplayArea(120, 220, 80, 40);
    m_modeButton->setDisplayArea(120, 160, 80, 40);
    
    // ボタンのスタイル設定 - 色を変更して視認性を向上
    m_playPauseButton->setLabel("Play");
    m_playPauseButton->setColor(TFT_GREEN, TFT_LIGHTGREY);
    m_playPauseButton->setFontSize(1.5);
    m_playPauseButton->setType(BUTTON_TYPE_TEXT);
    
    m_prevButton->setLabel("Prev");
    m_prevButton->setColor(TFT_YELLOW, TFT_LIGHTGREY);
    m_prevButton->setFontSize(1.5);
    m_prevButton->setType(BUTTON_TYPE_TEXT);
    
    m_nextButton->setLabel("Next");
    m_nextButton->setColor(TFT_YELLOW, TFT_LIGHTGREY);
    m_nextButton->setFontSize(1.5);
    m_nextButton->setType(BUTTON_TYPE_TEXT);
    
    m_homeButton->setLabel("Home");
    m_homeButton->setColor(TFT_BLUE, TFT_LIGHTGREY);
    m_homeButton->setFontSize(1.5);
    m_homeButton->setType(BUTTON_TYPE_TEXT);
    
    m_modeButton->setLabel("Built-in");
    m_modeButton->setColor(TFT_PURPLE, TFT_LIGHTGREY);
    m_modeButton->setFontSize(1.5);
    m_modeButton->setType(BUTTON_TYPE_TEXT);
    
    // クリックハンドラの設定
    m_playPauseButton->setClickHandler([this]() {
        togglePlayPause();
    });
    
    m_prevButton->setClickHandler([this]() {
        prevPattern();
    });
    
    m_nextButton->setClickHandler([this]() {
        nextPattern();
    });
    
    m_homeButton->setClickHandler([this]() {
        Serial.println("LEDControlActivity: Home button clicked");
        
        if (m_isPlaying) {
            Serial.println("LEDControlActivity: Stopping LED pattern");
            m_ledManager->stopPattern();
            m_isPlaying = false;
        }
        
        if (onHomeRequested) {
            Serial.println("LEDControlActivity: Calling onHomeRequested callback");
            onHomeRequested();
        } else {
            Serial.println("LEDControlActivity: onHomeRequested callback is not set");
        }
    });
    
    m_modeButton->setClickHandler([this]() {
        togglePatternMode();
    });
    
    // Fragmentの追加
    addFragment(m_playPauseButton, "playPauseButton");
    addFragment(m_prevButton, "prevButton");
    addFragment(m_nextButton, "nextButton");
    addFragment(m_homeButton, "homeButton");
    addFragment(m_modeButton, "modeButton");
    
    return true;
}

bool LEDControlActivity::onStart() {
    if (!Activity::onStart()) {
        return false;
    }
    return true;
}

bool LEDControlActivity::onResume() {
    if (!Activity::onResume()) {
        return false;
    }
    
    // 状態を初期化
    m_isPlaying = false;
    m_playPauseButton->setLabel("Play");
    
    // 画面を描画
    draw();
    
    return true;
}

void LEDControlActivity::onPause() {
    // パターン再生中なら停止
    if (m_isPlaying) {
        m_ledManager->stopPattern();
        m_isPlaying = false;
    }
    
    Activity::onPause();
}

void LEDControlActivity::onStop() {
    Activity::onStop();
}

void LEDControlActivity::onDestroy() {
    Activity::onDestroy();
}

bool LEDControlActivity::handleEvent(const framework::Event& event) {
    // 基底クラスのイベント処理
    if (Activity::handleEvent(event)) {
        return true;
    }
    
    return false;
}

void LEDControlActivity::initialize(LEDManager* ledManager) {
    m_ledManager = ledManager;
}

void LEDControlActivity::draw() {
    // 画面の背景をクリア
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // タイトルを描画
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString("LED Control", 160, 20);
    
    // ボタンを描画
    m_playPauseButton->draw();
    m_prevButton->draw();
    m_nextButton->draw();
    m_homeButton->draw();
    m_modeButton->draw();
    
    // パターン情報を表示
    updatePatternInfo();
}

void LEDControlActivity::updatePatternInfo() {
    String statusText;
    if (m_isPlaying) {
        statusText = "Playing: ";
    } else {
        statusText = "Pattern: ";
    }
    
    // モードに応じたパターン情報を表示
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(10, 160);
    
    if (m_isJsonPattern) {
        // JSONパターンモード
        if (m_ledManager->getJsonPatternCount() > 0) {
            String patternName = m_ledManager->getJsonPatternName(m_currentJsonPatternIndex);
            M5.Lcd.print(statusText + patternName + "     ");
            
            // パターン番号を表示
            M5.Lcd.setTextSize(1);
            M5.Lcd.setCursor(10, 180);
            M5.Lcd.print("JSON Pattern " + String(m_currentJsonPatternIndex + 1) + 
                        " of " + String(m_ledManager->getJsonPatternCount()) + "     ");
        } else {
            // JSONパターンがない場合
            M5.Lcd.print(statusText + "None" + "     ");
            
            // パターン番号を表示
            M5.Lcd.setTextSize(1);
            M5.Lcd.setCursor(10, 180);
            M5.Lcd.print("No JSON patterns available     ");
        }
    } else {
        // 通常パターンモード
        M5.Lcd.print(statusText + m_ledManager->getCurrentPatternName() + "     ");
        
        // パターン番号を表示
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(10, 180);
        M5.Lcd.print("Pattern " + String(m_ledManager->getCurrentPatternIndex() + 1) + 
                    " of " + String(m_ledManager->getPatternCount()) + "     ");
    }
}

void LEDControlActivity::togglePlayPause() {
    if (m_isPlaying) {
        // 一時停止
        m_ledManager->stopPattern();
        m_playPauseButton->setLabel("Play");
        m_isPlaying = false;
    } else {
        // 再生
        if (m_isJsonPattern) {
            // JSONパターンモード
            if (m_ledManager->getJsonPatternCount() > 0) {
                m_ledManager->runJsonPatternByIndex(m_currentJsonPatternIndex);
                m_playPauseButton->setLabel("Pause");
                m_isPlaying = true;
            } else {
                // JSONパターンがない場合
                M5.Lcd.setTextSize(1);
                M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
                M5.Lcd.setCursor(10, 200);
                M5.Lcd.print("No JSON patterns available");
                return;
            }
        } else {
            // 通常パターンモード
            m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
            m_playPauseButton->setLabel("Pause");
            m_isPlaying = true;
        }
    }
    
    // ボタンを再描画
    m_playPauseButton->draw();
    
    // パターン情報を更新
    updatePatternInfo();
}

void LEDControlActivity::nextPattern() {
    if (m_isJsonPattern) {
        // JSONパターンモード
        if (m_ledManager->getJsonPatternCount() > 0) {
            // 次のJSONパターンインデックスを計算
            m_currentJsonPatternIndex = (m_currentJsonPatternIndex + 1) % m_ledManager->getJsonPatternCount();
            
            if (m_isPlaying) {
                m_ledManager->stopPattern();
                m_ledManager->runJsonPatternByIndex(m_currentJsonPatternIndex);
            }
        }
    } else {
        // 通常パターンモード
        m_ledManager->nextPattern();
        
        if (m_isPlaying) {
            m_ledManager->stopPattern();
            m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
        }
    }
    
    // パターン情報を更新
    updatePatternInfo();
}

void LEDControlActivity::prevPattern() {
    if (m_isJsonPattern) {
        // JSONパターンモード
        if (m_ledManager->getJsonPatternCount() > 0) {
            // 前のJSONパターンインデックスを計算
            m_currentJsonPatternIndex = (m_currentJsonPatternIndex - 1 + m_ledManager->getJsonPatternCount()) % m_ledManager->getJsonPatternCount();
            
            if (m_isPlaying) {
                m_ledManager->stopPattern();
                m_ledManager->runJsonPatternByIndex(m_currentJsonPatternIndex);
            }
        }
    } else {
        // 通常パターンモード
        m_ledManager->prevPattern();
        
        if (m_isPlaying) {
            m_ledManager->stopPattern();
            m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
        }
    }
    
    // パターン情報を更新
    updatePatternInfo();
}

void LEDControlActivity::togglePatternMode() {
    // 再生中なら停止
    if (m_isPlaying) {
        m_ledManager->stopPattern();
        m_isPlaying = false;
        m_playPauseButton->setLabel("Play");
        m_playPauseButton->draw();
    }
    
    // モード切替
    m_isJsonPattern = !m_isJsonPattern;
    
    if (m_isJsonPattern) {
        // JSONパターンモードに切替
        m_modeButton->setLabel("JSON");
        m_currentJsonPatternIndex = 0; // インデックスをリセット
        
        // JSONパターンの数を確認
        if (m_ledManager->getJsonPatternCount() > 0) {
            // 最初のJSONパターンを選択
            m_ledManager->runJsonPatternByIndex(m_currentJsonPatternIndex);
            m_ledManager->stopPattern();
        } else {
            // JSONパターンがない場合
            M5.Lcd.setTextSize(1);
            M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
            M5.Lcd.setCursor(10, 200);
            M5.Lcd.print("No JSON patterns available");
        }
    } else {
        // 通常パターンモードに切替
        m_modeButton->setLabel("Built-in");
    }
    
    // ボタンを再描画
    m_modeButton->draw();
    
    // パターン情報を更新
    updatePatternInfo();
}
