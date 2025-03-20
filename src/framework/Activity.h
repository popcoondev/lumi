#ifndef ACTIVITY_H
#define ACTIVITY_H

#include "Component.h"
#include "Fragment.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace framework {

/**
 * @brief Activity state enumeration
 */
enum class ActivityState {
    UNCREATED,  // Activity has not been created
    CREATED,    // Activity has been created but not started
    STARTED,    // Activity has been started
    RESUMED,    // Activity is running and visible to the user
    PAUSED,     // Activity is partially visible but not in focus
    STOPPED,    // Activity is not visible
    DESTROYED   // Activity has been destroyed
};

/**
 * @brief Base class for full-screen activities
 * 
 * Activity represents a full-screen UI component that can contain
 * multiple fragments. It extends Component with additional lifecycle
 * methods and fragment management.
 */
class Activity : public Component {
public:
    /**
     * @brief Constructor with optional ID and name
     * @param id Optional activity ID, auto-generated if not provided
     * @param name Optional activity name
     */
    Activity(uint32_t id = 0, const std::string& name = "");
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Activity();
    
    /**
     * @brief Get the activity name
     * @return The activity name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * @brief Set the activity name
     * @param name The new name to set
     */
    void setName(const std::string& name) { m_name = name; }
    
    /**
     * @brief Get the current activity state
     * @return The current state
     */
    ActivityState getState() const { return m_state; }
    
    /**
     * @brief Lifecycle method called when activity is created
     * @return true if creation was successful, false otherwise
     */
    virtual bool onCreate() override;
    
    /**
     * @brief Lifecycle method called when activity is about to be destroyed
     */
    virtual void onDestroy() override;
    
    /**
     * @brief Lifecycle method called when activity is started
     * @return true if start was successful, false otherwise
     */
    virtual bool onStart();
    
    /**
     * @brief Lifecycle method called when activity is resumed
     * @return true if resume was successful, false otherwise
     */
    virtual bool onResume();
    
    /**
     * @brief Lifecycle method called when activity is paused
     */
    virtual void onPause();
    
    /**
     * @brief Lifecycle method called when activity is stopped
     */
    virtual void onStop();
    
    /**
     * @brief Add a fragment to the activity
     * @param fragment The fragment to add
     * @param tag Optional tag to identify the fragment
     * @return true if fragment was added successfully, false otherwise
     */
    bool addFragment(Fragment* fragment, const std::string& tag = "");
    
    /**
     * @brief Remove a fragment from the activity
     * @param fragment The fragment to remove
     * @return true if fragment was removed successfully, false otherwise
     */
    bool removeFragment(Fragment* fragment);
    
    /**
     * @brief Remove a fragment by tag
     * @param tag The tag of the fragment to remove
     * @return true if fragment was removed successfully, false otherwise
     */
    bool removeFragmentByTag(const std::string& tag);
    
    /**
     * @brief Find a fragment by tag
     * @param tag The tag to search for
     * @return Pointer to the fragment, or nullptr if not found
     */
    Fragment* findFragmentByTag(const std::string& tag) const;
    
    /**
     * @brief Find a fragment by ID
     * @param id The ID to search for
     * @return Pointer to the fragment, or nullptr if not found
     */
    Fragment* findFragmentById(uint32_t id) const;
    
    /**
     * @brief Get all fragments
     * @return Vector of fragment pointers
     */
    const std::vector<Fragment*>& getFragments() const { return m_fragments; }
    
    /**
     * @brief Handle an event
     * @param event The event to handle
     * @return true if the event was handled, false otherwise
     */
    virtual bool handleEvent(const Event& event) override;

protected:
    std::string m_name;                                // Activity name
    ActivityState m_state = ActivityState::UNCREATED;    // Current activity state
    std::vector<Fragment*> m_fragments;                // List of fragments
    std::unordered_map<std::string, Fragment*> m_taggedFragments; // Map of tagged fragments
    
    /**
     * @brief Set the activity state
     * @param state The new state to set
     */
    void setState(ActivityState state) { m_state = state; }
};

