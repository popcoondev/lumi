#ifndef NETWORK_SETTINGS_ACTIVITY_H
#define NETWORK_SETTINGS_ACTIVITY_H

#include "../framework/Activity.h"
#include "../fragments/ButtonFragment.h"
#include "../network/NetworkManager.h"
#include "../ui/components/TextView.h"

class NetworkSettingsActivity : public framework::Activity {
public:
    NetworkSettingsActivity();
    virtual ~NetworkSettingsActivity();
    
    virtual bool onCreate() override;
    virtual bool onStart() override;
    virtual bool onResume() override;
    virtual void onPause() override;
    virtual void onStop() override;
    virtual void onDestroy() override;
    
    virtual bool handleEvent(const framework::Event& event) override;
    
    void initialize(NetworkManager* networkManager);
    void draw();
    void update();
    
    // ホーム画面への遷移コールバック設定
    void setHomeTransitionCallback(std::function<void()> callback) {
        onHomeRequested = callback;
    }
    
private:
    // ネットワークマネージャー
    NetworkManager* m_networkManager;
    
    // UIコンポーネント
    ButtonFragment* m_homeButton;
    TextView* m_ssidTextView;
    TextView* m_ipAddressTextView;
    TextView* m_macAddressTextView;
    TextView* m_signalStrengthTextView;
    
    // 更新間隔の管理
    unsigned long m_lastUpdateTime;
    const unsigned long m_updateInterval = 5000; // 5秒ごとに更新
    
    // コールバック関数
    std::function<void()> onHomeRequested;
    
    // UIコンポーネント用のID定数
    enum {
        ID_NONE = 0,
        ID_BUTTON_HOME,
        ID_TEXT_SSID,
        ID_TEXT_IP_ADDRESS,
        ID_TEXT_MAC_ADDRESS,
        ID_TEXT_SIGNAL_STRENGTH
    };
};

#endif // NETWORK_SETTINGS_ACTIVITY_H
