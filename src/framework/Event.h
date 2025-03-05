#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <string>
#include <memory>
#include <chrono>

namespace framework {

/**
 * @brief Event type enumeration
 */
enum class EventType {
    NONE,           // No specific event type
    TOUCH,          // Touch event
    BUTTON,         // Button event
    SLIDER,         // Slider event
    KEYBOARD,       // Keyboard event
    SYSTEM,         // System event
    CUSTOM          // Custom event type
};

/**
 * @brief Touch action enumeration
 */
enum class TouchAction {
    DOWN,           // Touch down
    MOVE,           // Touch move
    UP,             // Touch up
    CANCEL          // Touch cancelled
};

/**
 * @brief Button action enumeration
 */
enum class ButtonAction {
    PRESS,          // Button pressed
    RELEASE,        // Button released
    CLICK,          // Button clicked (press and release)
    LONG_PRESS      // Button long-pressed
};

/**
 * @brief Base class for all events
 * 
 * Event is the foundation class for all events in the framework.
 * It provides type information and basic event data.
 */
class Event {
public:
    /**
     * @brief Constructor with event type
     * @param type The event type
     */
    Event(EventType type = EventType::NONE);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Event() = default;
    
    /**
     * @brief Get the event type
     * @return The event type
     */
    EventType getType() const { return m_type; }
    
    /**
     * @brief Set the event type
     * @param type The new type to set
     */
    void setType(EventType type) { m_type = type; }
    
    /**
     * @brief Get the event timestamp
     * @return The timestamp in milliseconds
     */
    uint64_t getTimestamp() const { return m_timestamp; }
    
    /**
     * @brief Set the event timestamp
     * @param timestamp The new timestamp to set
     */
    void setTimestamp(uint64_t timestamp) { m_timestamp = timestamp; }
    
    /**
     * @brief Check if the event is consumed
     * @return true if consumed, false otherwise
     */
    bool isConsumed() const { return m_consumed; }
    
    /**
     * @brief Mark the event as consumed
     * @param consumed true to mark as consumed, false otherwise
     */
    void setConsumed(bool consumed = true) { m_consumed = consumed; }
    
    /**
     * @brief Get a string representation of the event
     * @return String representation
     */
    virtual std::string toString() const;

protected:
    EventType m_type;           // Event type
    uint64_t m_timestamp;       // Event timestamp in milliseconds
    bool m_consumed = false;    // Whether the event has been consumed
    
