#include "LEDControlActivity.h"
#include <M5Unified.h>

LEDControlActivity::LEDControlActivity()
    : Activity(0, "LEDControlActivity"),
      m_ledManager(nullptr),
      m_playPauseButton(nullptr),
      m_prevButton(nullptr),
      m_nextButton(nullptr),
      m_homeButton(nullptr),
      m_isPlaying(false)
{
}

LEDControlActivity::~LEDControlActivity() {
    delete m_playPauseButton;
    delete m_prevButton;
    delete m_nextButton;
    delete m_homeButton;
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
    
    // ButtonFragment の作成
    m_playPauseButton->onCreate();
    m_prevButton->onCreate();
    m_nextButton->onCreate();
    m_homeButton->onCreate();
    
    // ボタンの位置とサイズを設定
    m_playPauseButton->setDisplayArea(120, 100, 80, 40);
    m_prevButton->setDisplayArea(20, 100, 80, 40);
    m_nextButton->setDisplayArea(220, 100, 80, 40);
    m_homeButton->setDisplayArea(120, 220, 80, 40);
    
    // ボタンのスタイル設定
    m_playPauseButton->setLabel("Play");
    m_playPauseButton->setColor(BLACK, TFT_LIGHTGREY);
    m_playPauseButton->setFontSize(1.5);
    m_playPauseButton->setType(BUTTON_TYPE_TEXT);
    
    m_prevButton->setLabel("Prev");
    m_prevButton->setColor(BLACK, TFT_LIGHTGREY);
    m_prevButton->setFontSize(1.5);
    m_prevButton->setType(BUTTON_TYPE_TEXT);
    
    m_nextButton->setLabel("Next");
    m_nextButton->setColor(BLACK, TFT_LIGHTGREY);
    m_nextButton->setFontSize(1.5);
    m_nextButton->setType(BUTTON_TYPE_TEXT);
    
    m_homeButton->setLabel("Home");
    m_homeButton->setColor(BLACK, TFT_LIGHTGREY);
    m_homeButton->setFontSize(1.5);
    m_homeButton->setType(BUTTON_TYPE_TEXT);
    
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
        if (m_isPlaying) {
            m_ledManager->stopPattern();
            m_isPlaying = false;
        }
        
        if (onHomeRequested) {
            onHomeRequested();
        }
    });
    
    // Fragmentの追加
    addFragment(m_playPauseButton, "playPauseButton");
    addFragment(m_prevButton, "prevButton");
    addFragment(m_nextButton, "nextButton");
    addFragment(m_homeButton, "homeButton");
    
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
    
    // パターン情報を表示
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(10, 160);
    M5.Lcd.print(statusText + m_ledManager->getCurrentPatternName() + "     ");
    
    // パターン番号を表示
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 180);
    M5.Lcd.print("Pattern " + String(m_ledManager->getCurrentPatternIndex() + 1) + 
                " of " + String(m_ledManager->getPatternCount()) + "     ");
}

void LEDControlActivity::togglePlayPause() {
    if (m_isPlaying) {
        // 一時停止
        m_ledManager->stopPattern();
        m_playPauseButton->setLabel("Play");
        m_isPlaying = false;
    } else {
        // 再生
        m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
        m_playPauseButton->setLabel("Pause");
        m_isPlaying = true;
    }
    
    // ボタンを再描画
    m_playPauseButton->draw();
    
    // パターン情報を更新
    updatePatternInfo();
}

void LEDControlActivity::nextPattern() {
    m_ledManager->nextPattern();
    
    if (m_isPlaying) {
        m_ledManager->stopPattern();
        m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
    }
    
    // パターン情報を更新
    updatePatternInfo();
}

void LEDControlActivity::prevPattern() {
    m_ledManager->prevPattern();
    
    if (m_isPlaying) {
        m_ledManager->stopPattern();
        m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
    }
    
    // パターン情報を更新
    updatePatternInfo();
}
