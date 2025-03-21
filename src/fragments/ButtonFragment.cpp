#include "ButtonFragment.h"
#include "../framework/Activity.h"

ButtonFragment::ButtonFragment(uint32_t id)
    : Fragment(id), 
      m_button(0, 0, 0, 0, "Button"),
      m_pressed(false),
      m_clickHandler(nullptr),
      m_label("Button"),
      m_normalColor(DARKGREY),
      m_pressedColor(LIGHTGREY),
      m_fontSize(2),
      m_type(BUTTON_TYPE_OUTLINE)
{
}

bool ButtonFragment::onCreate() {
    if (!Fragment::onCreate()) {
        return false;
    }
    
    // Update button position and size
    updateButtonGeometry();
    
    return true;
}

void ButtonFragment::onDestroy() {
    Fragment::onDestroy();
}

bool ButtonFragment::handleEvent(const framework::Event& event) {    
    int eventID = (int)event.getType();
    Serial.println("ButtonFragment::handleEvent - Button ID=" + String(getId()) + 
                   ", Label='" + m_label + "', Event type=" + String(eventID));
    
    // Check if base class handles the event
    if (Fragment::handleEvent(event)) {
        Serial.println("  Base class handled the event");
        return true;
    }
    
    // Only process events if the fragment is visible and enabled
    if (!isVisible() || !isEnabled()) {
        Serial.println("  Fragment not visible or not enabled");
        return false;
    }
    
    // Handle touch events
    if (event.getType() == framework::EventType::TOUCH) {
        const framework::TouchEvent& touchEvent = static_cast<const framework::TouchEvent&>(event);
        
        int touchX = touchEvent.getX();
        int touchY = touchEvent.getY();
        int actionType = (int)touchEvent.getAction();
        
        Serial.println("  Touch event: x=" + String(touchX) + ", y=" + String(touchY) + 
                      ", action=" + String(actionType));
        Serial.println("  Button area: x=" + String(getX()) + ", y=" + String(getY()) + 
                      ", width=" + String(getWidth()) + ", height=" + String(getHeight()));
        
        // Check if touch is within button area
        if (touchX >= getX() && touchX < getX() + getWidth() &&
            touchY >= getY() && touchY < getY() + getHeight()) {
            
            Serial.println("  Touch is within button area");
            
            if (touchEvent.getAction() == framework::TouchAction::DOWN) {
                // Touch down - set pressed state
                Serial.println("  Touch DOWN - setting pressed state");
                m_pressed = true;
                m_button.setPressed(true);
                draw();
                return true;
            }
            else if (touchEvent.getAction() == framework::TouchAction::UP) {
                // Touch up within button area - trigger click
                Serial.println("  Touch UP within button area");
                if (m_pressed) {
                    Serial.println("  Button ID=" + String(getId()) + ", Label='" + m_label + "' was pressed, triggering click");
                    m_pressed = false;
                    m_button.setPressed(false);
                    draw();
                    
                    // Get parent activity ID if available
                    uint32_t activityId = 0;
                    // Check if parent component is an Activity by checking if its ID matches any Activity ID
                    // This avoids using dynamic_cast which requires RTTI
                    if (m_parent && m_parent->getId() >= 100 && m_parent->getId() % 100 == 0) {
                        // Parent is likely an Activity (IDs 100, 200, 300, etc. are reserved for Activities)
                        activityId = m_parent->getId();
                    }
                    
                    // Send button click event with activity ID
                    framework::ButtonEvent buttonEvent(framework::ButtonAction::CLICK, getId(), activityId);
                    Serial.println("  Creating ButtonEvent with activityId=" + String(activityId));
                    framework::EventBus::getInstance().postEvent(buttonEvent);
                    
                    // Call click handler if set
                    if (m_clickHandler) {
                        Serial.println("  Calling click handler for Button ID=" + String(getId()) + ", Label='" + m_label + "'");
                        m_clickHandler();
                    } else {
                        Serial.println("  No click handler set");
                    }
                    
                    return true;
                } else {
                    Serial.println("  Button was not pressed, ignoring UP event");
                }
            }
        } else {
            Serial.println("  Touch is outside button area");
            if (touchEvent.getAction() == framework::TouchAction::UP && m_pressed) {
                // Touch up outside button area - reset state without triggering click
                Serial.println("  Touch UP outside button area, resetting pressed state");
                m_pressed = false;
                m_button.setPressed(false);
                draw();
                return true;
            }
        }
    }
    
    return false;
}

void ButtonFragment::setLabel(const char* label) {
    m_label = label;
    m_button.setLabel(label);
    draw();
}

void ButtonFragment::setColor(uint16_t normalColor, uint16_t pressedColor) {
    m_normalColor = normalColor;
    m_pressedColor = pressedColor;
    m_button.setColor(normalColor, pressedColor);
    draw();
}

void ButtonFragment::setFontSize(uint8_t size) {
    m_fontSize = size;
    m_button.setFontSize(size);
    draw();
}

void ButtonFragment::setType(uint8_t type) {
    m_type = type;
    m_button.setType(type);
    draw();
}

void ButtonFragment::setClickHandler(std::function<void()> handler) {
    m_clickHandler = handler;
}

void ButtonFragment::draw() {
    // Draw the button
    m_button.draw();
}

void ButtonFragment::setDisplayArea(int x, int y, int width, int height) {
    // Call the base class implementation
    Fragment::setDisplayArea(x, y, width, height);
    
    // Update the button's geometry
    updateButtonGeometry();
}

void ButtonFragment::updateButtonGeometry() {
    // Create a new button with the updated position and size
    m_button = Button(getX(), getY(), getWidth(), getHeight(), m_label.c_str());
    
    // Restore custom settings
    m_button.setColor(m_normalColor, m_pressedColor);
    m_button.setFontSize(m_fontSize);
    m_button.setType(m_type);
    m_button.setPressed(m_pressed);
}