// Inline implementation of constructor
inline Activity::Activity(uint32_t id, const std::string& name)
    : Component(id), m_name(name) {
        setState(ActivityState::UNCREATED);
    }

// Inline implementation of destructor
inline Activity::~Activity() {
    // Clean up all fragments
    for (auto fragment : m_fragments) {
        fragment->onDetach();
    }
    m_fragments.clear();
    m_taggedFragments.clear();
}

// Inline implementation of onCreate
inline bool Activity::onCreate() {
    bool result = Component::onCreate();
    setState(ActivityState::CREATED);
    return result;
}

// Inline implementation of onDestroy
inline void Activity::onDestroy() {
    // First detach all fragments
    for (auto fragment : m_fragments) {
        fragment->onDetach();
    }
    m_fragments.clear();
    m_taggedFragments.clear();
    
    setState(ActivityState::DESTROYED);
    Component::onDestroy();
}

// Inline implementation of onStart
inline bool Activity::onStart() {
    setState(ActivityState::STARTED);
    return true;
}

// Inline implementation of onResume
inline bool Activity::onResume() {
    setState(ActivityState::RESUMED);
    return true;
}

// Inline implementation of onPause
inline void Activity::onPause() {
    setState(ActivityState::PAUSED);
}

// Inline implementation of onStop
inline void Activity::onStop() {
    setState(ActivityState::STOPPED);
}

// Inline implementation of addFragment
inline bool Activity::addFragment(Fragment* fragment, const std::string& tag) {
    if (!fragment) return false;
    
    // Check if fragment is already in the activity
    for (auto f : m_fragments) {
        if (f == fragment) return false;
    }
    
    // Attach the fragment to this activity
    if (fragment->onAttach(this)) {
        m_fragments.push_back(fragment);
        
        // Add to tagged fragments if tag is provided
        if (!tag.empty()) {
            m_taggedFragments[tag] = fragment;
        }
        
        return true;
    }
    
    return false;
}

// Inline implementation of removeFragment
inline bool Activity::removeFragment(Fragment* fragment) {
    if (!fragment) return false;
    
    // Find and remove from fragments vector
    auto it = std::find(m_fragments.begin(), m_fragments.end(), fragment);
    if (it != m_fragments.end()) {
        // Detach the fragment
        fragment->onDetach();
        
        // Remove from fragments vector
        m_fragments.erase(it);
        
        // Remove from tagged fragments if present
        for (auto it = m_taggedFragments.begin(); it != m_taggedFragments.end(); ) {
            if (it->second == fragment) {
                it = m_taggedFragments.erase(it);
            } else {
                ++it;
            }
        }
        
        return true;
    }
    
    return false;
}

// Inline implementation of removeFragmentByTag
inline bool Activity::removeFragmentByTag(const std::string& tag) {
    auto it = m_taggedFragments.find(tag);
    if (it != m_taggedFragments.end()) {
        Fragment* fragment = it->second;
        return removeFragment(fragment);
    }
    return false;
}

// Inline implementation of findFragmentByTag
inline Fragment* Activity::findFragmentByTag(const std::string& tag) const {
    auto it = m_taggedFragments.find(tag);
    return (it != m_taggedFragments.end()) ? it->second : nullptr;
}

// Inline implementation of findFragmentById
inline Fragment* Activity::findFragmentById(uint32_t id) const {
    for (auto fragment : m_fragments) {
        if (fragment->getId() == id) {
            return fragment;
        }
    }
    return nullptr;
}

// Inline implementation of handleEvent
inline bool Activity::handleEvent(const Event& event) {
    // First try to handle the event with the base class
    if (Component::handleEvent(event)) {
        return true;
    }
    
    // Then try to handle the event with fragments (in reverse order for proper z-ordering)
    for (auto it = m_fragments.rbegin(); it != m_fragments.rend(); ++it) {
        Fragment* fragment = *it;
        if (fragment->isEnabled() && fragment->isVisible() && fragment->handleEvent(event)) {
            return true;
        }
    }
    
    return false;
}

} // namespace framework

#endif // ACTIVITY_H
