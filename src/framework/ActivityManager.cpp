#include "ActivityManager.h"
#include <M5Unified.h>

namespace framework {

ActivityManager::ActivityManager() : m_currentActivity(nullptr) {
}

ActivityManager::~ActivityManager() {
    // 登録されているActivityは外部で管理されるため、ここでは削除しない
    m_activities.clear();
    m_currentActivity = nullptr;
}

void ActivityManager::registerActivity(const std::string& name, Activity* activity) {
    if (activity) {
        m_activities[name] = activity;
    }
}

Activity* ActivityManager::getActivity(const std::string& name) {
    auto it = m_activities.find(name);
    return (it != m_activities.end()) ? it->second : nullptr;
}

Activity* ActivityManager::getCurrentActivity() {
    return m_currentActivity;
}

bool ActivityManager::startActivity(const std::string& name) {
    Serial.print("ActivityManager::startActivity - Starting activity: ");
    Serial.println(name.c_str());
    
    Activity* newActivity = getActivity(name);
    if (!newActivity) {
        Serial.print("ActivityManager::startActivity - Activity not found: ");
        Serial.println(name.c_str());
        return false;
    }
    
    // 現在のActivityがある場合は一時停止
    if (m_currentActivity) {
        Serial.print("ActivityManager::startActivity - Pausing current activity: ");
        Serial.println(m_currentActivity->getName().c_str());
        m_currentActivity->onPause();
        m_currentActivity->onStop();
    }
    
    // 新しいActivityを開始
    m_currentActivity = newActivity;
    
    // Activityの状態に応じてライフサイクルメソッドを呼び出す
    ActivityState state = m_currentActivity->getState();
    Serial.print("ActivityManager::startActivity - New activity state: ");
    Serial.println(static_cast<int>(state));
    
    if (state == ActivityState::CREATED || state == ActivityState::STOPPED) {
        Serial.print("ActivityManager::startActivity - Starting activity: ");
        Serial.println(name.c_str());
        m_currentActivity->onStart();
    }
    
    Serial.print("ActivityManager::startActivity - Resuming activity: ");
    Serial.println(name.c_str());
    m_currentActivity->onResume();
    
    return true;
}

void ActivityManager::finishCurrentActivity() {
    if (m_currentActivity) {
        m_currentActivity->onPause();
        m_currentActivity->onStop();
        m_currentActivity = nullptr;
    }
}

} // namespace framework
