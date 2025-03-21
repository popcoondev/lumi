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
      m_fpsToggleButton(nullptr),
      m_fps30Button(nullptr),
      m_fps60Button(nullptr),
      m_fps120Button(nullptr),
      m_isPlaying(false),
      m_isJsonPattern(false),
      m_currentJsonPatternIndex(0),
      m_lastFpsUpdateTime(0)
{
}

LEDControlActivity::~LEDControlActivity() {
    delete m_playPauseButton;
    delete m_prevButton;
    delete m_nextButton;
    delete m_homeButton;
    delete m_modeButton;
    delete m_fpsToggleButton;
    delete m_fps30Button;
    delete m_fps60Button;
    delete m_fps120Button;
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
    
    // FPS制御関連のボタン
    m_fpsToggleButton = new ButtonFragment(ID_BUTTON_FPS_TOGGLE);
    m_fps30Button = new ButtonFragment(ID_BUTTON_FPS_30);
    m_fps60Button = new ButtonFragment(ID_BUTTON_FPS_60);
    m_fps120Button = new ButtonFragment(ID_BUTTON_FPS_120);
    
    // ButtonFragment の作成
    m_playPauseButton->onCreate();
    m_prevButton->onCreate();
    m_nextButton->onCreate();
    m_homeButton->onCreate();
    m_modeButton->onCreate();
    m_fpsToggleButton->onCreate();
    m_fps30Button->onCreate();
    m_fps60Button->onCreate();
    m_fps120Button->onCreate();
    
    // ボタンの位置とサイズを設定 - サイズを大きくして視認性を向上
    m_playPauseButton->setDisplayArea(120, 80, 80, 40);
    m_prevButton->setDisplayArea(20, 80, 80, 40);
    m_nextButton->setDisplayArea(220, 80, 80, 40);
    m_homeButton->setDisplayArea(120, 220, 80, 40);
    m_modeButton->setDisplayArea(120, 140, 80, 40);
    
    // FPS制御ボタンの位置とサイズを設定
    m_fpsToggleButton->setDisplayArea(220, 180, 80, 30);
    m_fps30Button->setDisplayArea(20, 180, 50, 30);
    m_fps60Button->setDisplayArea(80, 180, 50, 30);
    m_fps120Button->setDisplayArea(140, 180, 50, 30);
    
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
    
    // FPS制御ボタンのスタイル設定
    m_fpsToggleButton->setLabel("FPS: ON");
    m_fpsToggleButton->setColor(TFT_CYAN, TFT_LIGHTGREY);
    m_fpsToggleButton->setFontSize(1.2);
    m_fpsToggleButton->setType(BUTTON_TYPE_TEXT);
    
    m_fps30Button->setLabel("30");
    m_fps30Button->setColor(TFT_DARKGREY, TFT_LIGHTGREY);
    m_fps30Button->setFontSize(1.2);
    m_fps30Button->setType(BUTTON_TYPE_TEXT);
    
    m_fps60Button->setLabel("60");
    m_fps60Button->setColor(TFT_DARKGREY, TFT_LIGHTGREY);
    m_fps60Button->setFontSize(1.2);
    m_fps60Button->setType(BUTTON_TYPE_TEXT);
    
    m_fps120Button->setLabel("120");
    m_fps120Button->setColor(TFT_DARKGREY, TFT_LIGHTGREY);
    m_fps120Button->setFontSize(1.2);
    m_fps120Button->setType(BUTTON_TYPE_TEXT);
    
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
    
    // FPS制御ボタンのクリックハンドラ
    m_fpsToggleButton->setClickHandler([this]() {
        toggleFpsControl();
    });
    
    m_fps30Button->setClickHandler([this]() {
        setFps(30);
    });
    
    m_fps60Button->setClickHandler([this]() {
        setFps(60);
    });
    
    m_fps120Button->setClickHandler([this]() {
        setFps(120);
    });
    
    // Fragmentの追加
    addFragment(m_playPauseButton, "playPauseButton");
    addFragment(m_prevButton, "prevButton");
    addFragment(m_nextButton, "nextButton");
    addFragment(m_homeButton, "homeButton");
    addFragment(m_modeButton, "modeButton");
    addFragment(m_fpsToggleButton, "fpsToggleButton");
    addFragment(m_fps30Button, "fps30Button");
    addFragment(m_fps60Button, "fps60Button");
    addFragment(m_fps120Button, "fps120Button");
    
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
    
    // FPS制御の状態を初期化
    if (m_ledManager) {
        // FPSボタンの状態を更新
        uint16_t currentFps = m_ledManager->getTargetFps();
        bool fpsEnabled = m_ledManager->isFpsControlEnabled();
        
        m_fpsToggleButton->setLabel(fpsEnabled ? "FPS: ON" : "FPS: OFF");
        
        // 現在のFPSに合わせてボタンの色を更新
        m_fps30Button->setColor(currentFps == 30 ? TFT_CYAN : TFT_DARKGREY, TFT_LIGHTGREY);
        m_fps60Button->setColor(currentFps == 60 ? TFT_CYAN : TFT_DARKGREY, TFT_LIGHTGREY);
        m_fps120Button->setColor(currentFps == 120 ? TFT_CYAN : TFT_DARKGREY, TFT_LIGHTGREY);
    }
    
    // FPS更新タイマーをリセット
    m_lastFpsUpdateTime = millis();
    
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
    m_fpsToggleButton->draw();
    m_fps30Button->draw();
    m_fps60Button->draw();
    m_fps120Button->draw();
    
    // パターン情報を表示
    updatePatternInfo();
    
    // FPS情報を表示
    updateFpsInfo();
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
    M5.Lcd.setCursor(10, 120);
    
    if (m_isJsonPattern) {
        // JSONパターンモード
        if (m_ledManager->getJsonPatternCount() > 0) {
            String patternName = m_ledManager->getJsonPatternName(m_currentJsonPatternIndex);
            M5.Lcd.print(statusText + patternName + "     ");
            
            // パターン番号を表示
            M5.Lcd.setTextSize(1);
            M5.Lcd.setCursor(10, 140);
            M5.Lcd.print("JSON Pattern " + String(m_currentJsonPatternIndex + 1) + 
                        " of " + String(m_ledManager->getJsonPatternCount()) + "     ");
        } else {
            // JSONパターンがない場合
            M5.Lcd.print(statusText + "None" + "     ");
            
            // パターン番号を表示
            M5.Lcd.setTextSize(1);
            M5.Lcd.setCursor(10, 140);
            M5.Lcd.print("No JSON patterns available     ");
        }
    } else {
        // 通常パターンモード
        M5.Lcd.print(statusText + m_ledManager->getCurrentPatternName() + "     ");
        
        // パターン番号を表示
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(10, 140);
        M5.Lcd.print("Pattern " + String(m_ledManager->getCurrentPatternIndex() + 1) + 
                    " of " + String(m_ledManager->getPatternCount()) + "     ");
    }
}