    /**
     * @brief Get the current timestamp in milliseconds
     * @return Current timestamp
     */
    static uint64_t getCurrentTimestamp();
};

// Inline implementation of constructor
inline Event::Event(EventType type) : m_type(type), m_timestamp(getCurrentTimestamp()) {}

// Inline implementation of toString
inline std::string Event::toString() const {
    std::string typeStr;
    switch (m_type) {
        case EventType::NONE: typeStr = "NONE"; break;
        case EventType::TOUCH: typeStr = "TOUCH"; break;
        case EventType::BUTTON: typeStr = "BUTTON"; break;
        case EventType::SLIDER: typeStr = "SLIDER"; break;
        case EventType::KEYBOARD: typeStr = "KEYBOARD"; break;
        case EventType::SYSTEM: typeStr = "SYSTEM"; break;
        case EventType::CUSTOM: typeStr = "CUSTOM"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    return "Event{type=" + typeStr + ", timestamp=" + std::to_string(m_timestamp) + 
           ", consumed=" + (m_consumed ? "true" : "false") + "}";
}

// Inline implementation of getCurrentTimestamp
inline uint64_t Event::getCurrentTimestamp() {
    // This is a simple implementation that may need to be replaced with a more accurate one
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

/**
 * @brief Touch event class
 */
class TouchEvent : public Event {
public:
    /**
     * @brief Constructor with touch action
     * @param action The touch action
     * @param x The X-coordinate
     * @param y The Y-coordinate
     */
    TouchEvent(TouchAction action = TouchAction::DOWN, int x = 0, int y = 0);
    
    /**
     * @brief Get the touch action
     * @return The touch action
     */
    TouchAction getAction() const { return m_action; }
    
    /**
     * @brief Set the touch action
     * @param action The new action to set
     */
    void setAction(TouchAction action) { m_action = action; }
    
    /**
     * @brief Get the X-coordinate
     * @return The X-coordinate
     */
    int getX() const { return m_x; }
    
    /**
     * @brief Set the X-coordinate
     * @param x The new X-coordinate to set
     */
    void setX(int x) { m_x = x; }
    
    /**
     * @brief Get the Y-coordinate
     * @return The Y-coordinate
     */
    int getY() const { return m_y; }
    
    /**
     * @brief Set the Y-coordinate
     * @param y The new Y-coordinate to set
     */
    void setY(int y) { m_y = y; }
    
    /**
     * @brief Get a string representation of the event
     * @return String representation
     */
    std::string toString() const override;

private:
    TouchAction m_action;   // Touch action
    int m_x;                // X-coordinate
    int m_y;                // Y-coordinate
};

// Inline implementation of constructor
inline TouchEvent::TouchEvent(TouchAction action, int x, int y)
    : Event(EventType::TOUCH), m_action(action), m_x(x), m_y(y) {}

// Inline implementation of toString
inline std::string TouchEvent::toString() const {
    std::string actionStr;
    switch (m_action) {
        case TouchAction::DOWN: actionStr = "DOWN"; break;
        case TouchAction::MOVE: actionStr = "MOVE"; break;
        case TouchAction::UP: actionStr = "UP"; break;
        case TouchAction::CANCEL: actionStr = "CANCEL"; break;
        default: actionStr = "UNKNOWN"; break;
    }
    return "TouchEvent{action=" + actionStr + ", x=" + std::to_string(m_x) + 
           ", y=" + std::to_string(m_y) + ", " + Event::toString() + "}";
}

/**
 * @brief Button event class
 */
class ButtonEvent : public Event {
public:
    /**
     * @brief Constructor with button action
     * @param action The button action
     * @param buttonId The button ID
     */
    ButtonEvent(ButtonAction action = ButtonAction::PRESS, uint32_t buttonId = 0);
    
    /**
     * @brief Get the button action
     * @return The button action
     */
    ButtonAction getAction() const { return m_action; }
    
    /**
     * @brief Set the button action
     * @param action The new action to set
     */
    void setAction(ButtonAction action) { m_action = action; }
    
    /**
     * @brief Get the button ID
     * @return The button ID
     */
    uint32_t getButtonId() const { return m_buttonId; }
    
    /**
     * @brief Set the button ID
     * @param buttonId The new button ID to set
     */
    void setButtonId(uint32_t buttonId) { m_buttonId = buttonId; }
    
    /**
     * @brief Get a string representation of the event
     * @return String representation
     */
    std::string toString() const override;

private:
    ButtonAction m_action;  // Button action
    uint32_t m_buttonId;    // Button ID
};

// Inline implementation of constructor
inline ButtonEvent::ButtonEvent(ButtonAction action, uint32_t buttonId)
    : Event(EventType::BUTTON), m_action(action), m_buttonId(buttonId) {}

// Inline implementation of toString
inline std::string ButtonEvent::toString() const {
    std::string actionStr;
    switch (m_action) {
        case ButtonAction::PRESS: actionStr = "PRESS"; break;
        case ButtonAction::RELEASE: actionStr = "RELEASE"; break;
        case ButtonAction::CLICK: actionStr = "CLICK"; break;
        case ButtonAction::LONG_PRESS: actionStr = "LONG_PRESS"; break;
        default: actionStr = "UNKNOWN"; break;
    }
    return "ButtonEvent{action=" + actionStr + ", buttonId=" + std::to_string(m_buttonId) + 
           ", " + Event::toString() + "}";
}

/**
 * @brief Slider event class
 */
class SliderEvent : public Event {
public:
    /**
     * @brief Constructor with slider value
     * @param value The slider value
     * @param sliderId The slider ID
     */
    SliderEvent(float value = 0.0f, uint32_t sliderId = 0);
    
    /**
     * @brief Get the slider value
     * @return The slider value
     */
    float getValue() const { return m_value; }
    
    /**
     * @brief Set the slider value
     * @param value The new value to set
     */
    void setValue(float value) { m_value = value; }
    
    /**
     * @brief Get the slider ID
     * @return The slider ID
     */
    uint32_t getSliderId() const { return m_sliderId; }
    
    /**
     * @brief Set the slider ID
     * @param sliderId The new slider ID to set
     */
    void setSliderId(uint32_t sliderId) { m_sliderId = sliderId; }
    
    /**
     * @brief Get a string representation of the event
     * @return String representation
     */
    std::string toString() const override;

private:
    float m_value;          // Slider value
    uint32_t m_sliderId;    // Slider ID
};

// Inline implementation of constructor
inline SliderEvent::SliderEvent(float value, uint32_t sliderId)
    : Event(EventType::SLIDER), m_value(value), m_sliderId(sliderId) {}

// Inline implementation of toString
inline std::string SliderEvent::toString() const {
    return "SliderEvent{value=" + std::to_string(m_value) + 
           ", sliderId=" + std::to_string(m_sliderId) + 
           ", " + Event::toString() + "}";
}

} // namespace framework

#endif // EVENT_H
