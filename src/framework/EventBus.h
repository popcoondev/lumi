#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include "Event.h"
#include "Component.h"
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>

namespace framework {

/**
 * @brief Event bus for delivering events to components
 * 
 * EventBus is a singleton class that manages event delivery to components.
 * It allows components to subscribe to specific event types and receive
 * notifications when those events occur.
 */
class EventBus {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static EventBus& getInstance();
    
    /**
     * @brief Destroy the singleton instance
     */
    static void destroyInstance();
    
    /**
     * @brief Post an event to all subscribers
     * @param event The event to post
     * @return true if the event was handled by any subscriber, false otherwise
     */
    bool postEvent(const Event& event);
    
    /**
     * @brief Subscribe a component to all events
     * @param component The component to subscribe
     * @return true if subscription was successful, false otherwise
     */
    bool subscribe(Component* component);
    
    /**
     * @brief Subscribe a component to a specific event type
     * @param component The component to subscribe
     * @param eventType The event type to subscribe to
     * @return true if subscription was successful, false otherwise
     */
    bool subscribe(Component* component, EventType eventType);
    
    /**
     * @brief Unsubscribe a component from all events
     * @param component The component to unsubscribe
     * @return true if unsubscription was successful, false otherwise
     */
    bool unsubscribe(Component* component);
    
    /**
     * @brief Unsubscribe a component from a specific event type
     * @param component The component to unsubscribe
     * @param eventType The event type to unsubscribe from
     * @return true if unsubscription was successful, false otherwise
     */
    bool unsubscribe(Component* component, EventType eventType);
    
    /**
     * @brief Register a custom event handler
     * @param eventType The event type to handle
     * @param handler The event handler function
     * @return true if registration was successful, false otherwise
     */
    bool registerHandler(EventType eventType, std::function<bool(const Event&)> handler);
    
    /**
     * @brief Unregister a custom event handler
     * @param eventType The event type to unregister
     * @return true if unregistration was successful, false otherwise
     */
    bool unregisterHandler(EventType eventType);

private:
    /**
     * @brief Private constructor (singleton pattern)
     */
    EventBus() = default;
    
    /**
     * @brief Private destructor (singleton pattern)
     */
    ~EventBus() = default;
    
    /**
     * @brief Private copy constructor (singleton pattern)
     */
    EventBus(const EventBus&) = delete;
    
    /**
     * @brief Private assignment operator (singleton pattern)
     */
    EventBus& operator=(const EventBus&) = delete;
    
    static EventBus* s_instance;                                // Singleton instance
    static std::mutex s_mutex;                                  // Mutex for thread safety
    
    std::unordered_map<EventType, std::vector<Component*>> m_subscribers;  // Map of event type to subscribers
    std::unordered_map<EventType, std::function<bool(const Event&)>> m_handlers;  // Map of event type to custom handlers
    std::vector<Component*> m_globalSubscribers;                // List of global subscribers (all events)
};

// Static member initialization
inline EventBus* EventBus::s_instance = nullptr;
inline std::mutex EventBus::s_mutex;

// Inline implementation of getInstance
inline EventBus& EventBus::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = new EventBus();
    }
    return *s_instance;
}

// Inline implementation of destroyInstance
inline void EventBus::destroyInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

// Inline implementation of postEvent
inline bool EventBus::postEvent(const Event& event) {
    bool handled = false;
    
    // First try custom handlers
    auto handlerIt = m_handlers.find(event.getType());
    if (handlerIt != m_handlers.end() && handlerIt->second) {
        handled = handlerIt->second(event);
        if (handled && event.isConsumed()) {
            return true;
        }
    }
    
    // Then try specific subscribers
    auto subscribersIt = m_subscribers.find(event.getType());
    if (subscribersIt != m_subscribers.end()) {
        for (auto component : subscribersIt->second) {
            if (component && component->isEnabled()) {
                if (component->handleEvent(event)) {
                    handled = true;
                    if (event.isConsumed()) {
                        return true;
                    }
                }
            }
        }
    }
    
    // Finally try global subscribers
    for (auto component : m_globalSubscribers) {
        if (component && component->isEnabled()) {
            if (component->handleEvent(event)) {
                handled = true;
                if (event.isConsumed()) {
                    return true;
                }
            }
        }
    }
    
    return handled;
}

// Inline implementation of subscribe (all events)
inline bool EventBus::subscribe(Component* component) {
    if (!component) return false;
    
    // Check if component is already subscribed
    for (auto c : m_globalSubscribers) {
        if (c == component) return false;
    }
    
    m_globalSubscribers.push_back(component);
    return true;
}

// Inline implementation of subscribe (specific event type)
inline bool EventBus::subscribe(Component* component, EventType eventType) {
    if (!component) return false;
    
    // Check if component is already subscribed to this event type
    auto& subscribers = m_subscribers[eventType];
    for (auto c : subscribers) {
        if (c == component) return false;
    }
    
    subscribers.push_back(component);
    return true;
}

// Inline implementation of unsubscribe (all events)
inline bool EventBus::unsubscribe(Component* component) {
    if (!component) return false;
    
    bool removed = false;
    
    // Remove from global subscribers
    auto it = std::find(m_globalSubscribers.begin(), m_globalSubscribers.end(), component);
    if (it != m_globalSubscribers.end()) {
        m_globalSubscribers.erase(it);
        removed = true;
    }
    
    // Remove from specific subscribers
    for (auto& pair : m_subscribers) {
        auto& subscribers = pair.second;
        auto it = std::find(subscribers.begin(), subscribers.end(), component);
        if (it != subscribers.end()) {
            subscribers.erase(it);
            removed = true;
        }
    }
    
    return removed;
}

// Inline implementation of unsubscribe (specific event type)
inline bool EventBus::unsubscribe(Component* component, EventType eventType) {
    if (!component) return false;
    
    auto it = m_subscribers.find(eventType);
    if (it != m_subscribers.end()) {
        auto& subscribers = it->second;
        auto componentIt = std::find(subscribers.begin(), subscribers.end(), component);
        if (componentIt != subscribers.end()) {
            subscribers.erase(componentIt);
            return true;
        }
    }
    
    return false;
}

// Inline implementation of registerHandler
inline bool EventBus::registerHandler(EventType eventType, std::function<bool(const Event&)> handler) {
    if (!handler) return false;
    
    m_handlers[eventType] = handler;
    return true;
}

// Inline implementation of unregisterHandler
inline bool EventBus::unregisterHandler(EventType eventType) {
    auto it = m_handlers.find(eventType);
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
        return true;
    }
    
    return false;
}

} // namespace framework

#endif // EVENT_BUS_H