void LEDControlActivity::updateFpsInfo() {
    if (!m_ledManager) return;
    
    // 1秒ごとにFPS情報を更新
    unsigned long currentTime = millis();
    if (currentTime - m_lastFpsUpdateTime >= 1000) {
        m_lastFpsUpdateTime = currentTime;
        
        // FPS情報を表示
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setCursor(10, 200);
        
        String fpsStatus = "Target FPS: " + String(m_ledManager->getTargetFps());
        fpsStatus += "  Actual FPS: " + String(m_ledManager->getActualFps());
        M5.Lcd.print(fpsStatus + "     ");
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

void LEDControlActivity::toggleFpsControl() {
    if (!m_ledManager) return;
    
    // FPS制御の有効/無効を切り替え
    bool currentState = m_ledManager->isFpsControlEnabled();
    m_ledManager->enableFpsControl(!currentState);
    
    // ボタンのラベルを更新
    m_fpsToggleButton->setLabel(m_ledManager->isFpsControlEnabled() ? "FPS: ON" : "FPS: OFF");
    m_fpsToggleButton->draw();
    
    // 再生中のパターンがあれば、一度停止して再開する（設定を反映するため）
    if (m_isPlaying) {
        m_ledManager->stopPattern();
        
        if (m_isJsonPattern) {
            m_ledManager->runJsonPatternByIndex(m_currentJsonPatternIndex);
        } else {
            m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
        }
    }
    
    // FPS情報を更新
    updateFpsInfo();
}

void LEDControlActivity::setFps(uint16_t fps) {
    if (!m_ledManager) return;
    
    // FPSを設定
    m_ledManager->setTargetFps(fps);
    
    // ボタンの色を更新
    m_fps30Button->setColor(fps == 30 ? TFT_CYAN : TFT_DARKGREY, TFT_LIGHTGREY);
    m_fps60Button->setColor(fps == 60 ? TFT_CYAN : TFT_DARKGREY, TFT_LIGHTGREY);
    m_fps120Button->setColor(fps == 120 ? TFT_CYAN : TFT_DARKGREY, TFT_LIGHTGREY);
    
    // ボタンを再描画
    m_fps30Button->draw();
    m_fps60Button->draw();
    m_fps120Button->draw();
    
    // 再生中のパターンがあれば、一度停止して再開する（設定を反映するため）
    if (m_isPlaying) {
        m_ledManager->stopPattern();
        
        if (m_isJsonPattern) {
            m_ledManager->runJsonPatternByIndex(m_currentJsonPatternIndex);
        } else {
            m_ledManager->runPattern(m_ledManager->getCurrentPatternIndex());
        }
    }
    
    // FPS情報を更新
    updateFpsInfo();
}
