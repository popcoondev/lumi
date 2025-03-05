#ifndef COMPONENT_H
#define COMPONENT_H

#include <cstdint>
#include <string>
#include <memory>
#include <functional>

namespace framework {

// Forward declaration
class Event;

/**
 * @brief Base class for all UI components
 * 
 * Component is the foundation class for all UI elements in the framework.
 * It provides ID management, lifecycle methods, and event handling capabilities.
 */
class Component {
public:
    /**
     * @brief Constructor with optional ID
     * @param id Optional component ID, auto-generated if not provided
     */
    Component(uint32_t id = 0);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Component() = default;
    
    /**
     * @brief Get the component's ID
     * @return The component ID
     */
    uint32_t getId() const { return m_id; }
    
    /**
     * @brief Set the component's ID
     * @param id The new ID to set
     */
    void setId(uint32_t id) { m_id = id; }
    
    /**
     * @brief Lifecycle method called when component is created
     * @return true if creation was successful, false otherwise
     */
    virtual bool onCreate() { return true; }
    
    /**
     * @brief Lifecycle method called when component is about to be destroyed
     */
    virtual void onDestroy() {}
    
    /**
     * @brief Handle an event
     * @param event The event to handle
     * @return true if the event was handled, false otherwise
     */
    virtual bool handleEvent(const Event& event) { return false; }
    
    /**
     * @brief Set an event handler for this component
     * @param handler The event handler function
     */
    void setEventHandler(std::function<bool(const Event&)> handler) {
        m_eventHandler = handler;
    }
    
    /**
     * @brief Check if component is enabled
     * @return true if enabled, false otherwise
     */
    bool isEnabled() const { return m_enabled; }
    
    /**
     * @brief Enable or disable the component
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    /**
     * @brief Check if component is visible
     * @return true if visible, false otherwise
     */
    bool isVisible() const { return m_visible; }
    
    /**
     * @brief Set component visibility
     * @param visible true to make visible, false to hide
     */
    void setVisible(bool visible) { m_visible = visible; }

protected:
    uint32_t m_id;                                   // Component unique identifier
    bool m_enabled = true;                           // Component enabled state
    bool m_visible = true;                           // Component visibility state
    std::function<bool(const Event&)> m_eventHandler; // Custom event handler
    
    /**
     * @brief Generate a unique ID for the component
     * @return A unique ID
     */
    static uint32_t generateId();
};

// Inline implementation of constructor
inline Component::Component(uint32_t id) : m_id(id == 0 ? generateId() : id) {}

// Inline implementation of ID generator
inline uint32_t Component::generateId() {
    static uint32_t nextId = 1;
    return nextId++;
}

} // namespace framework

#endif // COMPONENT_H
