#ifndef ACTIVITY_MANAGER_H
#define ACTIVITY_MANAGER_H

#include "Activity.h"
#include <unordered_map>
#include <string>

namespace framework {

class ActivityManager {
private:
    Activity* m_currentActivity;
    std::unordered_map<std::string, Activity*> m_activities;

public:
    ActivityManager();
    ~ActivityManager();
    
    // Activityの登録
    void registerActivity(const std::string& name, Activity* activity);
    
    // Activityの取得
    Activity* getActivity(const std::string& name);
    
    // 現在のActivityの取得
    Activity* getCurrentActivity();
    
    // Activityの開始
    bool startActivity(const std::string& name);
    
    // 現在のActivityの終了
    void finishCurrentActivity();
};

} // namespace framework

#endif // ACTIVITY_MANAGER_H
