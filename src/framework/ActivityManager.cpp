#include "ActivityManager.h"

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
    Activity* newActivity = getActivity(name);
    if (!newActivity) {
        return false;
    }
    
    // 現在のActivityがある場合は一時停止
    if (m_currentActivity) {
        m_currentActivity->onPause();
        m_currentActivity->onStop();
    }
    
    // 新しいActivityを開始
    m_currentActivity = newActivity;
    
    // Activityの状態に応じてライフサイクルメソッドを呼び出す
    if (m_currentActivity->getState() == ActivityState::CREATED) {
        m_currentActivity->onStart();
    }
    
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
